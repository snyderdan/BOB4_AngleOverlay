#include <stdio.h>

#include "command_handler.h"

const char CMD_IDENTIFIER[] = CSI_SEQ;	// 'CSI' or control sequence inducer as-per BOB-4

const char VALID_CMD[] = {CMD_ZERO, CMD_CALIBRATE, CMD_RELMODE, 
	CMD_ABSMODE, CMD_TOGMODE, CMD_GETPARAMS, CMD_GETRAW, CMD_GETSAMPLE};
const int VALID_CMD_COUNT = 8;

char CMDIN[256];
uint8_t filled = 0;
uint8_t processed = 0;

void commandHandler() {
	
	int i;
	char curChar, string[512];
	int flags = 0;
	
	# define FL_PANOPERAND		0x01
	# define FL_TILTOPERAND		0x02
	# define FL_TILTUOPERAND 	0x04
	# define FL_TILTLOPERAND    0x08

	//check to see if byte was received from the host
	while (ReadPointer2 != WritePointer2) {	
		//retrieve the byte from the buffer
		char inchar = GetChar2();
		//pass it on to the BOB-4
		CMDIN[filled++] = inchar;
	}
	
	// see if there is potentially a command in the buffer
	if (filled > (processed + CSI_LEN)) {
		
		for (i=0; i<CSI_LEN; i++) {	// identifier matches?
			// if not, then consider it garbage
			if (CMDIN[processed+i] != CMD_IDENTIFIER[i]) {
				processed += CSI_LEN;
				return;
			}
		}
		
		curChar = CMDIN[processed + CSI_LEN];	// potential command char
		
		// ensure that it is a valid command
		for (i=0; i<VALID_CMD_COUNT; i++) {
			if (VALID_CMD[i] == curChar) break;
		}
		
		if (i == VALID_CMD_COUNT) {	// it is not a valid command
			processed += CSI_LEN + 1;	// consider it garbage
		}
		
		// we now have a valid command. So get the operands
		for (i=1; i<=MAX_OPERAND_COUNT; i++) {
			curChar = CMDIN[processed + CSI_LEN + i];
			if (curChar == OPERAND_PAN) {
				flags |= FL_PANOPERAND;
			} else if (curChar == OPERAND_TILT) {
				flags |= FL_TILTOPERAND;
			} else if (curChar == OPERAND_TILTU) {
				flags |= FL_TILTUOPERAND;
			} else if (curChar == OPERAND_TILTL) {
				flags |= FL_TILTLOPERAND;
			}
		}
		
		curChar = CMDIN[processed + CSI_LEN];	// set it to the command again
		
		// figure out which command it is and execute
		if (curChar == CMD_ZERO) {
			processed += 1;
			if (flags & FL_PANOPERAND) {
				processed += 1;
				setPanShift(sampledRawPan());
			} 
			if (flags & FL_TILTOPERAND) {
				processed += 1;
				setTiltShift(sampledRawTilt());
			}
		} else if (curChar == CMD_CALIBRATE) {
			processed += 1;
			if (flags & FL_PANOPERAND) {
				processed += 1;
				calibratePan();
			} 
			if (flags & FL_TILTUOPERAND) {
				processed += 1;
				calibrateTiltUpper();
			} 
			if (flags & FL_TILTLOPERAND) {
				processed += 1;
				calibrateTiltLower();
			}
		} else if (curChar == CMD_RELMODE) {
			processed += 1;
			if (flags & FL_PANOPERAND) {
				processed += 1;
				setPanMode(MODE_RELATIVE);
			} 
			if (flags & FL_TILTOPERAND) {
				processed += 1;
				setTiltMode(MODE_RELATIVE);
			}
		} else if (curChar == CMD_ABSMODE) {
			processed += 1;
			if (flags & FL_PANOPERAND) {
				processed += 1;
				setPanMode(MODE_ABSOLUTE);
			} 
			if (flags & FL_TILTOPERAND) {
				processed += 1;
				setTiltMode(MODE_ABSOLUTE);
			}
		} else if (curChar == CMD_TOGMODE) {
			processed += 1;
			if (flags & FL_PANOPERAND) {
				processed += 1;
				togglePanMode();
			} 
			if (flags & FL_TILTOPERAND) {
				processed += 1;
				toggleTiltMode();
			}
		} else if (curChar == CMD_GETPARAMS) {
			processed += 1;
			sprintf(string, "\n\rPAN X SHIFT:  %4d  PAN Y SHIFT:  %-3d deg\
					         \n\rPAN RANGE:    %4d  PAN SLOPE:    %f \
					     \n\r\n\rTILT X SHIFT: %4d  TILT Y SHIFT: %-3d deg\
					         \n\rTILT RANGE:   %4d  TILT SLOPE:   %f \n\r",
					 panXShift, panYShift, panRange, panSlope,
					 tiltXShift, tiltYShift, tiltRange, tiltSlope
					);
			SendString2(string);
		} else if (curChar == CMD_GETRAW) {
			processed += 1;
			sprintf(string, "\n\rCURRENT RAW PAN: %4d  CURRENT RAW TILT: %d\n\r", 
					analogRead(PAN_PORT), analogRead(TILT_PORT)
					);
			SendString2(string);
		} else if (curChar == CMD_GETSAMPLE) {
			processed += 1;
			sprintf(string, "\n\rCURRENT RAW PAN: %4d  CURRENT RAW TILT: %d\n\r", 
					sampledRawPan(), sampledRawTilt()
					);
			SendString2(string);
		} else {
			processed += 1;
		}
	}
}
