#include "serial.h"
#include "serial_common_tx.c"

void _serial_init_tx(){}

void serial_n_write_s(uint8_t port, char *data){
	while (*data){
		serial_n_write_c(port, *data++);
	}
}

void serial_n_write_c(uint8_t port, char data){
	switch (port){
		case 0:
			//Nop loop to wait until last transmission has completed
			while (!(_UCSR0A & _BV(_UDRE0)));	
			_UDR0 = data;
			
#if SERIAL_PORT_COUNT > 1			
		case 1:
			//Nop loop to wait until last transmission has completed
			while (!(UCSR1A & _BV(UDRE1)));	
			_UDR1 = data;
#endif
		default:
			return; //Do nothing
	}
	
}
