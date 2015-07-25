//main.c
//Main implementation file. 

/*************************************************************************************/
//Prevision Systems LLC
//Project: Pan/Tilt video overlay
//Written by Dan Snyder
//Version 1 compiled 7/2/2015
//Compiled using GCC version 4.3.3 (WinAVR 20100110)

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
/*************************************************************************************/

# define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <util/delay.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "usart.h"
#include "pantilt.h"
#include "analog.h"
#include "command_handler.h"

/**********************************************
This routine calls initialization routines, then loops reading each analog value
and writing it to the screen. It then clears the screen, and reloops.
**********************************************/
int main(void)
{
	char str[50];

	//call the USART initalization routine
	usart_init();
	initADC();
	initPanTilt();
	
	setAnalogReference(AREF_EXAREF);

	//enable interrupts
	sei();

	//reset the BOB-4
	sprintf(str,"\e[3210|");
	SendString1(str);

	//send a greeting to the host PC
	sprintf(str,"BUD to BOB-4 Angle Overlay application\n\rready\n\r");
	SendString2(str);
	
	// set font to smallest english ASCII
	sprintf(str,"\e[4z");
	SendString1(str);
	
	while(1)
	{
		
		// go to PAN field and clear last number
		sprintf(str,"\e[0;0H");
		SendString1(str);
		
		sprintf(str, "PAN:  %-4d    ", getPanAngle());
		SendString1(str);
		
		// go to TILT field and clear last number
		sprintf(str, "\e[1;0H");
		SendString1(str);
		
		sprintf(str, "TILT: %-4d    ", getTiltAngle());
		SendString1(str);
		
		commandHandler();
		
		_delay_ms(25.0);
	}
}
