#include "lcd.h"

static uint8_t dev_addr = 0x00;
static I2C_HandleTypeDef i2c_handle;

static void lcd_send_cmd(uint8_t command) {
    uint8_t cmd[4];
    cmd[0] = (command & 0xF0) | 0x0C;
    cmd[1] = (command & 0xF0) | 0x08;
    cmd[2] = ((command << 4) & 0xF0) | 0x0C;
    cmd[3] = ((command << 4) & 0xF0) | 0x08;
    i2c_write(&i2c_handle, dev_addr, cmd, 4);
}

static void lcd_send_data(uint8_t data) {
    uint8_t cmd[4];
    cmd[0] = (data & 0xF0) | 0x0D;
    cmd[1] = (data & 0xF0) | 0x09;
    cmd[2] = ((data << 4) & 0xF0) | 0x0D;
    cmd[3] = ((data << 4) & 0xF0) | 0x09;
    i2c_write(&i2c_handle, dev_addr, cmd, 4);
}

void lcd_init(I2C_HandleTypeDef *I2Cx, uint8_t addr) {
    dev_addr = addr;
    i2c_handle = *I2Cx;
    HAL_Delay(50);
    lcd_send_cmd(0x30);
    HAL_Delay(5);
    lcd_send_cmd(0x30);
    HAL_Delay(1);
    lcd_send_cmd(0x30);
    HAL_Delay(10);
    lcd_send_cmd(0x20);
    HAL_Delay(10);

    lcd_send_cmd(0x28);
    HAL_Delay(1);
    lcd_send_cmd(0x08);
    HAL_Delay(1);
    lcd_send_cmd(0x01);
    HAL_Delay(2);
    lcd_send_cmd(0x06);
    HAL_Delay(1);
    lcd_send_cmd(0x0C);
}

void lcd_clear(void) {
    lcd_send_cmd(0x80);
    for(uint8_t i = 0; i < 80; i += 1) {
        lcd_send_data(' ');
    }
}

void lcd_goto(uint8_t x, uint8_t y) {
    if(y == 0) {
        x = x + 0x80;
    }
    else if(y == 1) {
        x = x + 0xC0;
    }
    else if(y == 2) {
        x = x + 0x94;
    }
    else if(y == 3) {
        x = x + 0xD4;
    }
    lcd_send_cmd(x);
}

void lcd_print(char *c) {
    while(*c) {
        lcd_send_data(*c++);
    }
}

void lcd_create_char(uint8_t index, uint8_t *char_map) {
    lcd_send_cmd(0x40 | (index << 3));
    for(uint8_t i = 0; i < 8; i = i + 1) {
        lcd_send_data(char_map[i]);
    }
}

void lcd_print_char(uint8_t index) {
    lcd_send_data(index);
}
