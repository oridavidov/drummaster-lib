/*
 * A modular implementation of serial library.  You only need to include those
 * parts which you need.
 *
 * This module defines the init methods; add rx / tx modules as you need.
 */

#include "serial.h"
#include <avr/interrupt.h>
#include <util/delay.h>

//Defined in rx / tx modules
void _serial_init_rx(uint8_t port);
void _serial_init_tx(uint8_t port);

void serial_n_init(uint8_t port, uint32_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits){
	volatile uint8_t *UBRRnH, *UBRRnL, *UCSRnA, *UCSRnB, *UCSRnC;
	uint8_t U2Xn, UCSZn0, UCSZn1, UCSZn2, MPCMn, UPMn0, USBSn, RXENn, TXENn;

	
	switch (port){
		case 0:
			UBRRnH = &_UBRR0H;
			UBRRnL = &_UBRR0L;
			UCSRnA = &_UCSR0A;
			UCSRnB = &_UCSR0B;
			UCSRnC = &_UCSR0C;
				
			U2Xn = _U2X0;
			UCSZn0 = _UCSZ00;
			UCSZn1 = _UCSZ01;
			UCSZn2 = _UCSZ02;
			MPCMn = _MPCM0;
			UPMn0 = _UPM00;
			USBSn = _USBS0;
			RXENn = _RXEN0;
			TXENn = _TXEN0;				
			
#if SERIAL_PORT_COUNT > 1			
		case 1:
#endif
		default:
			return; //Do nothing; trying to set null registers will cause all sorts of problems.
	}
	
	//Set baud rate
	uint16_t calculated_baud = (F_CPU / 16 / baud) - 1;
	*UBRRnH = calculated_baud >> 8;
	*UBRRnL = calculated_baud & 0xFF;

	//Make sure 2x and multi processor comm modes are off
	*UCSRnA &= ~(_BV(U2Xn) | _BV(MPCMn));

	//Calculate frame format
	//Init to 0; we populate this later
	*UCSRnC = 0x0;

	//Data bits
	if (data_bits == 9){
		//9 bits use an extra bit in register UCSR0B
		*UCSRnB |= _BV(UCSZn2);
		*UCSRnC |= _BV(UCSZn1) | _BV(UCSZn0);
	}
	else {
		*UCSRnC |= ((data_bits - 5) << UCSZn0);
	}

	//Parity, Stop bits
	*UCSRnC |= (parity << UPMn0) | ((stop_bits - 1) << USBSn);

	//Enable Rx / Tx if they are not disabled, and call init methods of rx/tx modules
#ifndef SERIAL_DISABLE_RX
	*UCSRnB |= _BV(RXENn);
	_serial_init_rx(port);
#endif
#ifndef SERIAL_DISABLE_TX
	*UCSRnB |= _BV(TXENn);
	_serial_init_tx(port);
#endif
}

void serial_init(uint32_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits){
	serial_n_init(0, baud, data_bits, parity, stop_bits);
}
void serial_n_init_b(uint8_t port, uint32_t baud){
	serial_n_init(port, baud, 8, 0, 1);
}
void serial_init_b(uint32_t baud){
	serial_n_init_b(0, baud);
}

