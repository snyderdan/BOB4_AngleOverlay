#include <stdint.h>

# define PANTILT_EEPROM_ADDR 0x400

# define PAN_PORT 0
# define PAN_ANGLE_RANGE 360.0

# define TILT_PORT 3
# define TILT_ANGLE_RANGE 244.0

# define MODE_ABSOLUTE 0
# define MODE_RELATIVE 1

# define MODE_DIGITAL 1
# define MODE_ANALOG  2

void initPanTilt();				// initialize pan and tilt - load settings from EEPROM
void setPanShift(uint16_t);		// set zero point essentially - x-shift
void setTiltShift(uint16_t);
void calibratePan();			// pan calibration mode
void calibrateTiltUpper();		// calibrate upper limit of tilt
void calibrateTiltLower();		// calibrate lower limit of tilt
void storePanTilt();			// store pan and tilt settings in EEPROM

void setDigitalPan(uint16_t);	// set digital pan
void setDigitalTilt(uint16_t);	// set digital tilt
uint16_t getAnalogPan();		// analog value of pan
uint16_t getDigitalPan();		// digital source for pan
uint16_t getAnalogTilt();		// analog value of tilt
uint16_t getDigitalTilt();		// digital value of tilt
uint16_t (*getRawPan)();		// get raw pan value based on mode
uint16_t (*getRawTilt)();		// get raw tilt based on mode
int getPanAngle();				// get calculated pan angle	
int getTiltAngle();				// get calculated tilt angle
uint16_t getSampledPan();		// get sampled pan value
uint16_t getSampledTilt();		// get sampled tilt value

void initMode();
void setSourceMode(uint16_t mode);
void setPanMode(uint16_t mode);
void setTiltMode(uint16_t mode);
void togglePanMode();
void toggleTiltMode();

extern uint16_t panRaw;
extern uint16_t tiltRaw;
extern double panSlope;
extern double tiltSlope;
extern uint16_t panXShift;
extern uint16_t panRange;
extern int16_t  panYShift;
extern uint16_t tiltXShift;
extern uint16_t tiltRange;
extern int16_t  tiltYShift;
extern uint16_t sourceMode;
