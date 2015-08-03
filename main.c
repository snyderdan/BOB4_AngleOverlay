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

#include "util/usart.h"
#include "util/analog.h"
#include "pantilt.h"
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
	sprintf(str,"\e[14m\e[66m\e[2J");
	SendString1(str);
	
	_delay_ms(500.00);	// allow time for BOB4 to make adjustments
	
	while(1)
	{
			// set font to smallest english ASCII
		sprintf(str,"\e[14m\e[66m");
		SendString1(str);
		
		if (debugMode == 1) {
			// go to PAN field and clear last number
			sprintf(str,"\e[18;37H");
			SendString1(str);
			sprintf(str, " PAN:%4d RAW:%5d SAMP:%5d ", getPanAngle(), getRawPan(), getSampledPan());
			SendString1(str);
			
			// go to TILT field and clear last number
			sprintf(str, "\e[19;37H");
			SendString1(str);
			
			sprintf(str, " TLT:%4d RAW:%5d SAMP:%5d ", getTiltAngle(), getRawTilt(), getSampledTilt());
			SendString1(str);
		} else {
			// go to PAN field and clear last number
			sprintf(str,"\e[18;58H");
			SendString1(str);
			sprintf(str, " PAN:%4d ", getPanAngle());
			SendString1(str);
			
			// go to TILT field and clear last number
			sprintf(str, "\e[19;58H");
			SendString1(str);
			
			sprintf(str, " TLT:%4d ", getTiltAngle());
			SendString1(str);
		}
		
		commandHandler();
	}
}
