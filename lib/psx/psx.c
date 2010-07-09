/******************************************************************
*  PS2 controller library for AVR
*
*	Loosly based off of code by Shutter on Arduino Forums, and modified 
*	(added analog stick support) by Bill Porter (http://www.billporter.info/?p=219)
*
*	Converted to generic AVR library without C++ or Arduino references, added 
*	the ability to use any pins for input / output, fixed numerous bugs, actually
*	follow proper PS2 controller protocols, etc by Wyatt Olson (http://digitalcave.ca)
*
*	Thanks to http://store.curiousinventor.com/guides/PS2/#high_level for protocol details.
*
*  Lib version history
*	 ????		0.1		Bill Porter		-Made into Arduino library
*										-Added analog stick support
*	 2010-07	0.2		Wyatt Olson		-Converted to generic AVR-GCC library
*										-Added port / pin initializers (psx_init())
*										-Changed to use uintx_t data types instead of char
*										-Fixed many bugs (including glaring protocol violations)
*										-Code cleanup
*
*
* Usage information:
*  Connect PSX controller wires to AVR:
*		Brown:	Data
*		Blue:	Clock
*		Orange: Command
*		Yellow: Attention
*  Call psx_init() function with the port / pins which you are using
*  Call psx_read_gameport() regularly to poll and update.
*  Call psx_button() function with PSB_X argument to read the value of the given button (see
*			   psx.h for possible buttons, e.g. PSB_SELECT, PSB_L3, PSB_PAD_UP, etc)
*  Call psx_stick() function with PSS_X argument to read the analog stick value (possible
*			   values are PSS_RX, PSS_RY, PSS_LX, PSS_LY for right / left and X / Y)
* 
******************************************************************/


#include "psx.h"

//Define ports used
volatile uint8_t *_psx_data_port = 0;
volatile uint8_t *_psx_clock_port = 0;
volatile uint8_t *_psx_command_port = 0;
volatile uint8_t *_psx_attention_port = 0;

//Define the input
volatile uint8_t *_psx_data_in = 0;

//Define ddr used
volatile uint8_t *_psx_data_ddr = 0;
volatile uint8_t *_psx_clock_ddr = 0;
volatile uint8_t *_psx_command_ddr = 0;
volatile uint8_t *_psx_attention_ddr = 0;

//Define pins to use
uint8_t _psx_clock_pin = 0;
uint8_t _psx_command_pin = 0;
uint8_t _psx_attention_pin = 0;
uint8_t _psx_data_pin = 0;

//Common structure for last read data
uint8_t _psx_data[9];

/* Initialize the pins, and set the controller up to the correct mode.  This must be called
 * before any other psx_* functions are called.
 * (TODO: separate the config options into optional functions, and call based on flags,
 * to allow calling programs to determine what options to use)
 */
