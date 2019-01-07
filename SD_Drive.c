#include<ioCC2530.h>
#include"SD_Drive.h"
#include"74LS164_8LED.h"
char SendPacket[]={0x0f,0x61,0x88,0x00,0x90,0xEB,0x18,0x54,0x18,0x55,2,3,4,5};
//��һ���ֽ�0x0C���壬����Լ����滹��12���ֽ�Ҫ����
//��5 6���ֽڱ�ʾ����PANID
//��7 8���ֽ�������ģ��Ŀ���豸�������ַ 0xEB90
//��9 10���Ǳ���ģ��������ַ
//11 ���ֽ����������õ�����
// CRC�� 12 13���ֽ� ��Ӳ���Զ�׷��
//��һλ=0x0b+�ҵ����ݳ���
char *pSendPacket=SendPacket;
void ON_32MOSC();
void ON_16MRC();
void DriveCfg();
void LEDs_Cfg();
void Bee_Cfg();
void LxChangR();
void KeysIntCfg();

void Uart0_Cfg();
void Uart0_SendCh(char ch);

void halRfInit(void);
void RevRFProc();
void Delay()
{
    int y,x;
    for(y=1000;y>0;y--)
      for(x=40;x>0;x--);
}
void ON_32MOSC()
{
      CLKCONCMD &= ~0x40;                          //����ϵͳʱ��ԴΪ32MHZ����
      while(CLKCONSTA & 0x40);                     //�ȴ������ȶ�
      CLKCONCMD &= ~0x47;                          //����ϵͳ��ʱ��Ƶ��Ϊ32MHZ 
}
void ON_16MRC()
{
      CLKCONCMD |=0x40;                      //����ϵͳʱ��Ϊ16M RC
      while(1==(CLKCONSTA & 0x40));          //�ȴ������ȶ�
      CLKCONCMD |=0x41;                      //����ϵͳ��ʱ��Ƶ��Ϊ16MHZ
}
void LEDs_Cfg()
{//LED��
     P0SEL&=~0x12;//P0_1(LED2),P0_4(LED3)  0001 0010  ��ͨIO��ģʽ
     P0DIR|=0x12;//���
  
     P1SEL&=~0x01;//P1_0��LED0��  0000 0001
     P1DIR|=0x01;//���
}
void Bee_Cfg()
{//������
     P0SEL&=~0x01;
     P0DIR|=0x01;//��� 
     OFF_BEE;
}
void LxChangR()
{//��������
     P0SEL&=~0x40;//P0_6  0100 0000  ��ͨIO��ģʽ
     P0DIR&=~0x40;
     P0INP&=~0x40;
     P2INP&=~0x20;//P2INP ��5  0010 0000 P0�����������蹦��
}
void Uart0_Cfg()
{
  /*
  PERCFG&=~0x01;   //��2������λ�ã�0ʹ�ñ���λ��1��1ʹ�ñ���λ��2
  P0SEL |= 0x0C;   //P0_2 RXD P0_3 TXD ���蹦�� 0000 1100
 
  U0CSR |= 0xC0;  //���ڽ���ʹ��  1100 0000 ����UARTģʽ+�������
  U0UCR |= 0x00;  //����żУ�飬1λֹͣλ
 
  U0GCR |= 11;  
  U0BAUD = 216;  //�����ʣ�115200bps  
 
  IEN0 |= 0X04;     //�����ڽ����ж� 'URX0IE = 1',Ҳ����д�� URX0IE=1;
  EA=1;*/
  PERCFG &=0xFE;//1111 1110 ѡ�д���0�ĵı���λ��1
  P0SEL  |=0x0C;       //0000 1100 P0_2 p0_3Ϊƫ�����蹦��
  
  U0CSR |=0Xc0;
  
  U0GCR =8;
  U0BAUD=59;
  EA=1;
  URX0IE=1;
}



void Uart0_SendCh(char ch)
{
    U0DBUF = ch;
    while(UTX0IF == 0);
    UTX0IF = 0;
}
void KeysIntCfg()
{//Key3  Key4   Key5
     
     IEN2|=0x10;//��P1IE���ж�
     P1IEN|=0x02;//��Key3�����ж�
     PICTL|=0x02;//����P1_1Ϊ�½���
     
     IEN2|=0x02;
     P2IEN|=0x01;
     PICTL|=0x08;//����P2_0Ϊ�½���
     
     P0IE=1;//����д�� IEN1|=0x20
     P0IEN|=0x20;
     PICTL|=0x01;//����P0_5Ϊ�½���
     
     
     EA=1;      //�����ж�
}


void halRfInit(void)
{
   EA=0;
    FRMCTRL0 |= 0x60;

    // Recommended RX settings  �Ƽ���Ƶ��������
    TXFILTCFG = 0x09;
    AGCCTRL1 = 0x15;
    FSCAL1 = 0x00;

    //����2���Ĵ��������ǿ���Ƶ�ж�
    // enable RXPKTDONE interrupt  
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;
    
    
//���ù����ŵ�
      FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);    
//����PANID,������ID�����ڷ���ģ��ͽ���ģ�鶼��ִ��������������Ժ���Ȼ���ǵĸ�����ID��һ���ģ��ŵ�Ҳ��һ����
      PAN_ID0=0x90;
      PAN_ID1=0xEB;    
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
    EA=1;    
}
void RevRFProc()
{
 static char len;
 static char  ch;
 len=ch=0;
    RFIRQM0 &= ~0x40;
    IEN2 &= ~0x01;
    EA=1;
 
    len=RFD;//����һ���ֽ��ж���һ�����ݺ����м����ֽڣ�
    //Uart0_SendCh(len);
    if(len==15)
    {
        while (len>0) 
        {//ֻҪ���滹��������ô�Ͱ������ӽ��ܻ�����ȡ����
            ch=RFD;
            //Uart0_SendCh(ch);
            if(3==len)//if((3==len)&&(LIGHTCMD==ch))
            {//��������������ֽڵ���7����ô���ǰ�LED0ȡ��
            LS164_BYTE(ch); 
            }
            len--;
         }  
    }
    EA=0;
    // enable RXPKTDONE interrupt
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;        
}

void RFSend(char *pstr,char len)
{
  char i;
    P0_4 = 0;
    RFST = 0xEC; //ȷ�������ǿյ�
    RFST = 0xE3; //����ձ�־λ
    while (FSMSTAT1 & 0x22);//�ȴ���Ƶ����׼����
    RFST = 0xEE;//ȷ�����Ͷ����ǿ�
    RFIRQF1 &= ~0x02;//�巢�ͱ�־λ
//Ϊ���ݷ�������׼������

    for(i=0;i<len;i++)
    {
       RFD=pstr[i];
    }  //ѭ���������ǰ�����Ҫ���͵�����ȫ��ѹ�����ͻ���������
    
    RFST = 0xE9; //����Ĵ���һ��������Ϊ0xE9,���ͻ����������ݾͱ����ͳ�ȥ
    while(!(RFIRQF1 & 0x02) );//�ȴ��������
    P0_4 = 1;
    RFIRQF1 = ~0x02;//�巢����ɱ�־
}
void DriveCfg()
{
     LEDs_Cfg();//��������˿�
//     Bee_Cfg();
//     LxChangR();
//     KeysIntCfg();//�����ж�
     Uart0_Cfg(); 
     halRfInit();
     
}


