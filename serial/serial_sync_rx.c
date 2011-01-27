#include "serial.h"

void _serial_init_rx(){}

uint8_t _serial_n_available(volatile uint8_t *UCSRnA, uint8_t RXCn) {
    return *UCSR0A & _BV(RXC0);
}
uint8_t serial_n_available(uint8_t port) {
	switch (port) {
		case 0: return _serial_n_available(&UCSR0A, RXC0);
		case 1: return _serial_n_available(&UCSR1A, RXC1);
	}
}
uint8_t serial_available() {
    return serial_available(0)
}

uint8_t _serial_read_c(char *c, volatile uint8_t *UCSRnA, uint8_t RXCn, volatile uint8_t *UDRn){
	while (!(UCSRnA & _BV(RXCn)));
	*c = UDRn;
	return 1;
}
uint8_t serial_n_read_c(uint8_t port, char *c){
	switch (port) {
		case 0: return _serial_n_read_c(&UCSR0A, RXC0, &UDR0);
		case 1: return _serial_n_read_c(&UCSR1A, RXC1, &UDR1);
	}
}
uint8_t serial_read_c(char *c){
	return serial_n_read_c(0);
}

uint8_t serial_read_s(char *s, uint8_t len){
	uint8_t count = 0;
	char data = 0;
	
	while (count < len && serial_read_c(&data)){
		s[count++] = data;
	}
	
	return count;
}
