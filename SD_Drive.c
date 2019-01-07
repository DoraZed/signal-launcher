#include<ioCC2530.h>
#include"SD_Drive.h"
#include"74LS164_8LED.h"
char SendPacket[]={0x0f,0x61,0x88,0x00,0x90,0xEB,0x18,0x54,0x18,0x55,2,3,4,5};
//第一个字节0x0C含义，这个自己后面还有12个字节要发送
//第5 6个字节表示的是PANID
//第7 8个字节是无线模块目标设备的网络地址 0xEB90
//第9 10就是本地模块的网络地址
//11 个字节是我们有用的数据
// CRC码 12 13个字节 是硬件自动追加
//第一位=0x0b+我的数据长度
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
      CLKCONCMD &= ~0x40;                          //设置系统时钟源为32MHZ晶振
      while(CLKCONSTA & 0x40);                     //等待晶振稳定
      CLKCONCMD &= ~0x47;                          //设置系统主时钟频率为32MHZ 
}
void ON_16MRC()
{
      CLKCONCMD |=0x40;                      //设置系统时钟为16M RC
      while(1==(CLKCONSTA & 0x40));          //等待晶振稳定
      CLKCONCMD |=0x41;                      //设置系统主时钟频率为16MHZ
}
void LEDs_Cfg()
{//LED组
     P0SEL&=~0x12;//P0_1(LED2),P0_4(LED3)  0001 0010  普通IO口模式
     P0DIR|=0x12;//输出
  
     P1SEL&=~0x01;//P1_0（LED0）  0000 0001
     P1DIR|=0x01;//输出
}
void Bee_Cfg()
{//蜂鸣器
     P0SEL&=~0x01;
     P0DIR|=0x01;//输出 
     OFF_BEE;
}
void LxChangR()
{//光敏电阻
     P0SEL&=~0x40;//P0_6  0100 0000  普通IO口模式
     P0DIR&=~0x40;
     P0INP&=~0x40;
     P2INP&=~0x20;//P2INP 第5  0010 0000 P0开起上拉电阻功能
}
void Uart0_Cfg()
{
  /*
  PERCFG&=~0x01;   //有2个备用位置，0使用备用位置1；1使用备用位置2
  P0SEL |= 0x0C;   //P0_2 RXD P0_3 TXD 外设功能 0000 1100
 
  U0CSR |= 0xC0;  //串口接收使能  1100 0000 工作UART模式+允许接受
  U0UCR |= 0x00;  //无奇偶校验，1位停止位
 
  U0GCR |= 11;  
  U0BAUD = 216;  //波特率：115200bps  
 
  IEN0 |= 0X04;     //开串口接收中断 'URX0IE = 1',也可以写成 URX0IE=1;
  EA=1;*/
  PERCFG &=0xFE;//1111 1110 选中串口0的的备用位置1
  P0SEL  |=0x0C;       //0000 1100 P0_2 p0_3为偏上外设功能
  
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
     
     IEN2|=0x10;//开P1IE组中断
     P1IEN|=0x02;//开Key3组内中断
     PICTL|=0x02;//设置P1_1为下降沿
     
     IEN2|=0x02;
     P2IEN|=0x01;
     PICTL|=0x08;//设置P2_0为下降沿
     
     P0IE=1;//或者写成 IEN1|=0x20
     P0IEN|=0x20;
     PICTL|=0x01;//设置P0_5为下降沿
     
     
     EA=1;      //开总中断
}


void halRfInit(void)
{
   EA=0;
    FRMCTRL0 |= 0x60;

    // Recommended RX settings  推荐射频接收设置
    TXFILTCFG = 0x09;
    AGCCTRL1 = 0x15;
    FSCAL1 = 0x00;

    //下面2个寄存器设置是开射频中断
    // enable RXPKTDONE interrupt  
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;
    
    
//设置工作信道
      FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);    
//设置PANID,个域网ID，由于发送模块和接受模块都会执行这个函数，所以很显然他们的个域网ID是一样的，信道也是一样的
      PAN_ID0=0x90;
      PAN_ID1=0xEB;    
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
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
 
    len=RFD;//读第一个字节判断这一串数据后面有几个字节；
    //Uart0_SendCh(len);
    if(len==15)
    {
        while (len>0) 
        {//只要后面还有数据那么就把他都从接受缓冲区取出来
            ch=RFD;
            //Uart0_SendCh(ch);
            if(3==len)//if((3==len)&&(LIGHTCMD==ch))
            {//如果倒数第三个字节等于7，那么我们把LED0取反
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
    RFST = 0xEC; //确保接收是空的
    RFST = 0xE3; //清接收标志位
    while (FSMSTAT1 & 0x22);//等待射频发送准备好
    RFST = 0xEE;//确保发送队列是空
    RFIRQF1 &= ~0x02;//清发送标志位
//为数据发送做好准备工作

    for(i=0;i<len;i++)
    {
       RFD=pstr[i];
    }  //循环的作用是把我们要发送的数据全部压到发送缓冲区里面
    
    RFST = 0xE9; //这个寄存器一旦被设置为0xE9,发送缓冲区的数据就被发送出去
    while(!(RFIRQF1 & 0x02) );//等待发送完成
    P0_4 = 1;
    RFIRQF1 = ~0x02;//清发送完成标志
}
void DriveCfg()
{
     LEDs_Cfg();//配置输出端口
//     Bee_Cfg();
//     LxChangR();
//     KeysIntCfg();//配置中断
     Uart0_Cfg(); 
     halRfInit();
     
}


