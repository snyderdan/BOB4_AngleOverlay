#include <stdint.h>

# define PANTILT_EEPROM_ADDR 0x400

# define PAN_PORT 0
# define PAN_ANGLE_RANGE 360.0

# define TILT_PORT 3
# define TILT_ANGLE_RANGE 270.0

# define MODE_ABSOLUTE 0
# define MODE_RELATIVE 1

void initPanTilt();
void setPanShift(uint16_t);
void setTiltShift(uint16_t);
void calibratePan();
void calibrateTiltUpper();
void calibrateTiltLower();
void storePanTilt();

int getPan();
int getTilt();

void initMode();
void setPanMode(uint16_t mode);
void setTiltMode(uint16_t mode);
void togglePanMode();
void toggleTiltMode();

extern double panSlope;
extern double tiltSlope;
extern uint16_t panXShift;
extern uint16_t panRange;
extern int16_t  panYShift;
extern uint16_t tiltXShift;
extern uint16_t tiltRange;
extern int16_t  tiltYShift;
