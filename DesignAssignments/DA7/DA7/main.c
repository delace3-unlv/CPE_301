/*
 * DA7.c
 *
 * Created: 5/11/2025 2:47:03 PM
 * Author : enriq
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "i2c.h"
#include "lcd.h"
#include "font.h"

// BMI160 I2C address and registers
#define BMI160_ADDR              0x68
#define CMD_REG                  0x7E
#define ACC_CONF_REG             0x40
#define GYR_CONF_REG             0x42
#define DATA_REG                 0x12

// Sensitivities and timing
#define ACCELEROMETER_SENSITIVITY 16384.0f   // LSB/g
#define GYROSCOPE_SENSITIVITY     16.4f      // LSB/(°/s)
#define DT                        0.01f

static int16_t accData[3], gyrData[3];
static float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;

// Initialize UART
static void uart_init(void) {
	UBRR0 = 103;
	UCSR0B = (1<<TXEN0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
}

static void uart_tx(char c) {
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

static void uart_print_str(const char *s) {
	while (*s) uart_tx(*s++);
}

// Initialize BMI160 sensor
static void bmi160_init(void) {
	// Soft reset
	i2c_start((BMI160_ADDR<<1)|0);
	i2c_write(CMD_REG);
	i2c_write(0xB6);
	i2c_stop();
	_delay_ms(100);

	// Accel: 2g, 100Hz
	i2c_start((BMI160_ADDR<<1)|0);
	i2c_write(ACC_CONF_REG);
	i2c_write(0x28);
	i2c_stop();

	// Gyro: 250°/s, 100Hz
	i2c_start((BMI160_ADDR<<1)|0);
	i2c_write(GYR_CONF_REG);
	i2c_write(0x28);
	i2c_stop();

	// Normal modes
	i2c_start((BMI160_ADDR<<1)|0);
	i2c_write(CMD_REG);
	i2c_write(0x15);
	i2c_stop();
	_delay_ms(50);
	i2c_start((BMI160_ADDR<<1)|0);
	i2c_write(CMD_REG);
	i2c_write(0x11);
	i2c_stop();
	_delay_ms(50);
}

// Read 6-axis data
static void bmi160_read(void) {
	i2c_start((BMI160_ADDR<<1)|0);
	i2c_write(DATA_REG);
	i2c_start((BMI160_ADDR<<1)|1);

	// Accel XYZ
	for (int i = 0; i < 3; i++) {
		uint8_t lo = i2c_read_ack();
		uint8_t hi = i2c_read_ack();
		accData[i] = (int16_t)(hi<<8 | lo);
	}
	// Gyro XYZ
	for (int i = 0; i < 2; i++) {
		uint8_t lo = i2c_read_ack();
		uint8_t hi = i2c_read_ack();
		gyrData[i] = (int16_t)(hi<<8 | lo);
	}
	uint8_t lo = i2c_read_ack();
	uint8_t hi = i2c_read_nack();
	gyrData[2] = (int16_t)(hi<<8 | lo);
	i2c_stop();
}

// Complementary filter
static void ComplementaryFilter(void) {
	float pitchAcc = atan2f((float)accData[1], (float)accData[2]) * 180.0f / M_PI;
	float rollAcc  = atan2f((float)accData[0], (float)accData[2]) * 180.0f / M_PI;
	pitch = pitch*0.98f + pitchAcc*0.02f;
	roll  = roll*0.98f  + rollAcc*0.02f;
	yaw  += ((float)gyrData[2] / GYROSCOPE_SENSITIVITY) * DT;
}

int main(void) {
	char uart_vals1[128];
	char uart_vals2[64];
	char sabuf[8], sbbuf[8], scbuf[8];
	char gxbuf[8], gybuf[8], gzbuf[8];
	char rbuf[8], pbuf[8], ybuf[8];

	// Init peripherals
	uart_init();
	i2c_init();
	bmi160_init();
	lcd_init(0xAF);
	lcd_clrscr();
	lcd_display();
	sei();

	while (1) {
		// Read and compute
		bmi160_read();
		ComplementaryFilter();
		float ax = accData[0] / ACCELEROMETER_SENSITIVITY;
		float ay = accData[1] / ACCELEROMETER_SENSITIVITY;
		float az = accData[2] / ACCELEROMETER_SENSITIVITY;
		float gx = gyrData[0] / GYROSCOPE_SENSITIVITY;
		float gy = gyrData[1] / GYROSCOPE_SENSITIVITY;
		float gz = gyrData[2] / GYROSCOPE_SENSITIVITY;
		dtostrf(ax, 4, 2, sabuf);
		dtostrf(ay, 4, 2, sbbuf);
		dtostrf(az, 4, 2, scbuf);
		dtostrf(gx, 4, 1, gxbuf);
		dtostrf(gy, 4, 1, gybuf);
		dtostrf(gz, 4, 1, gzbuf);
		dtostrf(roll, 4, 1, rbuf);
		dtostrf(pitch,4, 1, pbuf);
		dtostrf(yaw,  4, 1, ybuf);

		// UART output
		snprintf(uart_vals1, sizeof(uart_vals1),
		"AX = %sg  AY = %sg  AZ = %sg\n"
		"GX = %s  GY = %s  GZ = %s\n",
		sabuf, sbbuf, scbuf,
		gxbuf, gybuf, gzbuf
		);
		snprintf(uart_vals2, sizeof(uart_vals2),
		"R = [%s] P = [%s] Y = [%s]\n",
		rbuf, pbuf, ybuf
		);
		uart_print_str(uart_vals1);
		uart_print_str(uart_vals2);
		uart_print_str("\n\n");

		// OLED update
		lcd_clrscr();
		lcd_puts_fb(0, 0, "R:");   lcd_puts_fb(2, 0, rbuf);   lcd_puts_fb(6, 0, " deg");
		lcd_puts_fb(0, 1, "P:");   lcd_puts_fb(2, 1, pbuf);   lcd_puts_fb(6, 1, " deg");
		lcd_puts_fb(0, 2, "Y:");   lcd_puts_fb(2, 2, ybuf);   lcd_puts_fb(6, 2, " deg");
		lcd_display();

		_delay_ms(100);
	}
	return 0;
}