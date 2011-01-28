/*
 * An asynchronous implementation of the rx portion of the serial library.  This will let you 
 * avoid the need to poll for incoming data.
 */

#include "serial.h"
#include <avr/interrupt.h>

static volatile struct ring rx_buffer;

static inline uint8_t _buffer_empty(volatile struct ring *buffer){
	return (buffer->head == buffer->tail);
}

static inline uint8_t _buffer_full(volatile struct ring *buffer){
	return ((buffer->head + 1) % SERIAL_BUFFER_SIZE == buffer->tail);
}

static inline char _buffer_get(volatile struct ring *buffer){
	char c = buffer->buffer[buffer->tail];
	if (++buffer->tail >= SERIAL_BUFFER_SIZE) buffer->tail = 0;
	return c;
}

static inline void _buffer_put(volatile struct ring *buffer, char data){
	buffer->buffer[buffer->head] = data;
	if (++buffer->head >= SERIAL_BUFFER_SIZE) buffer->head = 0;
}

static inline void _serial_n_init_rx(volatile uint8_t *UCSRnB, uint8_t RXCIEn) {
	//Enable RX interrupts
	UCSRnB |= _BV(RXCIEn);

	//Enable interrupts if the NO_INTERRUPT_ENABLE define is not set.  If it is, you need to call sei() elsewhere.
	#ifndef NO_INTERRUPT_ENABLE
	sei();
	#endif	
}
void _serial_init_rx(uint8_t port){
#ifndef SERIAL_R_QUALIFIED
	_serial_n_init(&UCSRB, RXCIE);
#else
	switch (port) {
		case 0: _serial_n_init_rx(&UCSR0B, RXCIE0); break;
		#if SERIAL_PORTS > 1
		case 1: _serial_n_init_rx(&UCSR1B, RXCIE1); break;
		#endif
		#if SERIAL_PORTS > 2
		case 2: _serial_n_init_rx(&UCSR2B, RXCIE2); break;
		#endif
		#if SERIAL_PORTS > 3
		case 3: _serial_n_init_rx(&UCSR3B, RXCIE3); break;
		#endif
	}
#endif
}

static inline uint8_t _serial_n_available(volatile uint8_t *UCSRnB, uint8_t RXCIEn, volatile struct ring *rx_buffer) {
	UCSR0B &= ~_BV(RXCIE0); //Temporarily disable RX interrupts so we don't get interrupted
	uint8_t r = !_buffer_empty(&rx_buffer);
	UCSR0B |= _BV(RXCIE0); //Re-enable interrupts
	return r;
}
uint8_t serial_available() {
// TODO pass right buffers
#ifndef SERIAL_R_QUALIFIED
	_serial_n_available(&UCSRB, RXCIE, &rx_buffer);
#else
	switch (port) {
		case 0: _serial_n_available(&UCSR0B, RXCIE0, &rx_buffer); break;
		#if SERIAL_PORTS > 1
		case 1: _serial_n_available(&UCSR1B, RXCIE1, &rx_buffer); break;
		#endif
		#if SERIAL_PORTS > 2
		case 2: _serial_n_available(&UCSR2B, RXCIE2, &rx_buffer); break;
		#endif
		#if SERIAL_PORTS > 3
		case 3: _serial_n_available(&UCSR3B, RXCIE3, &rx_buffer); break;
		#endif
	}
#endif
}

uint8_t serial_read_c(char *c){
	UCSR0B &= ~_BV(RXCIE0); //Temporarily disable RX interrupts so we don't get interrupted
	if (!_buffer_empty(&rx_buffer)){
		*c = _buffer_get(&rx_buffer);
		UCSR0B |= _BV(RXCIE0); //Re-enable interrupts
		return 1;
	}
	UCSR0B |= _BV(RXCIE0); //Re-enable interrupts
	return 0;
}

uint8_t serial_read_s(char *s, uint8_t len){
	uint8_t count = 0;
	char data = 0;
	
	while (count < (len - 1) && serial_read_c(&data)){
		s[count++] = data;
	}
	
	s[count++] = 0x00;
	
	return count;
}


//Note: These defines are only a small subset of those which are available (and are
// pretty much only the chips which I personally use).  You can add more chips to 
// these defines, and as long as you match the correct chips with the correct vector
// names, it should just work.
#ifndef SERIAL_V_QUALIFIED
ISR(USART_RX_vect){
#else
ISR(USART0_RX_vect){
#endif
	char data = UDR0;
	if (!_buffer_full(&rx_buffer)){
		_buffer_put(&rx_buffer, data);
	}
}
