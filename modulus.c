#include <math.h>

# include "modulus.h"

int modulo(int dividend, int divisor) {
	return (int) (dividend - divisor * (int) floor(dividend / (float) divisor));
}
