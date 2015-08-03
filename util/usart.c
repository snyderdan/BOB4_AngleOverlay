//usart.c
//usart initialization and communication routines

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

//The 4 USARTs on the processor are dedicated as follows:
//USART0 -> GPIO Port J3 Pins 1,2 (through RS232 hardware interface)
//USART0 -> GPIO Port J3 Pins 3,4
//USART1 -> BOB-4
//USART2 -> Com Port J4 (through RS232 hardware interface)
//USART3 -> GPIO Port J3 Pins 33,34

//Code assumes no shunts on J5 of BUD which sets BOB-4 to 115.2 kbs. Using 
//different baud rates between BOB-BUD and BUD-host may cause buffer overflow.
//There are no provisions in this code for hardware handshaking however BOB-4 uses 
//XON/XOFF handshaking by default which is passed through BUD. If you have problems 
//with buffer overflow in BUD the size of the com buffer can be increased and several 
//clock cycles could be saved by placing the transmit code within the receive ISR.

//Demonstrates USART setup, interrupt-driven USART receive, use of circular
//buffer for USART receive.

#include <avr/interrupt.h>
#include <string.h>

#include "usart.h"

/**********************************************
This function clears the com buffers and buffer pointers, initializes baud rates and 
frame formats, then enables transmit and interrupt driven receive. As written, the baud
rates and frame formats are hard coded, to change them dynamically the function would
need to be rewritten with parameters for the new values. In that case you would need to 
ensure that all transmissions are completed and the Global Interrupt Flag is cleared
before reinitialization.
**********************************************/
void usart_init(void)
{
	//clear the com buffers and pointers
	ReadPointer0 =ReadPointer1 =ReadPointer2 = 0;
	WritePointer0 = WritePointer1 = WritePointer2 = 0;
	memset(&ComBuffer0,0,sizeof(ComBuffer0));
	memset(&ComBuffer1,0,sizeof(ComBuffer1));
	memset(&ComBuffer2,0,sizeof(ComBuffer2));
	memset(&ComBuffer3,0,sizeof(ComBuffer3));

	//set baud rates
	UBRR0 = BPS_9600;
	//BOB-4 is set to 115200 with no shunts on J5 
	UBRR1 = BPS_115200;
	UBRR2 = BPS_115200;
	UBRR3 = BPS_9600;

	//enable transmit and interrupt driven recieve
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	UCSR1B = (1<<RXEN1)|(1<<TXEN1)|(1<<RXCIE1);
	UCSR2B = (1<<RXEN2)|(1<<TXEN2)|(1<<RXCIE2);
	UCSR3B = (1<<RXEN3)|(1<<TXEN3)|(1<<RXCIE3);

	//set frame format 8,N,1
	UCSR0C = (1<<UCSZ00)|(1<<UCSZ01);
	UCSR1C = (1<<UCSZ00)|(1<<UCSZ01);
	UCSR2C = (1<<UCSZ00)|(1<<UCSZ01);
	UCSR3C = (1<<UCSZ00)|(1<<UCSZ01);

	//perform a dummy read to clear RXC bits
	uint8_t temp;
	temp = UDR0;
	temp = UDR1;
	temp = UDR2;
	temp = UDR3;
}
/**********************************************/

/**********************************************
usart recieve ISRs
These routines collect a byte from the Receive Data Buffer Register and place it in
the com buffer at the location indicated by the write pointer, the write pointer is 
advanced, checked for overflow, and wrapped to zero if necessary.
**********************************************/
ISR(USART0_RX_vect)
{
	//place received byte in com buffer
	ComBuffer0[WritePointer0] = UDR0;

	//advance pointer
	WritePointer0++;

	//check for overflow
	if (WritePointer0 == COMBUFFERSIZE)
		WritePointer0 = 0;
}
/**********************************************/

ISR(USART1_RX_vect)
{
	ComBuffer1[WritePointer1] = UDR1;
	WritePointer1++;
	if (WritePointer1 == COMBUFFERSIZE)
		WritePointer1 = 0;
}
/**********************************************/

ISR(USART2_RX_vect)
{
	//PutChar0(UDR2);
	ComBuffer2[WritePointer2] = UDR2;
	WritePointer2++;
	if (WritePointer2 == COMBUFFERSIZE)
		WritePointer2 = 0;
}
/**********************************************/

