#ifndef PWM_H
#define PWM_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

//The absolute maximum number of pins defined.  The actual 'used' count is passed
// into pwm_init, and there is no problem with having a larger number here than is
// actually used (other than a bit of wasted SRAM space for pre-defined arrays).
// By default we set this to 8 as that is a decently large number, but not so huge
// that tons of excess SRAM is wasted.  If you are low on memory, feel free to redefine
// this in your makefile (in the CDEFS variable, beside where F_CPU is defined).
#ifndef PWM_MAX_PINS
#define PWM_MAX_PINS 8
#endif


/*
 * Initializes the PWM library at a given period, using the specified ports.
 * You can have up to PWM_MAX_PINS pins defined here. 
 * 
 * To initialize this, you must pass in references to each of the PORTs, DDRs, and
 * the values of each pin.  You can do it like this:
 * 
 *    volatile uint8_t *ports[8];
 *    ports[0] = &PORTB;
 *    ports[1] = &PORTD;
 *    ....
 *    ports[5] = &PORTD;
 *
 *    uint8_t pins[8];
 *    pins[0] = 1;
 *    ....
 *    pins[5] = 3;
 *
 *    pwm_init(ports, pins, 6, 20000);
 *
 * While the code to do this is uglier than if it were to just be a function 
 * call, by allowing an arbitrary number of PWM pins, it is best to do it like this.
 *
 * The period must be less than 3354624�s (about 3.3 seconds); any more than this and
 * we just set it to the maximum.  There is no theoretical minimum, but in practice
 * you should probably keep it at 1000�s or so (1ms).
 */
void pwm_init(volatile uint8_t *ports[],
				uint8_t pins[],
				uint8_t count,
				uint32_t period);
				
				
/*
 * Sets the phase of the pin at the given index to the specified value.  The index
 * matches the index used for the arrays in pwm_init().
 */
void pwm_set_phase(uint8_t index, uint16_t phase);

/*
 * Sets the period of the PWM generator, changing the period specified in init().  
 * Applies to all active PWM pins.
 */
void pwm_set_period(uint32_t period);

/*
 * Turns off the PWM generator and stops the timer clock.  Does not clear any config
 * variables.  You can turn PWM back on again using the pwm_start() function.
 */
void pwm_stop();

/*
 * Turns on the PWM generator, starting back at 0.  This should be called after pwm_off() 
 * has been called, but pwm_init() *must* already have been called.  Calling this when
 * the generator is already on will reset the timer to 0, but otherwise will have no effect.
 *
 * If TIMER1 control registers have been modified by other libraries, you must call 
 * pwm_init() instead of pwm_start().
 */
void pwm_start();

#endif
