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

static inline void _serial_n_init(uint8_t port, uint32_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits,
					volatile uint8_t *UBRRnH, volatile uint8_t *UBRRnL, 
					volatile uint8_t *UCSRnA, uint8_t U2Xn, uint8_t MPCMn,
					volatile uint8_t *UCSRnB, uint8_t RXENn, uint8_t TXENn, uint8_t UCSZn2,
					volatile uint8_t *UCSRnC, uint8_t UPMn0, uint8_t USBSn, uint8_t UCSZn1, uint8_t UCSZn0){
	
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
void serial_n_init(uint8_t port, uint32_t baud, uint8_t data_bits, uint8_t parity, uint8_t stop_bits){

#ifndef SERIAL_R_QUALIFIED
	_serial_n_init(port, baud, data_bits, parity, stop_bits,
					&UBRRH, &UBRRH, 
					&UCSRA, U2X, MPCM, 
					&UCSRB, RXEN, TXEN, UCSZ2,
					&UCSRC, UPM0, USBS, UCSZ1, UCSZ0);
#else
	switch (port) {
		case 0:
			_serial_n_init(port, baud, data_bits, parity, stop_bits,
							&UBRR0H, &UBRR0H, 
							&UCSR0A, U2X0, MPCM0, 
							&UCSR0B, RXEN0, TXEN0, UCSZ02,
							&UCSR0C, UPM00, USBS0, UCSZ01, UCSZ00);
			break;
		#if SERIAL_PORTS > 1
		case 1:
			_serial_n_init(port, baud, data_bits, parity, stop_bits,
							&UBRR1H, &UBRR1H, 
							&UCSR1A, U2X1, MPCM1, 
							&UCSR1B, RXEN1, TXEN1, UCSZ12,
							&UCSR1C, UPM10, USBS1, UCSZ11, UCSZ10);
			break;
		#endif
		#if SERIAL_PORTS > 2
		case 2:
			_serial_n_init(port, baud, data_bits, parity, stop_bits,
							&UBRR2H, &UBRR2H, 
							&UCSR2A, U2X2, MPCM2, 
							&UCSR2B, RXEN2, TXEN2, UCSZ22,
							&UCSR2C, UPM20, USBS2, UCSZ21, UCSZ20);
			break;
		#endif
		#if SERIAL_PORTS > 3
		case 3:
			_serial_n_init(port, baud, data_bits, parity, stop_bits,
							&UBRR3H, &UBRR3H, 
							&UCSR3A, U2X3, MPCM3, 
							&UCSR3B, RXEN3, TXEN3, UCSZ32,
							&UCSR3C, UPM30, USBS3, UCSZ31, UCSZ30);
			break;
		#endif
#endif
}
#else 
#error You must define USART vectors for your chip!  Please verify that MMCU is set correctly
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

