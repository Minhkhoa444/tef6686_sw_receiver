#ifndef LCD_H__
#define LCD_H__

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "i2c_wrapper.h"

void lcd_init(I2C_HandleTypeDef *I2Cx, uint8_t dev_addr);

void lcd_clear(void);

void lcd_goto(uint8_t x, uint8_t y);

void lcd_print(char *c);

void lcd_create_char(uint8_t index, uint8_t *char_map);

void lcd_print_char(uint8_t index);

#endif
