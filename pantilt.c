#include <stdint.h>
#include <avr/eeprom.h>

#include "analog.h"
#include "pantilt.h"

double panSlope = 0.43956044;
uint16_t panRange = 819;
uint16_t panXShift = 102;
int16_t  panYShift = 180;
uint16_t panMode = MODE_RELATIVE;

double tiltSlope = 0.43956044;
uint16_t tiltRange = 613;
uint16_t tiltXShift = 205;
int16_t  tiltYShift = 180;
uint16_t tiltMode = MODE_RELATIVE;

void initPanTilt() {
	
	return;
	
	uint8_t *addr = (uint8_t *) PANTILT_EEPROM_ADDR;
	
	// read byte from EEPROM
	uint8_t notFirstRun = eeprom_read_byte(addr);
	
	// if it's not the first run, then read in the previous values
	if (notFirstRun) {
		addr += 2;
		panSlope = eeprom_read_float((float *) addr);
		addr += 4;
		panXShift = eeprom_read_word((uint16_t *) addr);
		addr += 2;
		panRange = eeprom_read_word((uint16_t *) addr);
		addr += 2;
		panYShift = eeprom_read_word((uint16_t *) addr);
		addr += 2;
		setPanMode(eeprom_read_word((uint16_t *) addr));
		addr += 2;
		tiltSlope = eeprom_read_float((float *) addr);
		addr += 4;
		tiltXShift = eeprom_read_word((uint16_t *) addr);
		addr += 2;
		tiltRange = eeprom_read_word((uint16_t *) addr);
		addr += 2;
		tiltYShift = eeprom_read_word((uint16_t *) addr);
		addr += 2;
		setTiltMode(eeprom_read_word((uint16_t *) addr));
	} else {
		storePanTilt();
	}
}

void storePanTilt() {
	
	uint8_t *addr = (uint8_t *) PANTILT_EEPROM_ADDR;
	
	eeprom_update_byte(addr, 1);
	addr += 2;
	eeprom_update_float((float *) addr, panSlope);
	addr += 4;
	eeprom_update_word((uint16_t *) addr, panXShift);
	addr += 2;
	eeprom_update_word((uint16_t *) addr, panRange);
	addr += 2;
	eeprom_update_word((uint16_t *) addr, panYShift);
	addr += 2;
	eeprom_update_word((uint16_t *) addr, panMode);
	addr += 2;
	eeprom_update_float((float *) addr, tiltSlope);
	addr += 4;
	eeprom_update_word((uint16_t *) addr, tiltXShift);
	addr += 2;
	eeprom_update_word((uint16_t *) addr, tiltRange);
	addr += 2;
	eeprom_update_word((uint16_t *) addr, tiltYShift);
	addr += 2;
	eeprom_update_word((uint16_t *) addr, tiltMode);
}

void setPanMode(uint16_t mode) {
	
	// return if we are already in said mode
	if (panMode == mode) {
		return;
	}
	
	if (mode == MODE_RELATIVE) {
		panXShift -= (panRange / 2);
		panYShift = 180;
		panMode = MODE_RELATIVE;
	} else if (mode == MODE_ABSOLUTE) {
		panXShift += (panRange / 2);
		panYShift = 0;
		panMode = MODE_ABSOLUTE;
	}
	
	storePanTilt();
}

void togglePanMode() {
	setPanMode(!panMode);
}

void setTiltMode(uint16_t mode) {
		// return if we are already in said mode
	if (tiltMode == mode) {
		return;
	}
	
	if (mode == MODE_RELATIVE) {
		tiltXShift -= (tiltRange / 2);
		tiltYShift = 180;
		tiltMode = MODE_RELATIVE;
	} else if (mode == MODE_ABSOLUTE) {
		tiltXShift += (tiltRange / 2);
		tiltYShift = 0;
		tiltMode = MODE_ABSOLUTE;
	}
	
	storePanTilt();
}

void toggleTiltMode() {
	setTiltMode(!tiltMode);
}

/**
 * Basic logic behind the getPan() and getTilt() functions:
 * 
 * 	 1) Read analog input and subtract the lower limit of possible inputs so
 *  that the domain is shifted to start at zero. 
 *   2) Subtract the shift (or zero-point) which determines where the saw
 *  tooth will be on the graph. This causes an issue when an x-input is less
 *  than the zero-point as the result is negative, which is handled in the next step.
 *   3) Add the range (or the 'upper limit' of values that will result from step 1)
 *  to properly adjust negative numbers. Positive numbers should just yield themselves,
 *  whereas negative numbers should be added to the upper limit.
 *   4) Perform a modulus operation which will divide out the upper limit which was
 *  added if the result from step 2 was positive, or leave it properly adjusted
 *  if the result from step 2 was negative. 
 *   5) Multiply the adjusted result from step 4 to get the value between 0-360
 *   6) Shift the output depending on what mode we are in
 * 
 */
int getPan() {
	return (int) (panSlope * (((signed) panRange + ((signed) analogRead(PAN_PORT) - (signed) panXShift)) % (signed) panRange)) - panYShift;
	
}

int getTilt() {
	return (int) (tiltSlope * (((signed) tiltRange + ((signed) analogRead(TILT_PORT) - (signed) tiltXShift)) % (signed) tiltRange)) - tiltYShift;
}

void setPanShift(uint16_t value) {
	
	if (panMode == MODE_RELATIVE) {
		panXShift = value + (panRange / 2);
	} else {
		panXShift = value;
	}
	
	storePanTilt();
}

void setTiltShift(uint16_t value) {
	
	if (tiltMode == MODE_RELATIVE) {
		tiltXShift = value + (tiltRange / 2);
	} else {
		tiltXShift = value;
	}
	
	storePanTilt();
}

void calibratePan() {
	
	// set upper and lower limits to the current value so we can detect the edge
	uint16_t lower_limit = analogRead(PAN_PORT);
	uint16_t upper_limit = lower_limit;
	uint16_t current;
	
	// while the difference is smaller than 744 
	// this assumes the biggest variation from 0 or 3.3V will be +-0.45V
	while ((upper_limit - lower_limit) < 744) {
		current = analogRead(PAN_PORT);
		
		if (current > upper_limit) {
			upper_limit = current;
		} else if (current < lower_limit) {
			lower_limit = current;
		}
	}
	
	panRange = upper_limit - lower_limit;
	panSlope = PAN_ANGLE_RANGE / (float) (panRange);
	storePanTilt();
}

void calibrateTiltUpper() {
	
	uint16_t upper_limit = analogRead(TILT_PORT);
	
	uint16_t temp_range = upper_limit - tiltXShift;
	
	tiltRange = (int) ((float) temp_range / (TILT_ANGLE_RANGE/360.0));
	tiltSlope = 360.0 / tiltRange;
	storePanTilt();
}

void calibrateTiltLower() {
	tiltXShift = analogRead(TILT_PORT);
}
