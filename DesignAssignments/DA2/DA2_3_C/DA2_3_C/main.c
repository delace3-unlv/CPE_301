/*
 * DA2_3_C.c
 *
 * Created: 3/21/2025 11:46:25 PM
 * Author : enriq
 */ 

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


void setup()
{
	DDRB |= (1 << DDB5);      // LED as output
	
	DDRD &= ~(1 << DDD2);     // PD2 as input
	PORTD |= (1 << PORTD2);   // Enable internal pull-up

	// INT0 triggers on falling edge 
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);

	EIMSK |= (1 << INT0);     // Enable INT0
	
	sei();                    // Global interrupt enable
}

int main(void)
{
	setup();

	while (1)
	{
		
	}
}

ISR(INT0_vect)
{
	PORTB |= (1 << PORTB5);   // LED ON
	_delay_ms(3000);          // Wait 3 seconds
	PORTB &= ~(1 << PORTB5);  // LED OFF
}