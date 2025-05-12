/* lcd.h */
#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>

// Initialize SSD1306: pass display on command (e.g., 0xAF)
void lcd_init(uint8_t disp_on_cmd);

// Clear internal framebuffer
void lcd_clrscr(void);

// Set text cursor: x = column (chars), y = page (0-7)
void lcd_gotoxy(uint8_t x, uint8_t y);

// Print a null-terminated string at current cursor
void lcd_puts(const char *s);

// Send framebuffer or text buffer to display
void lcd_display(void);

#endif // _LCD_H_