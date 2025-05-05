/*
 * DA5.c
 *
 * Created: 4/28/2025 10:44:50 PM
 * Author : enriq
 */ 

/*
 * DA4.c
 *
 * Created: 4/5/2025 6:22:31 PM
 * Author : enriq
 */ 

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#define BAUD 9600
#define UBRR_VALUE ((F_CPU/16/BAUD) - 1)

#define PWM_DDR    DDRD
#define PWM_PORT   PORTD
#define PWM_PIN    PD6     // OC0A
#define INA1_PIN   PD4
#define INA2_PIN   PD5

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint16_t adc_result = 0;

void pwm_init(void) {
	// 1) Set PD6, PD2, PD3 to outputs
	PWM_DDR |= (1<<PWM_PIN)|(1<<INA1_PIN)|(1<<INA2_PIN);

	// 2) Pick a direction (here: INA1=1, INA2=0 ? “forward”)
	PWM_PORT |=  (1<<INA1_PIN);
	PWM_PORT &= ~(1<<INA2_PIN);

	// 3) Timer0 ? Fast?PWM, non-inverting on OC0A
	//    WGM00=1, WGM01=1 ? Fast PWM; COM0A1=1 ? non-invert
	TCCR0A = (1<<WGM00)|(1<<WGM01)|(1<<COM0A1);
	//    CS01=1 ? prescaler = 8
	TCCR0B = (1<<CS01);
}

void uart_init() {
	UBRR0H = (UBRR_VALUE >> 8);
	UBRR0L = UBRR_VALUE;
	UCSR0B = (1<<TXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

void uart_send(char c) {
	while (!(UCSR0A & (1<<UDRE0)));
		UDR0 = c;
}

void adc_init() {
	ADMUX = (1<<REFS0); 
	ADCSRA = (1<<ADEN)|(1<<ADATE)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); // Enable ADC, Auto Trigger, Interrupt
	ADCSRB = 0x00;
	DIDR0 = (1<<ADC0D); 
	ADCSRA |= (1<<ADSC); // Start ADC conversion
}

void timer_init() {
	TCCR1B |= (1<<WGM12); // CTC Mode
	OCR1A = 12499;	// OCR1A = 2499; // 10ms
	TCCR1B |= (1<<CS11)|(1<<CS10); // prescaler 64
	TIMSK1 |= (1<<OCIE1A); 
}

ISR(ADC_vect) {
	adc_result = ADC;
	OCR0A = (uint8_t)(adc_result >> 2);
}

ISR(TIMER1_COMPA_vect) {
	uint16_t mv = (adc_result * 500) / 1023;  // Convert ADC to  millivolts
	
	uart_send((mv / 10) + '0'); // Send whole number part
	uart_send('.');
	uart_send((mv % 10) + '0'); // Send decimal part
	uart_send('\n');              
}


int main(void) {
	uart_init();
	adc_init();
	timer_init();
	pwm_init();
	
	OCR0A = 128;
	 
	sei(); // set global interrupt
	
	while (1) {
		
	}
}
