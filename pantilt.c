#include <stdint.h>
#include <math.h>
#include <avr/eeprom.h>

#include "util/analog.h"
#include "util/array.h"
#include "util/modulus.h"
#include "pantilt.h"

uint16_t sourceMode;
volatile uint16_t digitalPanUpdate = 0;
volatile uint16_t digitalTiltUpdate = 0;

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
uint16_t tiltULimit;	// real upper limit, not equal to range
uint16_t tiltXShift;
int16_t  tiltYShift;
uint16_t tiltMode;
uint16_t tiltRaw = 1000;
uint16_t tiltTempUpper = 1000;
uint16_t tiltTempLower = 1000;

# define SAMPLE_COUNT 16
# define INDEX_MASK   (SAMPLE_COUNT - 1)

uint16_t pan_real_readings[SAMPLE_COUNT];
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
uint16_t EEMEM panYShiftEE = 179;
uint16_t EEMEM panModeEE = MODE_RELATIVE;

float EEMEM tiltSlopeEE = 0.205;
uint16_t EEMEM tiltRangeEE = 1000;
uint16_t EEMEM tiltLLimitEE = 500;
uint16_t EEMEM tiltULimitEE = 1000;
uint16_t EEMEM tiltXShiftEE = 1000;
uint16_t EEMEM tiltYShiftEE = 180;
uint16_t EEMEM tiltModeEE = MODE_RELATIVE;

/***********************************************************************/

void initPanTilt() {
	
	int i;
	
	for (i=0; i<SAMPLE_COUNT; i++) {
		pan_real_readings[i] = panRaw;
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
	tiltULimit = eeprom_read_word(&tiltULimitEE);
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
		panYShift = 179;
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
		tiltYShift = 360;
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
	
	uint16_t sampledValue, realMin, realMax, sorted[SAMPLE_COUNT];
	
	// if pan has not been updated, return the current sample 
	if (digitalPanUpdate == 0) {
		return pan_total / SAMPLE_COUNT;
	}
	
	// if a new pan value is present, add it to the sample and notify for next iteration
	digitalPanUpdate = 0;
	
	// update real pan readings to detect changes in limits
	pan_real_readings[pan_index] = getRawPan();
	
	// sort array to push spikes to the edges
	sortArray16(pan_real_readings, sorted, SAMPLE_COUNT);
	// grab the middle 6 samples and get the min and max from them to check for new limits
	// yes, we may miss an actual limit this way, but that is better than detecting a 
	// spike and using that as the limit. It will be close enough.
	realMin = min16(sorted + (SAMPLE_COUNT / 2) - 2, 4);
	realMax = max16(sorted + (SAMPLE_COUNT / 2) - 2, 4);
	
	if (realMax > panTempUpper) {
		// if we have a potentially new high value, set it
		panTempUpper = realMax;
	} else if (realMin < panTempLower) {
		// if we have a potentially new low value, set it
		panTempLower = realMin;
	} 
	
	// potentially new accepted values
	if ((panTempUpper > (panRange + panLLimit)) || (panTempLower < panLLimit)) {
		// New high or low value!
		setPanParams(panTempLower, panTempUpper);
	}

	// update synthetic readings which will actually be returned
	pan_total -= pan_readings[pan_index];
	pan_readings[pan_index] = getRawPan();
	pan_total += pan_readings[pan_index];
	pan_index = (pan_index + 1) & INDEX_MASK;
	
	sampledValue = pan_total / SAMPLE_COUNT;
	
	// if the difference between the current value and the sampled value is drastic
	if ((getRawPan() - sampledValue) > (panRange * 0.7)) {
		// we know we rolled over the zero point, so to avoid having the angle go wacky
		// we fill the sampling with the current value
		int i, current = getRawPan();
		pan_total = 0;
		for (i=0; i<SAMPLE_COUNT; i++) {
			pan_readings[i] = current;
			pan_total += current;
		}
		
		sampledValue = pan_total / SAMPLE_COUNT;
	}
	
	return sampledValue;
}

uint16_t getSampledTilt() {
	
	uint16_t sampledValue;
	
	// if tilt has not been updated, return the current sample 
	if (digitalTiltUpdate == 0) {
		return tilt_total / SAMPLE_COUNT;
	}
	
	// if a new tilt value is present, add it to the sample and notify for next iteration
	digitalTiltUpdate = 0;

	tilt_total -= tilt_readings[tilt_index];
	tilt_readings[tilt_index] = getRawTilt();
	tilt_total += tilt_readings[tilt_index];
	tilt_index = (tilt_index + 1) & INDEX_MASK;
	
	sampledValue = tilt_total / SAMPLE_COUNT;
	
	if (sampledValue > tiltTempUpper) {
		// if we have a potentially new high value, set it
		tiltTempUpper = sampledValue;
	} else if (sampledValue < tiltTempLower) {
		// if we have a potentially new low value, set it
		tiltTempLower = sampledValue;
	}
	
	// potentially new accepted values
	if ((tiltTempUpper > tiltULimit) || (tiltTempLower < tiltLLimit)) {
		// New high or low value!
		setTiltParams(tiltTempLower, tiltTempUpper);
	}
	
	// if the difference between the current value and the sampled value is drastic
	if ((getRawTilt() - sampledValue) > (tiltRange * 0.7)) {
		// we know we rolled over the zero point, so to avoid having the angle go wacky
		// we fill the sampling with the current value
		int i, current = getRawTilt();
		tilt_total = 0;
		for (i=0; i<SAMPLE_COUNT; i++) {
			tilt_readings[i] = current;
			tilt_total += current;
		}
		
		return tilt_total / SAMPLE_COUNT;
	}
	
	return sampledValue;
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
	// Always round Pan down since it goes full 360. 
	// If you round away from 0, as tilt does, 0 becomes near impossible to hit
	// If you round up, the range may be bound to [1,360] which is far less desirable than [0,359]
	double angle = panSlope * modulo(panRange + (signed)(getSampledPan() - panXShift), panRange);
	return (int) floor(angle)  - panYShift;
}

int getTiltAngle() {
	double angle = tiltSlope * modulo(tiltRange + (signed)(getSampledTilt() - tiltXShift), tiltRange);
	if ((angle - tiltYShift) > 0)
		return (int) ceil(angle) + tiltYShift;
	else
		return (int) floor(angle) + tiltYShift;
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
	
	tiltXShift = tiltXShift - tiltLLimit + lower;
	tiltRange = (int) ((double)(upper - lower) / (TILT_ANGLE_RANGE / 360.0));
	tiltLLimit = lower;
	tiltULimit = upper;
	tiltSlope = - (TILT_ANGLE_RANGE / (double) (upper - lower));
	
	eeprom_write_word(&tiltXShiftEE, tiltXShift);
	eeprom_write_word(&tiltRangeEE, tiltRange);
	eeprom_write_word(&tiltLLimitEE, tiltLLimit);
	eeprom_write_float(&tiltSlopeEE, tiltSlope);
	eeprom_write_word(&tiltULimitEE, tiltULimit);
	
	tiltTempLower = lower;
	tiltTempUpper = upper;
}
