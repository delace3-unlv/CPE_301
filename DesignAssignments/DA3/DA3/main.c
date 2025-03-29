/*
 * DA3.c
 *
 * Created: 3/28/2025 5:03:16 PM
 * Author : enriq
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

// Timer4 Variables
volatile int t4_count = 0;
volatile int PB3_led = 0;

// Timer3 Variables
volatile int t3_count = 0;
volatile int PB4_led = 0;

// Timer0 polling
void Normal_Timer0(void)
{
	TCNT0 = 225;             			// start count from 225
	TIFR0 |= (1 << TOV0);    			// clear overflow flag
	TCCR0B = (1 << CS01) | (1 << CS00); // prescaler 64

	while (!(TIFR0 & (1 << TOV0)))
	{
		// wait
	}
	// stop timer
	TCCR0B = 0x00;
}

// Timer3
ISR(TIMER1_COMPA_vect)
{
	t3_count++;

	if (t3_count >= 8000)
	{
		PB4_led = !PB4_led;
		if (PB4_led)
		PORTB |= (1 << PB4);
		else
		PORTB &= ~(1 << PB4);
		
		t3_count = 0;
	}
}

void CTC_Timer3(void)
{
	DDRB |= (1 << PB4);  					// PB4 output
	TCCR1B |= (1 << WGM12); 				// CTC mode => WGM12 = 1
	OCR1A = 61; 							// OCR1A = 61 => ~248us
	TCCR1B |= (1 << CS11) | (1 << CS10); 	// Prescaler=64 => CS11=1, CS10=1
	TIMSK1 |= (1 << OCIE1A); 				// Enable interrupt
}

// Timer4 ISR
ISR(TIMER2_OVF_vect)
{
	// Reload for 100us
	TCNT2 = 231;

	t4_count++;
	if (t4_count >= 10000) // ~1 second
	{
		PB3_led = !PB3_led;
		if (PB3_led)
		PORTB |= (1 << PB3);
		else
		PORTB &= ~(1 << PB3);

		t4_count = 0;
	}
}

void Normal_Timer4(void)
{
	DDRB |= (1 << PB3);   // PB3 output
	TCNT2 = 231;          // preload for 100us

	// Normal mode, prescaler=64 => TCCR2B = 0b100
	TCCR2A = 0x00;
	TCCR2B = (1 << CS22);

	// Enable OVF interrupt
	TIMSK2 |= (1 << TOIE2);
}

int main(void)
{
	CTC_Timer3();       // Initialize Timer3
	Normal_Timer4();    // Initialize Timer4

	// Set PB5 as output for Timer0 toggling
	DDRB |= (1 << PB5);

	// Enable global interrupts
	sei();

	// Timer0 poll loop for PB5 => 1.5s on/off
	while (1)
	{
		// PB5 on
		PORTB |= (1 << PB5);
		for (uint16_t i = 0; i < 12000; i++)
		{
			Normal_Timer0();
		}

		// PB5 off
		PORTB &= ~(1 << PB5);
		for (uint16_t i = 0; i < 12000; i++)
		{
			Normal_Timer0();
		}
	}
}
