/*
 * This file is included by serial_*_rx.c, and includes the implementations
 * for all non-port-specific functions (which just delegate to the 
 * port-specific equivalent, passing 0 as the port).
 */

uint8_t serial_read_c(char *c){
	return serial_n_read_c(0, c);
}
uint8_t serial_available() {
    return serial_n_available(0);
}
uint8_t serial_read_s(char *s, uint8_t len){
	return serial_n_read_s(0, s, len);
}