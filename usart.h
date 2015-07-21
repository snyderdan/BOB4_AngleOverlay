//uart.h
//header file for usart routines

/*************************************************************************************/
//Decade Engineering
//Project: BUD (BOB-4 application development & deployment platform)
//Written by Eric L. Jennings
//Version 1 compiled 5/25/2011
//Compiled using GCC version 4.1.2 (WinAVR 20070525) AVRStudio version 4.17 build 666

//Target Processor: Atmel ATMega640 at 14.7456MHz
//Fuse settings:
//BODLEVEL Brown-out detection at VCC=4.3V
//BOOTRST enabled
//BOOTSZ Boot Flash Size=1024 Boot address=$7C00
//SPIEN enabled
//SUT_CKSEL Ext. Crystal Osc. 8.0- MHz; Start-up time: 16K CK + 65ms

//BUD is shipped with a bootloader to allow programming over the serial port.
//The bootloader is activated by placing a shunt on pins 4,6 of J1. If the 
//bootloader is not desired, the BOOTRST fuse can be disabled. This will 
//give an additional 1K of code space.

//Disclaimer:
//All BUD software is provided "as is," without warranty of any kind, express or implied, including but not limited to the implied warranties 
//of merchantability and fitness for a particular purpose and noninfringement. In no event shall Decade Engineering be liable for any direct, 
//indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; 
//loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, 
//or tort (including negligence or otherwise) arising in any way out of the use of this software, even if Decade Engineering is advised of the 
//possibility of such damage. 
/*************************************************************************************/

#ifndef UART_H
#define UART_H
#include <stdint.h>


/*
BPS			UBRR
2400		383
4800		191
9600		95
14.4k		63
19.2k		47
28.8k		31
38.4k		23
57.6k		15
76.8k		11
115.2k		7
230.4k		3
*/
#define BPS_2400	383
#define BPS_4800	191
#define BPS_9600	95
#define BPS_14400	63
#define BPS_19200	47
#define BPS_28800	31
#define BPS_38400	23
#define BPS_57600	15
#define BPS_76800	11
#define BPS_115200	7
#define BPS_230400	3

#define COMBUFFERSIZE 64

//combuffers and write pointers need to be declared 
//volatile because they are modified within ISRs
volatile uint8_t ComBuffer0[COMBUFFERSIZE];
volatile uint8_t ComBuffer1[COMBUFFERSIZE];
volatile uint8_t ComBuffer2[COMBUFFERSIZE];
volatile uint8_t ComBuffer3[COMBUFFERSIZE];

volatile uint8_t WritePointer0,WritePointer1,WritePointer2,WritePointer3;

uint8_t ReadPointer0,ReadPointer1,ReadPointer2,ReadPointer3;



void usart_init(void);
void PutChar0(uint8_t);
uint8_t GetChar0(void);
void SendString0(const char *str);
void SendData0(const uint8_t *data,uint16_t length);

void PutChar1(uint8_t);
uint8_t GetChar1(void);
void SendString1(const char *str);
void SendData1(const uint8_t *data,uint16_t length);

void PutChar2(uint8_t);
uint8_t GetChar2(void);
void SendString2(const char *str);
void SendData2(const uint8_t *data,uint16_t length);

void PutChar3(uint8_t);
uint8_t GetChar3(void);
void SendString3(const char *str);
void SendData2(const uint8_t *data,uint16_t length);

#endif 
