#include <avr/interrupt.h>
#include "analog.h"

uint8_t analog_reference = AREF_AVCC;

void initADC() {
	// turn on ADC
	ADCSRA |= _BV(ADEN);
}

void setAnalogReference(uint8_t ref) {
	// set reference type, defined in analog.h
	analog_reference = ref;
}

/**
 * Single-ended analog read
 * Returns 10-bit unsigned value
 * 
 * pin should specify 0-7 which correlate to PF0-PF7
 */
uint16_t analogRead(uint8_t pin)
{
        uint8_t low, high;

        // set the analog reference (high two bits of ADMUX) and select the
        // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
        // to 0 (the default).
        
        ADMUX = analog_reference | (pin & 0x07);
        // clear upper bit of 5-bit analog MUX value
        ADCSRB &= ~(0x08);

        // start the conversion
        ADCSRA |= _BV(ADSC);

        // ADSC is cleared when the conversion finishes
        loop_until_bit_is_clear(ADCSRA,ADSC);

        // we have to read ADCL first; doing so locks both ADCL
        // and ADCH until ADCH is read.  reading ADCL second would
        // cause the results of each conversion to be discarded,
        // as ADCL and ADCH would be locked when it completed.
        low  = ADCL;
        high = ADCH;

        // combine the two bytes
        return (high << 8) | low;
}
