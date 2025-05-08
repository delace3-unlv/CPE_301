/*
 * DA5.c
 *
 * Created: 4/28/2025 10:44:50 PM
 * Author : enriq
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

// Motor driver pins 
#define IN1_PIN   PD4
#define IN2_PIN   PD5
#define PWM_PIN   PD6

// Variables
volatile uint16_t icr_last      = 0;
volatile uint16_t period_counts = 0;
volatile uint8_t  new_period    = 0;

uint8_t set_speed      = 0;  // 0–255, PWM 
uint8_t measured_speed = 0;  // 0–255, from measurement

// UART parsing 
char    rx_buf[4];
uint8_t rx_pos         = 0;
uint8_t override_flag  = 0;
uint8_t override_speed = 0;

// UART at 9600 baud 
static int uart_putchar(char c, FILE *stream) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = (uint8_t)c;
	return 0;
}
static FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void uart_init(void) {
	
	UBRR0  = F_CPU/16/9600 - 1;
	UCSR0B = (1<<TXEN0)|(1<<RXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
	stdout  = &uart_str;
}

// ADC 
void adc_init(void) {
	ADMUX  = (1<<REFS0);                     
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1); 
}
uint16_t adc_read(void) {
	ADCSRA |= (1<<ADSC);
	while (ADCSRA & (1<<ADSC));
	return ADC;
}

// PWM (PD6)
void pwm0_init(void) {
	DDRD  |= (1<<PWM_PIN);
	TCCR0A = (1<<COM0A1)|(1<<WGM01)|(1<<WGM00); 
	TCCR0B =  (1<<CS01);                       
}

// ICP1 (PB0)
ISR(TIMER1_CAPT_vect) {
	uint16_t ic = ICR1;
	period_counts = ic - icr_last;
	icr_last      = ic;
	new_period    = 1;
}
void icap_init(void) {
	DDRB   &= ~(1<<PB0);                             
	TCCR1B  = (1<<ICNC1)|(1<<ICES1)|(1<<CS11);       
	TIMSK1  = (1<<ICIE1);                            
}

// Motor direction pins (PD4/PD5)
void motor_dir_init(void) {
	DDRD  |= (1<<IN1_PIN)|(1<<IN2_PIN);
	PORTD |=  (1<<IN1_PIN);  // IN1=1
	PORTD &= ~(1<<IN2_PIN);  // IN2=0 ? forward
}

int main(void) {
	motor_dir_init();
	pwm0_init();
	adc_init();
	uart_init();
	icap_init();
	sei();

	while (1) {
		
		if (UCSR0A & (1<<RXC0)) {
			char c = UDR0;
			if (c >= '0' && c <= '9' && rx_pos < sizeof(rx_buf) - 1) {
				rx_buf[rx_pos++] = c;
			}
			else if ((c=='\r' || c=='\n') && rx_pos) {
				rx_buf[rx_pos] = '\0';
				uint16_t v = atoi(rx_buf);
				if (v > 255) v = 255;
				override_speed = (uint8_t)v;
				override_flag  = 1;
				rx_pos = 0;
			}
		}

		// set_speed
		if (override_flag) {
			set_speed = override_speed;
			} else {
			uint16_t raw = adc_read();
			set_speed = raw >> 2;  
		}
		OCR0A = set_speed;

		// Update measured_speed 
		if (new_period) {
			new_period = 0;
			if (period_counts) {
				uint32_t freq = F_CPU/8/period_counts;
				measured_speed = (freq > 255 ? 255 : (uint8_t)freq);
				} else {
				measured_speed = 0;
			}
		}

		// Print on BAUD 9600
		uint16_t temp = ADC;  // last ADC reading
		printf("%u %u %u\n", temp, set_speed, measured_speed);


		_delay_ms(200);
	}
}