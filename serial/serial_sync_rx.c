#include "serial.h"

#include "serial_common_rx.c"

void _serial_init_rx(){}

uint8_t serial_n_available(uint8_t port) {
	switch (port){
		case 0:
			return _UCSR0A & _BV(RXC0);
			
#if SERIAL_PORT_COUNT > 1			
		case 1:
			return _UCSR1A & _BV(RXC1);
#endif
	}
    return 0;
}

uint8_t serial_n_read_c(uint8_t port, char *c){
	while (!serial_n_available(port));

	switch (port){
		case 0:
			*c = _UDR0;
			
#if SERIAL_PORT_COUNT > 1			
		case 1:
			*c = _UDR1;		
#endif
		default:
			return 0; //Do nothing; trying to read null registers will cause all sorts of problems.
	}

	return 1;
}

uint8_t serial_n_read_s(uint8_t port, char *s, uint8_t len){
	uint8_t count = 0;
	char data = 0;
	
	while (count < len && serial_n_read_c(port, &data)){
		s[count++] = data;
	}
	
	return count;
}
