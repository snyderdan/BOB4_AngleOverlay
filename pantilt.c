#include <stdint.h>
#include <avr/eeprom.h>

#include "analog.h"
#include "pantilt.h"
#include "array.h"

uint16_t sourceMode = 0;

float panSlope = 0;
uint16_t panRaw = 1000;
uint16_t panRange = 1000;
uint16_t panLLimit = 500;
uint16_t panXShift = 500;
int16_t  panYShift = 180;
uint16_t panMode = MODE_RELATIVE;
uint16_t panTempUpper = 1000;
uint16_t panTempLower = 1000;

float tiltSlope = 0;
uint16_t tiltRaw = 1000;
uint16_t tiltRange = 1000;
uint16_t tiltLLimit = 500;
uint16_t tiltXShift = 500;
int16_t  tiltYShift = 180;
uint16_t tiltMode = MODE_RELATIVE;
uint16_t tiltTempUpper = 1000;
uint16_t tiltTempLower =1000;

# define SAMPLE_COUNT 32
# define INDEX_MASK   (SAMPLE_COUNT - 1)

uint16_t pan_readings[SAMPLE_COUNT];
uint32_t pan_total = 0;
uint16_t pan_index = 0;

uint16_t tilt_readings[SAMPLE_COUNT];
uint32_t tilt_total = 0;
uint16_t tilt_index = 0;

