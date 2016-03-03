#include "array.h"

void sortArray16(uint16_t *src, uint16_t *dest, uint16_t length) {
	
	int i, j, elem;
	
	for (i=0; i<length; i++) {
		
		elem = src[i];
		j = i;
		
		while ((j > 0) && (dest[j-1] > elem)) {
			dest[j] = dest[j-1];
			j--;
		}
		
		dest[j] = elem;
	}
}
