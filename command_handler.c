#include <stdio.h>

#include "command_handler.h"

const char VALID_CMD[] = {CMD_ZERO, CMD_CALIBRATE, CMD_RELMODE, CMD_ABSMODE,
	 CMD_TOGMODE, CMD_GETPARAMS, CMD_GETRAW, CMD_GETSAMPLE, CMD_SETPT,
	 CMD_ANALOG, CMD_DIGITAL};

const uint8_t VALID_CMD_ARGC[] = {2,1,2,2,2,0,0,0,8,0,0};

# define MAX_ARG_COUNT 8	// Number of args used in setpt command
	
const int VALID_CMD_COUNT = 11;

char CMDIN[256];

uint8_t filled = 0;
uint8_t processed = 0;

uint16_t hex2int16(char hh, char hl, char lh, char ll) {
	
	uint16_t value;
	
	value |= (((hh <= '9') ? hh-'0' : hh-'7') << 12);
	value |= (((hl <= '9') ? hl-'0' : hl-'7') << 8);
	value |= (((lh <= '9') ? lh-'0' : lh-'7') << 4);
	value |= ((ll <= '9')  ? ll-'0' : ll-'7');
	
	return value;
}

void commandHandler() {
	
	int i, argc, available;
	char curChar, string[512];
	char args[MAX_ARG_COUNT];

	//check to see if byte was received from the host
	while (ReadPointer2 != WritePointer2) {	
		//retrieve the byte from the buffer
		char inchar = GetChar2();
		//pass it on to the BOB-4
		CMDIN[filled++] = inchar;
	}
	
	available = (uint8_t)(filled - processed);
	
	// see if there is potentially a CSI and command present
	if (available < 3) {
		// if not, then return and wait for next iteration
		return;
	}
	
	// CSI matches?
	if (CMDIN[processed] != CSI_SEQ) {
		// if not, consider it garbage
		processed++;
		return;
	}
	
	// CSI matches, check for valid command
	
	curChar = CMDIN[(uint8_t)(processed + 1)];	// potential command char
	
	// ensure that it is a valid command
	for (i=0; i<VALID_CMD_COUNT; i++) {
		if (VALID_CMD[i] == curChar) break;
	}
	
	if (i == VALID_CMD_COUNT) {	// it is not a valid command
		processed += 2;	// consider it garbage
	}
	
	// we now have a valid command. So get the argument count
	argc = VALID_CMD_ARGC[i];
	
	// isolate arguments
	for (i=0; (i<available) && (i<=argc); i++) {
		curChar = CMDIN[(uint8_t)(processed+i+2)];
		if (curChar == CSE_SEQ) {	// termination? 
			argc = i;
			break;			// exit loop - only real way out
		}
		args[i] = curChar;	// assign next arg byte
	}
	
	if (i == available) {	// meaning we are waiting for more chars
		return;				// wait for next iteration
	} else if (i > argc) {	// meaning too many args were passed
		processed += i + 2;	// disregard entire command 
		return;
	}
	// We made it this far! Now we have a valid command and we can increment 
	// our processed count.
	processed += 3 + argc;
	// figure out which command it is and execute
	if (curChar == CMD_ZERO) {
		for (i=0; i<argc; i++) {
			if (args[i] == OPERAND_PAN) {
				setPanShift(getSampledPan());
			} else if (args[i] == OPERAND_TILT) {
				setTiltShift(getSampledTilt());
			}
		}
	} else if (curChar == CMD_CALIBRATE) {
		for (i=0; i<argc; i++) {
			if (args[i] == OPERAND_PAN) {
				calibratePan();
			} else if (args[i] == OPERAND_TILTU) {
				calibrateTiltUpper();
			} else if (args[i] == OPERAND_TILTL) {
				calibrateTiltLower();
			}
		}
	} else if (curChar == CMD_RELMODE) {
		for (i=0; i<argc; i++) {
			if (args[i] == OPERAND_PAN) {
				setPanMode(MODE_RELATIVE);
			} else if (args[i] == OPERAND_TILT) {
				setTiltMode(MODE_RELATIVE);
			}
		}
	} else if (curChar == CMD_ABSMODE) {
		for (i=0; i<argc; i++) {
			if (args[i] == OPERAND_PAN) {
				setPanMode(MODE_ABSOLUTE);
			} else if (args[i] == OPERAND_TILT) {
				setTiltMode(MODE_ABSOLUTE);
			}
		}
	} else if (curChar == CMD_TOGMODE) {
		for (i=0; i<argc; i++) {
			if (args[i] == OPERAND_PAN) {
				togglePanMode();
			} else if (args[i] == OPERAND_TILT) {
				toggleTiltMode();
			}
		}
	} else if (curChar == CMD_GETPARAMS) {
		sprintf(string, "\n\rPAN X SHIFT:  %4d  PAN Y SHIFT:  %-3d deg\
						 \n\rPAN RANGE:    %4d  PAN SLOPE:    %f \
					 \n\r\n\rTILT X SHIFT: %4d  TILT Y SHIFT: %-3d deg\
						 \n\rTILT RANGE:   %4d  TILT SLOPE:   %f \n\r",
				 panXShift, panYShift, panRange, panSlope,
				 tiltXShift, tiltYShift, tiltRange, tiltSlope
				);
		SendString2(string);
	} else if (curChar == CMD_GETRAW) {
		sprintf(string, "\n\rCURRENT RAW PAN: %4d  CURRENT RAW TILT: %d\n\r", 
				getRawPan(), getRawTilt()
				);
		SendString2(string);
	} else if (curChar == CMD_GETSAMPLE) {
		sprintf(string, "\n\rCURRENT SAMPLED PAN: %4d  CURRENT SAMPLED TILT: %d\n\r", 
				getSampledPan(), getSampledTilt()
				);
		SendString2(string);
	} else if (curChar == CMD_SETPT) {
		// if we don't have full args, then return
		if (argc != 8) {
			return;
		}
		setDigitalPan(hex2int16(args[0],args[1],args[2],args[3]));
		setDigitalTilt(hex2int16(args[4],args[5],args[6],args[7]));
	} else if (curChar == CMD_ANALOG) {
		setSourceMode(MODE_ANALOG);
	} else if (curChar == CMD_DIGITAL) {
		setSourceMode(MODE_DIGITAL);
	}
}
