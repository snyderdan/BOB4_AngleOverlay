#include <stdio.h>

#include "command_handler.h"

const char VALID_CMD[] = {CMD_ZERO, CMD_RELMODE, CMD_ABSMODE,
	 CMD_TOGMODE, CMD_GETPARAMS, CMD_GETRAW, CMD_GETSAMPLE, CMD_SETPT,
	 CMD_ANALOG, CMD_DIGITAL, CMD_SETPARAMS, CMD_DEBUG};

const uint8_t VALID_CMD_ARGC[] = {2,2,2,2,0,0,0,8,0,0,9,0};

# define MAX_ARG_COUNT 9	// Number of args used in setparams command
	
const int VALID_CMD_COUNT = 12;

int debugMode = 0;

char CMDIN[256];

uint8_t filled = 0;
uint8_t processed = 0;

uint16_t hex2int16(char hh, char hl, char lh, char ll) {
	
	uint16_t value = 0;
	
	value |= (((hh <= '9') ? hh-'0' : hh-'7') << 12);
	value |= (((hl <= '9') ? hl-'0' : hl-'7') << 8);
	value |= (((lh <= '9') ? lh-'0' : lh-'7') << 4);
	value |= ((ll <= '9')  ? ll-'0' : ll-'7');
	
	return value;
}

void commandHandler() {
	
	int i, argc, available;
	char curChar, string[256];
	// we add 1 as a safety net because I'm too lazy to write my arg grabbing loop any better
	char args[MAX_ARG_COUNT+1];

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
	
	available -= 2;		// subtract CSI and command byte
	
	// isolate arguments
	for (i=0; (i<available) && (i<=argc); i++) {
		curChar = CMDIN[(uint8_t)(processed+i+2)];
		if (curChar == CST_SEQ) {	// termination? 
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
	curChar = CMDIN[(uint8_t)(processed + 1)];	// get command byte
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
		sprintf(string, "\n\rPAN X SHIFT:  %5u  PAN Y SHIFT:  %-3d deg\
						 \n\rPAN RANGE:    %5u  PAN SLOPE:    %f \
						 \n\rPAN MINIMUM:  %5u \
					 \n\r\n\rTILT X SHIFT: %5u  TILT Y SHIFT: %-3d deg\
						 \n\rTILT RANGE:   %5u  TILT SLOPE:   %f  \
						 \n\rTILT MINIMUM: %5u \n\r",
				 panXShift, panYShift, panRange, panSlope, panLLimit,
				 tiltXShift, tiltYShift, tiltRange, tiltSlope, tiltLLimit
				);
		SendString2(string);
	} else if (curChar == CMD_GETRAW) {
		sprintf(string, "\n\rCURRENT RAW PAN: %5u  CURRENT RAW TILT: %u\n\r", 
				getRawPan(), getRawTilt()
				);
		SendString2(string);
	} else if (curChar == CMD_GETSAMPLE) {
		sprintf(string, "\n\rCURRENT SAMPLED PAN: %5u  CURRENT SAMPLED TILT: %u\n\r", 
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
	} else if (curChar == CMD_SETPARAMS) {
		if (argc != 9) {
			return;
		}
		
		if (args[0] == OPERAND_PAN) {
			setPanParams(hex2int16(args[1],args[2],args[3],args[4]), hex2int16(args[5],args[6],args[7],args[8]));
		} else if (args[0] == OPERAND_TILT) {
			setTiltParams(hex2int16(args[1],args[2],args[3],args[4]), hex2int16(args[5],args[6],args[7],args[8]));
		}
	} else if (curChar == CMD_DEBUG) {
		debugMode = !debugMode;
	}
}
