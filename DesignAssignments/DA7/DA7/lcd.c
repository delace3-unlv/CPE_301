/* lcd.c */
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "i2c.h"
#include "lcd.h"
#include "font.h"

#define SSD1306_ADDR 0x3C
#define SSD1306_128_64
static uint8_t framebuf[128*8];

static void ssd1306_command(uint8_t cmd) {
	i2c_start((SSD1306_ADDR<<1));
	i2c_write(0x00);
	i2c_write(cmd);
	i2c_stop();
}

void lcd_init(uint8_t disp_on_cmd) {
	// hardware reset on PD2
	DDRD |= (1<<PD2);
	PORTD &= ~(1<<PD2); _delay_ms(10);
	PORTD |=  (1<<PD2); _delay_ms(10);

	// initialization sequence
	ssd1306_command(0xAE);             // display off
	ssd1306_command(0xD5); ssd1306_command(0x80);
	ssd1306_command(0xA8); ssd1306_command(0x3F);
	ssd1306_command(0xD3); ssd1306_command(0x00);
	ssd1306_command(0x20); ssd1306_command(0x02); 
	ssd1306_command(0x8D); ssd1306_command(0x14);
	ssd1306_command(0xA1);
	ssd1306_command(0xC8);
	ssd1306_command(0xDA); ssd1306_command(0x12);
	ssd1306_command(0x81); ssd1306_command(0xFF);
	ssd1306_command(0xD9); ssd1306_command(0xF1);
	ssd1306_command(0xDB); ssd1306_command(0x40);
	ssd1306_command(0xA4);
	ssd1306_command(0xA6);
	ssd1306_command(disp_on_cmd);      // display on/off
}

void lcd_clrscr(void) {
	memset(framebuf, 0, sizeof(framebuf));
}

void lcd_gotoxy(uint8_t x, uint8_t y) {
	uint8_t col = x * 6;
	ssd1306_command(0xB0 + y);
	ssd1306_command(0x00 + (col & 0x0F));
	ssd1306_command(0x10 + (col >> 4));
}

void lcd_puts(const char *s) {
	while (*s) {
		char c = *s++;
		if (c < 32 || c > 127) c = '?';
		uint8_t ci = c < 32 || c > 127 ? 0 : (c - 32);
		i2c_start((SSD1306_ADDR<<1));
		i2c_write(0x40);              // data stream
		for (uint8_t i = 0; i < 6; i++) {
			uint8_t b = pgm_read_byte(&ssd1306oled_font[ci][i]);
			i2c_write(b);
		}
		i2c_stop();
	}
}

void lcd_putchar_fb(uint8_t x, uint8_t y, char c) {
	if (c < 32 || c > 127) c = '?';
	uint8_t ci = c - 32;
	uint16_t base = y*128 + x*6;
	for (uint8_t i = 0; i < 6; i++) {
		framebuf[base + i] = pgm_read_byte(&ssd1306oled_font[ci][i]);
	}
}

void lcd_puts_fb(uint8_t x, uint8_t y, const char* s) {
	while (*s) {
		lcd_putchar_fb(x++, y, *s++);
	}
}

void lcd_display(void) {
	for (uint8_t page = 0; page < 8; page++) {
		ssd1306_command(0xB0 + page);
		ssd1306_command(0x00);
		ssd1306_command(0x10);
		i2c_start((SSD1306_ADDR<<1));
		i2c_write(0x40);
		for (uint8_t col = 0; col < 128; col++) {
			i2c_write(framebuf[page*128 + col]);
		}
		i2c_stop();
	}
}