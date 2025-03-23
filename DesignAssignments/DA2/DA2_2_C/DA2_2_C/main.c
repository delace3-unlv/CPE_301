/*
 * DA2_2_C.c
 *
 * Created: 3/21/2025 8:58:26 PM
 * Author : enriq
 */ 

#include <avr/io.h>


#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
	// Set PORTB5 as output (for LED)
	DDRB |= (1 << PORTB5);

	// Set PORTC1 as input (for switch)
	DDRC &= ~(1 << PORTC1);

	PORTC |= (1 << PORTC1);

	while (1)
	{
		// check if pressed (low)
		if (!(PINC & (1 << PINC1)))
		{
			// Turn LED on
			PORTB |= (1 << PORTB5);

			// Delay for 1.5 seconds
			_delay_ms(1500);

			// Turn LED off
			PORTB &= ~(1 << PORTB5);
		}
	}

	return 0;
}

