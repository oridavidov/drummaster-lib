/*
 * The software driver implementation for the ST7565R LCD driver chip.  This will implement
 * the API functions declared in glcd.h.
 */

#include "glcd_st7565r.h"
#include "glcd.h"

static volatile uint8_t *_data_port = 0;
static volatile uint8_t *_clock_port = 0;
static volatile uint8_t *_a0_port = 0;
static volatile uint8_t *_reset_port = 0;

static uint8_t _data_pin = 0;
static uint8_t _clock_pin = 0;
static uint8_t _a0_pin = 0;
static uint8_t _reset_pin = 0;

static uint8_t _st7565r_buffer[LCD_WIDTH][LCD_HEIGHT >> 3];

//State of display inversion.
static uint8_t _st7565r_display_invert = 0;

/*
 * Shifts out a SPI command.  See page 24 of driver datasheet for format.
 */
void _st7565r_shift(uint8_t a0, uint8_t data){
	//If a0 = 1, then we are outputting data; a0 low is a command.
	if (a0){
		*_a0_port |= _BV(_a0_pin);
	}
	else {
		*_a0_port &= ~(_BV(_a0_pin));
	}
	
	//Send the data
	for(int8_t i = 7; i >= 0; i--) {
		//Drop the clock...
		*_clock_port &= ~_BV(_clock_pin);
	
		//...set the command (outgoing) pin...
		if (data & (_BV(i))) {
			*_data_port |= _BV(_data_pin);
		}
		else {
			*_data_port &= ~_BV(_data_pin);
		}
		
		//...wait half the clock cycle...
		_delay_us(CTRL_CLK);
		
		//...raise the clock to HIGH...
		*_clock_port |= _BV(_clock_pin);
		
		//...and wait the other half of the clock cycle
		_delay_us(CTRL_CLK);
	}	
}

void _st7565r_command(uint8_t data){
	_st7565r_shift(0, data);
}

void _st7565r_data(uint8_t data){
	_st7565r_shift(1, data);
}

/*
 * Implementation of the glcd API.  
 * 
 * Writes the a subset of the buffer to the LCD.
 */
void glcd_write_buffer_bounds(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2){
	if (x1 >= LCD_WIDTH) x1 = LCD_WIDTH - 1;
	if (x2 >= LCD_WIDTH) x1 = LCD_WIDTH - 1;
	if (y1 >= LCD_HEIGHT) y1 = LCD_HEIGHT - 1;
	if (y2 >= LCD_HEIGHT) y2 = LCD_HEIGHT - 1;
	
	for (uint8_t y = (y1 >> 3); y <= (y2 >> 3); y++){
		_st7565r_command(0xB0 | y);	//Set page
		_st7565r_command(0x10 | (x1 >> 4));	//Set column; 4 MSB
		_st7565r_command(0x00 | (x1 & 0xF));	// Set column; 4 LSB
		for (uint8_t x = x1; x <= x2; x++){	
			if (x < LCD_WIDTH && y < (LCD_HEIGHT >> 3)){
				_st7565r_data(_st7565r_buffer[x][y]);
			}
		}
	}
}

/*
 * Implementation of the glcd API.  
 * 
 * Writes the entire buffer to the LCD.  Resets the line to the start position, 
 * and iterates through the entire buffer.
 */