ISR(USART3_RX_vect)
{
	ComBuffer3[WritePointer3] = UDR3;
	WritePointer3++;
	if (WritePointer3 == COMBUFFERSIZE)
		WritePointer3 = 0;
}
/**********************************************/

/**********************************************
retrieve characters from combuffers
These routines return a byte retrieved from the com buffer at the location indicated 
by the read pointer, the read pointer is advanced, checked for overflow, and wrapped 
to zero if necessary.
**********************************************/
uint8_t GetChar0(void)
{
	//get byte from com buffer
	uint8_t ret = ComBuffer0[ReadPointer0];

	//advance pointer
	ReadPointer0++;

	//check for overflow
	if (ReadPointer0 == COMBUFFERSIZE)
		ReadPointer0 = 0;

	//return byte
	return ret;
}
/**********************************************/

uint8_t GetChar1(void)
{
	uint8_t ret = ComBuffer1[ReadPointer1];
	ReadPointer1++;
	if (ReadPointer1 == COMBUFFERSIZE)
		ReadPointer1 = 0;
	return ret;
}
/**********************************************/

uint8_t GetChar2(void)
{
	uint8_t ret = ComBuffer2[ReadPointer2];
	ReadPointer2++;
	if (ReadPointer2 == COMBUFFERSIZE)
		ReadPointer2 = 0;
	return ret;
}
/**********************************************/

uint8_t GetChar3(void)
{
	uint8_t ret = ComBuffer3[ReadPointer3];
	ReadPointer3++;
	if (ReadPointer3 == COMBUFFERSIZE)
		ReadPointer3 = 0;
	return ret;
}
/**********************************************/

/**********************************************
low level usart send
These routines transmit a single byte out the USART
**********************************************/
void PutChar0(uint8_t outchar)
{
	//ensure that any previous transmissions are complete
	loop_until_bit_is_set(UCSR0A,UDRE0);

	//send the byte
	UDR0 = outchar;
}
/**********************************************/

void PutChar1(uint8_t outchar)
{
	loop_until_bit_is_set(UCSR1A,UDRE1);
	UDR1 = outchar;
}
/**********************************************/
void PutChar2(uint8_t outchar)
{
	loop_until_bit_is_set(UCSR2A,UDRE2);
	UDR2 = outchar;
}
/**********************************************/
void PutChar3(uint8_t outchar)
{
	loop_until_bit_is_set(UCSR3A,UDRE3);
	UDR3 = outchar;
}
/**********************************************/


/**********************************************
upper level uart send
These routines transmit a string or data array out the USART
**********************************************/
void SendString0(const char *str)
{
	unsigned int index = 0;
	//advance though the string until a zero byte is reached
	while(str[index] != 0)
	{
		PutChar0(str[index++]);
	}
}
/**********************************************/

void SendString1(const char *str)
{
	unsigned int index = 0;
	while(str[index] != 0)
	{
		PutChar1(str[index++]);
	}
}
/**********************************************/

void SendString2(const char *str)
{
	unsigned int index = 0;
	while(str[index] != 0)
	{
		PutChar2(str[index++]);
	}
}
/**********************************************/

void SendString3(const char *str)
{
	unsigned int index = 0;
	while(str[index] != 0)
	{
		PutChar3(str[index++]);
	}
}
/**********************************************/

void SendData0(const uint8_t *data,uint16_t length)
{
	//advance though the array 'data' until 'length' bytes are sent
	uint8_t index = 0;
	while(index < length)
	{
		PutChar0(data[index++]);
	}
	
}
/**********************************************/
void SendData1(const uint8_t *data,uint16_t length)
{
	uint16_t index = 0;
	while(index < length)
	{
		PutChar1(data[index++]);
	}
	
}
/**********************************************/
void SendData2(const uint8_t *data,uint16_t length)
{
	uint16_t index = 0;
	while(index < length)
	{
		PutChar2(data[index++]);
	}
	
}
/**********************************************/
void SendData3(const uint8_t *data,uint16_t length)
{
	uint16_t index = 0;
	while(index < length)
	{
		PutChar3(data[index++]);
	}
	
}
/**********************************************/
