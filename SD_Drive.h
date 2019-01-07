#ifndef __SD_DRIVE_H__
#define __SD_DRIVE_H__
#include<ioCC2530.h>
extern void ON_32MOSC();
extern void ON_16MRC();
extern void DriveCfg();
extern void Delay();
extern void LEDs_Cfg();
extern void Bee_Cfg();
extern void LxChangR();
extern void KeysIntCfg();
extern void Uart0_Cfg();
extern void Uart0_SendCh(char ch);
extern void halRfInit(void);
extern void RevRFProc();
extern void RFSend(char *pstr,char len);
#define OFF_LED0  P1_0=1
#define ON_LED0   P1_0=0
#define OFF_LED2  P0_1=1
#define ON_LED2   P0_1=0
#define OFF_LED3  P0_4=1
#define ON_LED3   P0_4=0

#define OFF_BEE   P0_0=0
#define ON_BEE    P0_0=1

#define LXCHANGER  P0_6
#endif