void glcd_write_buffer(){
	glcd_write_buffer_bounds(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	/*
	for (uint8_t y = 0; y < (LCD_HEIGHT >> 3); y++){
		_st7565r_command(0xB0 | y);	//Set page
		_st7565r_command(0x10);	//Set column
		_st7565r_command(0x00);	
		for (uint8_t x = 0; x < LCD_WIDTH; x++){	
			_st7565r_data(_st7565r_buffer[x][y]);
		}
	}
	*/
}



/*
 * Implementation of the glcd API.
 *
 * Sets the pixel at the given location in the buffer, using the given overlay mode.
 */
void set_pixel(uint8_t x, uint8_t y, uint8_t value, uint8_t overlay){
	if (x < LCD_WIDTH && y < LCD_HEIGHT){
		if (overlay == OVERLAY_NAND || value == 0){
			_st7565r_buffer[x][y >> 3] &= ~_BV(y & 0x7);
		}
		else if (overlay == OVERLAY_OR){
			_st7565r_buffer[x][y >> 3] |= _BV(y & 0x7);
		}
		else if (overlay == OVERLAY_XOR){
			_st7565r_buffer[x][y >> 3] ^= _BV(y & 0x7);
		}
	}
}

uint8_t get_pixel(uint8_t x, uint8_t y){
	return (_st7565r_buffer[x][y >> 3] & _BV(y & 0x7)) != 0;
}

void glcd_invert_display(){
	_st7565r_display_invert ^= 0x1;
	glcd_set_display_inverted(_st7565r_display_invert);
}

void glcd_set_display_inverted(uint8_t invert){
	_st7565r_display_invert = invert;
	_st7565r_command(0xA6 | _st7565r_display_invert);
}

void glcd_set_contrast(uint8_t contrast){
	_st7565r_command(0x81); //Electronic Volume Mode Set (get into contrast mode)
	_st7565r_command(contrast & 0x3F); //Set the contrast.
}

/*
 * Init the LCD.  Thanks to Lady Ada's example ST7565R driver for the correct
 * sequence of commands to init the LCD properly (https://github.com/adafruit/ST7565-LCD)
 */
void st7565r_init(volatile uint8_t *data_port, uint8_t data_pin, 
		volatile uint8_t *clock_port, uint8_t clock_pin, 
		volatile uint8_t *a0_port, uint8_t a0_pin,
		volatile uint8_t *reset_port, uint8_t reset_pin){

	_data_port = data_port;
	_clock_port = clock_port;
	_a0_port = a0_port;
	_reset_port = reset_port;

	//Set up all pins as output; PORTX - 0x1 is DDRX
	*(_data_port - 0x1) |= _BV(data_pin);
	*(_clock_port - 0x1) |= _BV(clock_pin);
	*(_a0_port - 0x1) |= _BV(a0_pin);
	*(_reset_port - 0x1) |= _BV(reset_pin);
	
	_data_pin = data_pin;
	_clock_pin = clock_pin;
	_a0_pin = a0_pin;
	_reset_pin = reset_pin;
	
	//ST7565R Driver datasheet says that "When the power is turned on, the IC 
	// internal state becomes unstable, and it is necessary to initialize it 
	// using the /RES terminal" (page 40).
	
	//Bring reset low
	*_reset_port &= ~(_BV(_reset_pin));
	
	//Wait 100ms for power to stabilize, then bring the reset pin high (/reset)
	_delay_ms(100);
	*_reset_port |= _BV(_reset_pin);
	
	//See ST7565R Driver datasheet page 51
	_delay_ms(1);
	
	//See driver datasheet page 51 for required init sequence
	_st7565r_command(0xA2);	//Set LCD bias to 1/9 (page 43)
	_st7565r_command(0xA0);	//ADC Select (page 43)
	
	//Common output mode select (page 45).  This allows you to have the LCD flipped 
	// upside down and still address things in the correct way.
	_st7565r_command(0xC8);
	
	//Set display start line
	_st7565r_command(0x40);
	
	
	//Enable power supply circuits slowly (not sure why, but Lady Ada does this...)
	_st7565r_command(0x28 | 0x4);	// turn on voltage converter (VC=1, VR=0, VF=0)
	// Wait for 50% rising
	_delay_ms(50);
	// turn on voltage regulator (VC=1, VR=1, VF=0)
	_st7565r_command(0x28 | 0x6);
	// wait >=50ms
	_delay_ms(50);
	// turn on voltage follower (VC=1, VR=1, VF=1)
	_st7565r_command(0x28 | 0x7);
	// wait a bit
	_delay_ms(10);

	// set lcd operating voltage (regulator resistor, ref voltage resistor)
	_st7565r_command(0x20 | 0x1);
	
	//Write the existing (empty) buffer to get rid of any pre-existing noise in
	// the LCD buffer.
	glcd_write_buffer();
	
	_st7565r_command(0xAF);	//LCD On
	_st7565r_command(0xA4);	//All points normal
	
	//Medium contrast
	glcd_set_contrast(0x20);
}
