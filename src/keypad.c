#include "keypad.h"

static GPIO_TypeDef *col_ports[4];
static uint16_t col_pins[4];
static GPIO_TypeDef *row_ports[4];
static uint16_t row_pins[4];

static const char keypad_button_value[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static void set_col(uint8_t col) {
    for(uint8_t i = 0; i < 4; i += 1) {
        if(i == col) {
            HAL_GPIO_WritePin(col_ports[col], col_pins[col], GPIO_PIN_RESET);   //  Pull the pin in scan to LOW
        }
        else {
            HAL_GPIO_WritePin(col_ports[i], col_pins[i], GPIO_PIN_SET);
        }
    }
}

static char check_row(uint8_t col) {
    for(uint8_t row = 0; row < 4; row += 1) {
        if(!HAL_GPIO_ReadPin(row_ports[row], row_pins[row])) {
            if(!HAL_GPIO_ReadPin(row_ports[row], row_pins[row]));
            return keypad_button_value[row][col];
        }
    }
    return '\0';
}

/*
    4 column (output) then 4 row (input) ports
*/

void keypad_init(GPIO_TypeDef **ports, uint16_t *pins) {
    for(uint8_t i = 0; i < 4; i += 1) {
        col_ports[i] = ports[i];
        col_pins[i] = pins[i];
    }
    for(uint8_t i = 0; i < 4; i += 1) {
        row_ports[i] = ports[i + 4];
        row_pins[i] = pins[i + 4];
    }
}

char keypad_get(void) {
    char key = '\0';
    for(uint8_t col = 0; col < 4; col += 1) {
        set_col(col);
        key = check_row(col);
        if(key != '\0') {
            break;
        }
    }
    for(uint8_t i =0; i < 4; i += 1) {
        HAL_GPIO_WritePin(col_ports[i], col_pins[i], GPIO_PIN_SET);
    }
    return key;
}
