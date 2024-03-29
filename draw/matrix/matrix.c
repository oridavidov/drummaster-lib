/*
 * The software driver implementation for the Digital Cave LED Matrix driver board (13 chained shift registers).  This will implement
 * the API functions declared in matrix.h.
 */


#include "matrix.h"
#include <util/delay.h>

//Data is copies to the working buffer first, and then flushed to _buffer for display by the user.  This
// prevents flickering when doing animations (and in fact any live drawing).
static uint8_t buffer[MATRIX_WIDTH][MATRIX_HEIGHT >> MATRIX_HEIGHT_SHIFT];
static uint8_t mode = 0x00;

static uint8_t twi_master_tx_writer(uint16_t i){
	if (i == 0){
		return mode;
	}
	else {
		return ((uint8_t*) buffer)[i - 1];
	}
}

void matrix_init(){
	twi_init();
	twi_attach_master_tx_writer(twi_master_tx_writer);
}

void set_pixel(uint8_t x, uint8_t y, uint8_t value, uint8_t overlay){
	if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) return;	//Bounds check

	if (overlay == OVERLAY_REPLACE){
		buffer[x][y] = value;
	}
	else if (overlay == OVERLAY_OR){
		buffer[x][y] |= value;
	}
	else if (overlay == OVERLAY_NAND){
		buffer[x][y] &= ~value;
	}
	else if (overlay == OVERLAY_XOR){
		buffer[x][y] ^= value;
	}
}

void matrix_write_buffer(){
	twi_write_to(MATRIX_DRIVER_SLAVE_ADDRESS, (uint8_t*) buffer, MATRIX_LENGTH + 1, TWI_BLOCK, TWI_STOP);
}

void matrix_set_mode(uint8_t new_mode){
	mode = new_mode;
}

uint8_t get_pixel(uint8_t x, uint8_t y){
	if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) return 0;	//Bounds check
	return buffer[x][y];
}

uint8_t* matrix_get_buffer(){
	return (uint8_t*) buffer;
}