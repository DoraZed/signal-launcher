
#include<ioCC2530.h>
#include"SD_Drive.h"
#include"74LS164_8LED.h"
extern char SendPacket[];
char cycle=12,amplitude=2,mode=0,most=80,zhi=0;
unsigned char jieshou[]={0,0,0,0,0};
unsigned char wei=0;
void Init32M()
{
   SLEEPCMD &=0xFB;//1111 1011 开启2个高频时钟源
   while(0==(SLEEPSTA & 0x40));// 0100 0000 等待32M稳定
   Delay();
   CLKCONCMD &=0xF8;//1111 1000 不分频输出
   CLKCONCMD &=0XBF;//1011 1111 设置32M作为系统主时钟
   while(CLKCONSTA & 0x40); //0100 0000 等待32M成功成为当前系统主时钟
}


void main()
{
   LS164_Cfg();//74LS164控制数码管的初始化
   Init32M(); //主时钟晶振工作在32M 
   DriveCfg();
   
  SHORT_ADDR0=0x18;
  SHORT_ADDR1=0x55;//设置本模块地址  5518
  
    LS164_BYTE(1); 
    while(1)
    {
      if(zhi>14)zhi=14;
      if(zhi<1)zhi=1;
    }
}
#pragma vector=P1INT_VECTOR
__interrupt void Key3_ISR() //P1_1
{
     Delay();
     if(0==P1_1)
     {
         if(0x02 & P1IFG)
         {
             ON_LED0;
             OFF_LED2;
             OFF_LED3;
             most++;mode++;amplitude++;cycle++;
             SendPacket[10]=most;
             SendPacket[11]=mode;
             SendPacket[12]=amplitude;
             SendPacket[13]=cycle;
             RFSend(SendPacket,15);
             
         }     
     }


     P1IFG &=~0x02;
     P1IF=0;
}
#pragma vector=P2INT_VECTOR
__interrupt void Key4_ISR()//P2_0
{
     ON_LED2;
     OFF_LED0;
     OFF_LED3;

     P2IFG &=~0x01;
     P2IF=0;
}
#pragma vector=P0INT_VECTOR
__interrupt void Key5_ISR()//P0_5
{
     ON_LED3;
     OFF_LED0;
     OFF_LED2;
     zhi--;
     most--;mode--;amplitude--;cycle--;
     SendPacket[10]=most;
     SendPacket[11]=mode;
     SendPacket[12]=amplitude;
     SendPacket[13]=cycle;
     RFSend(SendPacket,15);
     P0IFG &=~0x20;
     P0IF=0;
}


#pragma vector=RF_VECTOR
__interrupt void RF_IRQ(void)
{
  unsigned long i=1000;
    EA=0;
    
    if( RFIRQF0 & 0x40 )
    {
        RevRFProc();//处理中断;
        RFIRQF0&= ~0x40;   // Clear RXPKTDONE interrupt
    }
    S1CON= 0;                   // Clear general RF interrupt flag
    while(i--);
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;

}

#pragma vector=URX0_VECTOR
__interrupt void URX0_IRQ(void)
{ 
  char j;
      URX0IF=0;
      zhi=U0DBUF; 
      jieshou[0]=zhi;
      if(jieshou[4]==235)      
      { 
        SendPacket[10]=jieshou[3];
        SendPacket[11]=jieshou[2];
        SendPacket[12]=jieshou[1];
        SendPacket[13]=jieshou[0];  
        RFSend(SendPacket,15);
      }
      
      for(j=5; j>0; j--)
      {
        jieshou[j]=jieshou[j-1];
      } 
}