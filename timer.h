
# ifndef CUSTOM_TIMER_H
# define CUSTOM_TIMER_H
# 	ifndef F_CPU
# 	warning Warning: F_CPU should be defined in Hz. Setting as 16MHz
// 16MHz Clock
#	define F_CPU 16000000UL
# 	endif
 
// Calculate the value needed for 
// the CTC match value in OCR1A.
#	define CTC_MATCH_OVERFLOW ((F_CPU / 1000) / 8) 
 
#	include <avr/io.h>
#	include <avr/interrupt.h>
#	include <util/atomic.h>
#	include <stdint.h>

volatile uint32_t timer1_millis;
volatile uint32_t synchronize_time;
 
ISR (TIMER1_COMPA_vect)
{
    timer1_millis++;
}

uint32_t millis () {
	
    uint32_t millis_return;

    // Ensure this cannot be disrupted
    ATOMIC_BLOCK(ATOMIC_FORCEON) {
        millis_return = timer1_millis;
    }
 
    return millis_return;
}

void delayms(uint32_t ms) {
	
	uint32_t start = millis();
	
	while ((start - millis()) < ms) {
	}
}

void synchronized(uint32_t ms) {
	
	while ((signed) (synchronize_time - millis()) < ms) {
	}
	
	synchronize_time = millis();
}

# endif
 
