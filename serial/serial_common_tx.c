/*
 * This file is included by serial_*_tx.c, and includes the implementations
 * for all non-port-specific functions (which just delegate to the 
 * port-specific equivalent, passing 0 as the port).
 */

void serial_write_s(char *data){
	serial_n_write_s(0, data);
}

void serial_write_c(char data){
	serial_n_write_c(0, data);
}
