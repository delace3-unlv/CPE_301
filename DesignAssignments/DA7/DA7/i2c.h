// i2c.h
#ifndef _I2C_H_
#define _I2C_H_

#include <stdint.h>

void    i2c_init(void);
uint8_t i2c_start(uint8_t addr_rw);
uint8_t i2c_write(uint8_t data);
void    i2c_stop(void);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);

#define I2C_WRITE(addr)  ((addr<<1)|0)
#define I2C_READ(addr)   ((addr<<1)|1)

#endif

