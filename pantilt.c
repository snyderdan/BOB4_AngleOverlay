#include <stdint.h>
#include <avr/eeprom.h>

#include "analog.h"
#include "pantilt.h"
#include "array.h"

uint16_t sourceMode;

float panSlope;
uint16_t panRange;
uint16_t panLLimit;
uint16_t panXShift;
int16_t  panYShift;
uint16_t panMode;
uint16_t panRaw = 1000;
uint16_t panTempUpper = 1000;
uint16_t panTempLower = 1000;

float tiltSlope;
uint16_t tiltRange;
uint16_t tiltLLimit;
uint16_t tiltXShift;
int16_t  tiltYShift;
uint16_t tiltMode;
uint16_t tiltRaw = 1000;
uint16_t tiltTempUpper = 1000;
uint16_t tiltTempLower = 1000;

# define SAMPLE_COUNT 32
# define INDEX_MASK   (SAMPLE_COUNT - 1)

uint16_t pan_readings[SAMPLE_COUNT];
uint32_t pan_total = 0;
uint16_t pan_index = 0;

uint16_t tilt_readings[SAMPLE_COUNT];
uint32_t tilt_total = 0;
uint16_t tilt_index = 0;

/**
 * EEPROM definitions so that the compiler can handle the addresses and allocation
 * 
 ***********************************************************************/
 
uint16_t EEMEM sourceModeEE = MODE_DIGITAL;

float EEMEM panSlopeEE = 0.205;
uint16_t EEMEM panRangeEE = 1000;
uint16_t EEMEM panLLimitEE = 500;
uint16_t EEMEM panXShiftEE = 1000;
uint16_t EEMEM panYShiftEE = 180;
uint16_t EEMEM panModeEE = MODE_RELATIVE;

float EEMEM tiltSlopeEE = 0.205;
uint16_t EEMEM tiltRangeEE = 1000;
uint16_t EEMEM tiltLLimitEE = 500;
uint16_t EEMEM tiltXShiftEE = 1000;
uint16_t EEMEM tiltYShiftEE = 180;
uint16_t EEMEM tiltModeEE = MODE_RELATIVE;

/***********************************************************************/

void initPanTilt() {
	
	int i;
	
	for (i=0; i<SAMPLE_COUNT; i++) {
		pan_readings[i] = panRaw;
		tilt_readings[i] = tiltRaw;
		pan_total += panRaw;
		tilt_total += tiltRaw;
	}
	
	getRawPan = getDigitalPan;
	getRawTilt = getDigitalTilt;

	setSourceMode(eeprom_read_word(&sourceModeEE));
	panSlope = eeprom_read_float(&panSlopeEE);
	panLLimit = eeprom_read_word(&panLLimitEE);
	panRange = eeprom_read_word(&panRangeEE);
	panMode = eeprom_read_word(&panModeEE);
	panXShift = eeprom_read_word(&panXShiftEE);
	panYShift = eeprom_read_word(&panYShiftEE);
	tiltSlope = eeprom_read_float(&tiltSlopeEE);
	tiltLLimit = eeprom_read_word(&tiltLLimitEE);
	tiltRange = eeprom_read_word(&tiltRangeEE);
	tiltMode = eeprom_read_word(&tiltModeEE);
	tiltXShift = eeprom_read_word(&tiltXShiftEE);
	tiltYShift = eeprom_read_word(&tiltYShiftEE);
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
	} else {
		return;
	}

	eeprom_write_word(&panYShiftEE, panYShift);
	eeprom_write_word(&panXShiftEE, panXShift);
	eeprom_write_word(&panModeEE, panMode);
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
	} else {
		return;
	}

	eeprom_write_word(&tiltYShiftEE, tiltYShift);
	eeprom_write_word(&tiltXShiftEE, tiltXShift);
	eeprom_write_word(&tiltModeEE, tiltMode);
}

void toggleTiltMode() {
	setTiltMode(!tiltMode);
}

void setSourceMode(uint16_t mode) {

	if (mode == MODE_ANALOG) {
		sourceMode = mode;
		getRawPan = getAnalogPan;
		getRawTilt = getAnalogTilt;
	} else if (mode == MODE_DIGITAL) {
		sourceMode = mode;
		getRawPan = getDigitalPan;
		getRawTilt = getDigitalTilt;
	}
	
	eeprom_write_word(&sourceModeEE, sourceMode);
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

	eeprom_write_word(&panXShiftEE, panXShift);
}

void setTiltShift(uint16_t value) {

	if (tiltMode == MODE_RELATIVE) {
		tiltXShift = value + (tiltRange / 2);
	} else {
		tiltXShift = value;
	}

	eeprom_write_word(&tiltXShiftEE, tiltXShift);
}


void setPanParams(uint16_t lower, uint16_t upper) {
	panXShift = panXShift - panLLimit + lower;
	panLLimit = lower;
	panRange = upper - lower;
	panSlope = PAN_ANGLE_RANGE / (float) (panRange);
	
	eeprom_write_word(&panXShiftEE, panXShift);
	eeprom_write_word(&panLLimitEE, panLLimit);
	eeprom_write_word(&panRangeEE, panRange);
	eeprom_write_float(&panSlopeEE, panSlope);
	
	panTempLower = lower;
	panTempUpper = upper;
}

void setTiltParams(uint16_t lower, uint16_t upper) {
	tiltXShift = tiltXShift - tiltLLimit;
	tiltRange = (int) ((float) (upper - lower) * 360.0 / TILT_ANGLE_RANGE);
	tiltLLimit = lower - ((tiltRange - (upper - lower)) / 2);
	tiltXShift += tiltLLimit;
	tiltSlope = 244.0 / (float)(upper - lower);
	
	eeprom_write_word(&tiltXShiftEE, tiltXShift);
	eeprom_write_word(&tiltRangeEE, tiltRange);
	eeprom_write_word(&tiltLLimitEE, tiltLLimit);
	eeprom_write_float(&tiltSlopeEE, tiltSlope);
	
	tiltTempLower = lower;
	tiltTempUpper = upper;
}
