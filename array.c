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

uint16_t min16(uint16_t *arr, uint16_t len) {
	
	uint16_t i, localMin = arr[0];
	
	for (i=1; i<len; i++) {
		if (arr[i] < localMin) {
			localMin = arr[i];
		}
	}
	
	return localMin;
}

uint16_t max16(uint16_t *arr, uint16_t len) {
	
	uint16_t i, localMax = arr[0];
	
	for (i=1; i<len; i++) {
		if (arr[i] > localMax) {
			localMax = arr[i];
		}
	}
	
	return localMax;
}
