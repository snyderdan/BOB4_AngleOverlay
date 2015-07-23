#include <string.h>
#include <stdint.h>

#include "usart.h"
#include "pantilt.h"
#include "analog.h"

extern const char CMD_IDENTIFIER[];	// 'CSI' or control sequence inducer as-per BOB-4
extern const char VALID_CMD[];

# define CSI_SEQ  "\e["
# define CSI_LEN  2

# define CMD_ZERO 	   'Z'
# define CMD_CALIBRATE 'C'
# define CMD_RELMODE   'R'
# define CMD_ABSMODE   'A'
# define CMD_TOGMODE   'T'
# define CMD_GETPARAMS 'P'
# define CMD_GETRAW    'Q'
# define CMD_GETSAMPLE 'S'

# define OPERAND_PAN  'P'
# define OPERAND_TILT 'T'
# define OPERAND_TILTU  'U'
# define OPERAND_TILTL  'L'
# define MAX_OPERAND_COUNT 3

void commandHandler();