void psx_init(volatile uint8_t *data_port, volatile uint8_t *data_in, volatile uint8_t *data_ddr, uint8_t data_pin,
				volatile uint8_t *clock_port, volatile uint8_t *clock_ddr, uint8_t clock_pin,
				volatile uint8_t *command_port, volatile uint8_t *command_ddr, uint8_t command_pin,
				volatile uint8_t *attention_port, volatile uint8_t *attention_ddr, uint8_t attention_pin){

	//Store the ports...
	_psx_data_port = data_port;
	_psx_clock_port = clock_port;
	_psx_command_port = command_port;
	_psx_attention_port = attention_port;
	
	//... and data in....
	_psx_data_in = data_in;

	//... and output registers...
	_psx_data_ddr = data_ddr;
	_psx_clock_ddr = clock_ddr;
	_psx_command_ddr = command_ddr;
	_psx_attention_ddr = attention_ddr;

	//... and pin numbers.
	_psx_clock_pin = clock_pin;
	_psx_command_pin = command_pin;
	_psx_attention_pin = attention_pin;
	_psx_data_pin = data_pin;
	
	
	//Set clock, attention, and command pins to output mode
	*_psx_clock_ddr |= _BV(_psx_clock_pin);
	*_psx_attention_ddr |= _BV(_psx_attention_pin);
	*_psx_command_ddr |= _BV(_psx_command_pin);

	//Set data pin to input mode, and set pullup resistor
	*_psx_data_ddr &= ~(_BV(_psx_data_pin));
	*_psx_data_port |= _BV(_psx_data_pin);

	//Initialize game pad
	*_psx_clock_port |= _BV(_psx_clock_pin);
	*_psx_command_port |= _BV(_psx_command_pin);
	
	
	//Init by polling once
	psx_read_gamepad();


	//Enable joystick
	uint8_t enable_joystick_command[] = {0x01, 0x43, 0x00, 0x01, 0x00};
	_psx_send_command(enable_joystick_command, 5);


	// Lock to Analog Mode on Stick
	uint8_t lock_analog_mode_command[] = {0x01, 0x44, 0x00, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00};
	_psx_send_command(lock_analog_mode_command, 9);

	
	// Vibration (disabled; need to define vibration pins if you want to enable it)
//	uint8_t enable_vibration_command[] = {0x01, 0x4D, 0x00, 0x00, 0x01}
//	_psx_send_command(enable_vibration_command, 5);
    
	//Config controller to return all pressure values
//	uint8_t pressure_values_command[] = {0x01, 0x4F, 0x00, 0xFF, 0xFF, 0x03, 0x00, 0x00, 0x00}
//	_psx_send_command(pressure_values_command, 9);

	//Exit config mode
	uint8_t exit_config_command[] = {0x01, 0x43, 0x00, 0x00, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A};
	_psx_send_command(exit_config_command, 9);
}


uint8_t psx_button(uint16_t button) {
	uint16_t buttons = *(uint16_t*)(_psx_data + 3); //Get 2 bytes, comprising data positions 3 and 4.
	return ((~buttons & button) > 0);
}

uint8_t psx_stick(unsigned int stick) {
	return _psx_data[stick];
}

void _psx_send_command(uint8_t send_data[], uint8_t size){
	//Note: before you submit each command packet, you must set attention low; once
	// you are done each packet, return it high.  You have to toggle the line before
	// you submit another command.
	*_psx_attention_port &= ~(_BV(_psx_attention_pin));
	
	for (uint8_t i = 0; i < size; i++){
		_psx_gamepad_shift(send_data[i]);
	}
	
	*_psx_attention_port |= _BV(_psx_attention_pin);
}

/* The actual serial transfer.  Handles clock.  The PSX controller is full duplex,
 * so this will send a byte as well as receive one.
 */
uint8_t _psx_gamepad_shift(uint8_t sent) {
	uint8_t received = 0;
	for(uint8_t i = 0; i < 8; i++) {
		if (sent & (_BV(i))) {
			*_psx_command_port |= _BV(_psx_command_pin);
		}
		else {
			*_psx_command_port &= ~_BV(_psx_command_pin);
		}
		*_psx_clock_port &= ~_BV(_psx_clock_pin);
		_delay_us(CTRL_CLK);
		if(*_psx_data_in & _BV(_psx_data_pin)) {
			received |= _BV(i);
		}
		*_psx_clock_port |= _BV(_psx_clock_pin);
	}

	*_psx_command_port |= _BV(_psx_command_pin);
	_delay_us(CTRL_BYTE_DELAY);
	return received;
}

void psx_read_gamepad() {
	*_psx_command_port |= _BV(_psx_command_pin);
	*_psx_clock_port |= _BV(_psx_clock_pin);
	*_psx_attention_port &= ~_BV(_psx_attention_pin); // Get controller attention (pull pin low)
	_delay_us(CTRL_BYTE_DELAY);
	
	//Send the command to send button and joystick data;
	char dword[9] = {0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	for (uint8_t i = 0; i < 9; i++) {
		_psx_data[i] = _psx_gamepad_shift(dword[i]);
	}
	*_psx_attention_port |= _BV(_psx_attention_pin); // Done communication, return attention high
}