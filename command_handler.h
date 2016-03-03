#include <string.h>
#include <stdint.h>

#include "util/usart.h"
#include "util/analog.h"
#include "pantilt.h"

extern const char VALID_CMD[];
extern const int VALID_CMD_COUNT;
extern volatile int debugMode;

# define CSI_SEQ  '\xC3'
# define CST_SEQ  '\x5D'

# define CMD_ZERO		'Z'
# define CMD_RELMODE	'R'
# define CMD_ABSMODE	'A'
# define CMD_TOGMODE	'T'
# define CMD_GETPARAMS	'P'
# define CMD_GETRAW		'G'
# define CMD_GETSAMPLE	'Q'
# define CMD_ANALOG		'V'
# define CMD_DIGITAL	'D'
# define CMD_SETPT		'S'
# define CMD_SETPARAMS	'p'
# define CMD_DEBUG		'd'

# define OPERAND_PAN	'P'
# define OPERAND_TILT	'T'

void commandHandler();
