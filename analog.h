#include <stdint.h>

# define AREF_EXAREF 0x00	// external AREF (forced to 4.096 on BUD)
# define AREF_AVCC   0x40	// AVCC (nominal 5V)
# define AREF_1_1V   0x80	// 1.1V
# define AREF_2_56V  0xC0	// 2.56V

void setAnalogReference(uint8_t);
uint16_t analogRead(uint8_t pin);
