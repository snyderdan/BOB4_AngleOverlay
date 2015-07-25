#include <string.h>
#include <stdint.h>

#include "usart.h"
#include "pantilt.h"
#include "analog.h"

extern const char VALID_CMD[];
extern const int VALID_CMD_COUNT;

# define CSI_SEQ  '\xC3'
# define CSE_SEQ  '\x5D'

# define CMD_ZERO 	   'Z'
# define CMD_CALIBRATE 'C'
# define CMD_RELMODE   'R'
# define CMD_ABSMODE   'A'
# define CMD_TOGMODE   'T'
# define CMD_GETPARAMS 'P'
# define CMD_GETRAW    'G'
# define CMD_GETSAMPLE 'Q'
# define CMD_ANALOG    'V'
# define CMD_DIGITAL   'D'
# define CMD_SETPT     'S'

# define OPERAND_PAN  'P'
# define OPERAND_TILT 'T'
# define OPERAND_TILTU  'U'
# define OPERAND_TILTL  'L'

void commandHandler();
