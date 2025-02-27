#ifndef KEYPAD_HEADER
#define	KEYPAD_HEADER

#include "twi_pca.h"
#include <stdint.h>


uint16_t scan_row(uint8_t row);
uint16_t scan_keypad();
uint16_t scan_keypad_rising_edge();
 
uint8_t keypad_to_ascii();
#endif	/* KEYPAD_HEADER */
