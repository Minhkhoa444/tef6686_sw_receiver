#ifndef KEYPAD_H__
#define KEYPAD_H__

#include <stdint.h>
#include "stm32f4xx_hal.h"

/*
    4 column then 4 row ports
*/

void keypad_init(GPIO_TypeDef **ports, uint16_t *pins);

char keypad_get(void);

#endif
