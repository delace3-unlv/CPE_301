// i2c.c
#define F_CPU 16000000UL

#include <avr/io.h>
#include "i2c.h"

void i2c_init(void) {
	TWSR = 0;  // prescaler = 1
	TWBR = (uint8_t)(((F_CPU/100000UL) - 16UL) / 2UL);
	TWCR = (1<<TWEN);
	// enable internal pull-ups on PC4/PC5
	DDRC  &= ~((1<<PC4)|(1<<PC5));
	PORTC |=  (1<<PC4)|(1<<PC5);
}

uint8_t i2c_start(uint8_t addr_rw) {
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	TWDR = addr_rw;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return (TWSR & 0xF8);
}

uint8_t i2c_write(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return (TWSR & 0xF8);
}

void i2c_stop(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	while (TWCR & (1<<TWSTO));
}

uint8_t i2c_read_ack(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}

uint8_t i2c_read_nack(void) {
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}