void initPanTilt() {
	
	int i;

	setSourceMode(MODE_DIGITAL);
	
	for (i=0; i<SAMPLE_COUNT; i++) {
		pan_readings[i] = 0;
		tilt_readings[i] = 0;
	}

	uint8_t *addr = (uint8_t *) PANTILT_EEPROM_ADDR;

	// read byte from EEPROM
	uint8_t notFirstRun = eeprom_read_byte(addr);

	// if it's not the first run, then read in the previous values
	if (notFirstRun) {
		addr += 2;
		panSlope = eeprom_read_float((float *) addr);
		addr += 4;
		panLLimit = eeprom_read_word((uint16_t *) addr);
		addr += 2;
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
		tiltLLimit = eeprom_read_word((uint16_t *) addr);
		addr += 2;
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
	eeprom_update_word((uint16_t *) addr, panLLimit);
	addr += 2;
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
	eeprom_update_word((uint16_t *) addr, tiltLLimit);
	addr += 2;
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

void setSourceMode(uint16_t mode) {

	if (sourceMode == mode) {
		return;
	}

	if (mode == MODE_ANALOG) {
		sourceMode = mode;
		getRawPan = getAnalogPan;
		getRawTilt = getAnalogTilt;
	} else if (mode == MODE_DIGITAL) {
		sourceMode = mode;
		getRawPan = getDigitalPan;
		getRawTilt = getDigitalTilt;
	}
}

void setDigitalPan(uint16_t pan) {
	// Clear the bottom 2 bits because the resolution is too damn high (12-bit ADC on other end)
	panRaw = (pan >> 4) & 0x0FFF;
}

void setDigitalTilt(uint16_t tilt) {
	// Clear the bottom 2 bits because the resolution is too damn high (12-bit ADC on other end)
	tiltRaw = (tilt >> 4) & 0x0FFF;
}

uint16_t getDigitalPan() {
	return panRaw;
}

uint16_t getDigitalTilt() {
	return tiltRaw;
}

uint16_t getAnalogPan() {
	return analogRead(PAN_PORT);
}

uint16_t getAnalogTilt() {
	return analogRead(TILT_PORT);
}

uint16_t getSampledPan() {
	
	uint16_t sampledPan, sorted[SAMPLE_COUNT+1];

	pan_total -= pan_readings[pan_index];
	pan_readings[pan_index] = getRawPan();
	pan_total += pan_readings[pan_index];
	pan_index = (pan_index + 1) & INDEX_MASK;
	
	sortArray16(pan_readings, sorted, SAMPLE_COUNT);

	sampledPan = sorted[SAMPLE_COUNT/2];
	
	if (sampledPan > panTempUpper) {
		// if we have a potentially new high value, set it
		panTempUpper = sampledPan;
	} else if (sampledPan < panTempLower) {
		// if we have a potentially new low value, set it
		panTempLower = sampledPan;
	} else {
		// otherwise just return normally
		return (pan_total / SAMPLE_COUNT);
	}
	
	// potentially new accepted values
	if ((panTempUpper > (panRange + panLLimit)) || (panTempLower < panLLimit)) {
		// New high or low value!
		setPanParams(panTempLower, panTempUpper);
	}
	
	return (pan_total / SAMPLE_COUNT);
}

uint16_t getSampledTilt() {
	
	uint16_t sampledTilt, sorted[SAMPLE_COUNT+1];

	tilt_total -= tilt_readings[tilt_index];
	tilt_readings[tilt_index] = getRawTilt();
	tilt_total += tilt_readings[tilt_index];
	tilt_index = (tilt_index + 1) & INDEX_MASK;

	sortArray16(tilt_readings, sorted, SAMPLE_COUNT);

	sampledTilt = sorted[SAMPLE_COUNT/2];
	
	if (sampledTilt > tiltTempUpper) {
		// if we have a potentially new high value, set it
		tiltTempUpper = sampledTilt;
	} else if (sampledTilt < tiltTempLower) {
		// if we have a potentially new low value, set it
		tiltTempLower = sampledTilt;
	} else {
		// otherwise just return normally
		return (tilt_total / SAMPLE_COUNT);
	}
	
	// potentially new accepted values
	if ((tiltTempUpper > (tiltRange + tiltLLimit)) || (tiltTempLower < tiltLLimit)) {
		// New high or low value!
		setTiltParams(tiltTempLower, tiltTempUpper);
	}
	
	return (tilt_total / SAMPLE_COUNT);
}

/**
 * Basic logic behind the getPanAngle() and getTiltAngle() functions:
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
int getPanAngle() {
	return ((signed) (panSlope * ((panRange + (uint16_t)(getSampledPan() - panXShift)) % panRange))) - panYShift;

}

int getTiltAngle() {
	return ((signed) (tiltSlope * ((tiltRange + (uint16_t)(getSampledTilt() - tiltXShift)) % tiltRange))) - tiltYShift;
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


void setPanParams(uint16_t lower, uint16_t upper) {
	panXShift = panXShift - panLLimit + lower;
	panLLimit = lower;
	panRange = upper - lower;
	panSlope = PAN_ANGLE_RANGE / (float) (panRange);
	storePanTilt();
	
	panTempLower = lower;
	panTempUpper = upper;
}

void setTiltParams(uint16_t lower, uint16_t upper) {
	tiltXShift = tiltXShift - tiltLLimit;
	tiltRange = (int) ((float) (upper - lower) * 360.0 / TILT_ANGLE_RANGE);
	tiltLLimit = lower - ((tiltRange - (upper - lower)) / 2);
	tiltXShift += tiltLLimit;
	tiltSlope = 244.0 / (float)(upper - lower);
	storePanTilt();
	
	tiltTempLower = lower;
	tiltTempUpper = upper;
}

void calibratePan() {
	
	if (sourceMode != MODE_ANALOG) {
		return;
	}

	// set upper and lower limits to the current value so we can detect the edge
	uint16_t lower_limit = analogRead(PAN_PORT);
	uint16_t upper_limit = lower_limit;
	uint16_t current;

	// while the difference is smaller than 600
	while ((upper_limit - lower_limit) < 600) {
		current = analogRead(PAN_PORT);

		if (current > upper_limit) {
			upper_limit = current;
		} else if (current < lower_limit) {
			lower_limit = current;
		}
	}

	setPanParams(lower_limit, upper_limit);
}

void calibrateTiltLower() {
	
	if (sourceMode != MODE_ANALOG) {
		return;
	}

	uint16_t upper_limit = getSampledTilt();

	uint16_t temp_range = upper_limit - tiltXShift;

	tiltRange = (int) ((float) temp_range / (TILT_ANGLE_RANGE/360.0));
	tiltSlope = 360.0 / tiltRange;
	storePanTilt();
}

void calibrateTiltUpper() {
	
	if (sourceMode != MODE_ANALOG) {
		return;
	}
	
	tiltXShift = getSampledTilt();
}
