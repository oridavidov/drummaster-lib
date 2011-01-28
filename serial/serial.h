#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>

//The buffer size; if separate buffer sizes are not defined for rx and tx, this one is
// used for both RX and TX buffers.  Defaults to 64 bytes.  You can change it by 
// redefining SERIAL_BUFFER_SIZE in your makefile (in the CDEFS variable,
// beside where F_CPU is defined).
#ifdef SERIAL_BUFFER_SIZE
#if SERIAL_BUFFER_SIZE > 255
#undef SERIAL_BUFFER_SIZE
#define SERIAL_BUFFER_SIZE 255
#endif
#else
#define SERIAL_BUFFER_SIZE 64
#endif

#if defined(__AVR_ATtiny2313__)
	//Registers
	#define _UDR0		UDR
	#define _UBRR0H		UBRRH
	#define _UBRR0L		UBRRL
	#define _UCSR0A		UCSRA
	#define _UCSR0B		UCSRB
	#define _UCSR0C		UCSRC
	
	//Register Indices
	#define _U2X0		U2X
	#define _UCSZ00		UCSZ0
	#define _UCSZ01		UCSZ1
	#define _UCSZ02		UCSZ2
	#define _MPCM0		MPCM
	#define _UPM00		UPM0
	#define _UDRE0		UDRE
	#define _USBS0		USBS
	#define _RXEN0		RXEN
	#define _TXEN0		TXEN
	#define _RXC0		RXC

	#define SERIAL_PORT_COUNT	1

#elif defined(__AVR_ATmega48__)		|| \
	defined(__AVR_ATmega88__)		|| \
	defined(__AVR_ATmega168__)		|| \
	defined(__AVR_ATmega644__)
	//Registers
	#define _UDR0		UDR0
	#define _UBRR0H		UBRR0H
	#define _UBRR0L		UBRR0L
	#define _UCSR0A		UCSR0A
	#define _UCSR0B		UCSR0B
	#define _UCSR0C		UCSR0C

	//Register Indices
	#define _U2X0		U2X0
	#define _UCSZ00		UCSZ00
	#define _UCSZ01		UCSZ01
	#define _UCSZ02		UCSZ02
	#define _MPCM0		MPCM0
	#define _UPM00		UPM00
	#define _UDRE0		UDRE0
	#define _USBS0		USBS0
	#define _RXEN0		RXEN0
	#define _TXEN0		TXEN0
	#define _RXC0		RXC0
	
	#define SERIAL_PORT_COUNT	1
	
#elif defined(__AVR_ATmega644P__)
	//TODO

	#define SERIAL_PORT_COUNT	2
#else
#error Serial library does not currently support your MCU!  Please verify that MMCU is set correctly.
#endif


struct ring {
	//Just like a snake eating something and pooping it out...
	volatile uint8_t head;  //You put data into head...
	volatile uint8_t tail;  //...and take it off of tail.
	//If they are equal, then there is nothing in the buffer.  If 
	// (head + 1) % length == tail then the buffer is full.  The current
	// position of head points to the location where the next byte will
	// be written (and head will then be incremented after the byte is written); 
	// the position of tail points to the location of the last byte which was read,
	// and must be incremented before the next read.
	// NOTE:  You *cannot* use anything larger than uint8_t for the head / tail
	// indices; doing so will result in corrupt output, presumably because it
	// takes more than one instruction to do comparisons.
	volatile uint8_t buffer[SERIAL_BUFFER_SIZE];
};

/*
 * Initializes the USART with the given parameters.  Valid arguments include:
 *  baud: Any valid baud rate based on hardware support
 *  dataBits: Between 5 and 9 inclusive.  8 recommended.
 *  parity: 0 (no parity bit), 2 (even parity), 3 (odd parity).  0 or 2 recommended.
 *  stopBits: 1 or 2.
 *  interruptsEnabled: if true, enable interrupts for Rx Complete, Tx Complete, and 
 *       USART Data Register Empty
 */
void serial_n_init(uint8_t port, uint32_t baud, uint8_t data, uint8_t parity, uint8_t stopBits);
void serial_init(uint32_t baud, uint8_t data, uint8_t parity, uint8_t stopBits);

/*
 * Simplified init method which only asks for baud, and gives sane defaults for the rest.
 * Implementations should call serial_init with values baud, 8, 0, 1.
 */
void serial_n_init_b(uint8_t port, uint32_t baud);
void serial_init_b(uint32_t baud);

/*
 * Reads a single character from the serial port.  Pass in a pointer to a byte, and
 * the function will write a single byte to that pointer.  If the read was successful,
 * return 1; otherwise return 0.  Implementations MAY block until a byte is received.
 */
uint8_t serial_n_read_c(uint8_t port, char *c);
uint8_t serial_read_c(char *c);

/*
 * Reads a single character from the serial port.  Pass in a pointer to a byte, and
 * the function will write a single byte to that pointer.  If the read was successful,
 * return 1; otherwise return 0.  Implementations MAY block until a byte is received.
 */
uint8_t serial_n_read_c(uint8_t port, char *c);
uint8_t serial_read_c(char *c);

/*
 * Reads data into buffer of (at most) the given length - 1.  Returns the number of bytes
 * which were read.  Implementations MAY block until the entire buffer is filled.
 * The character after the last read character will be null terminated (which is why
 * the most you can read is length - 1).
 */
uint8_t serial_n_read_s(uint8_t port, char *s, uint8_t len);
uint8_t serial_read_s(char *s, uint8_t len);

/*
 * Writes a string to the serial port.  Implementations MAY block until
 * all bytes are written.
 */
void serial_n_write_s(uint8_t port, char *data);
void serial_write_s(char *data);

/*
 * Writes a single byte to the serial port.  Implementations MAY block until the 
 * write is completed.
 */
void serial_n_write_c(uint8_t port, char data);
void serial_write_c(char data);

/*
 * Checks if any bytes are available for read.  Returns 0 when no bytes are available; 
 * returns non-zero when there are any bytes avilable.  Implementations MUST NOT block.
 * Implementations MAY return the total number of bytes available.
 */
uint8_t serial_n_available(uint8_t port);
uint8_t serial_available();

#endif
