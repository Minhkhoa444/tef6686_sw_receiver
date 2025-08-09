#ifndef RADIO_TASKS_H__
#define RADIO_TASKS_H__

#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "keypad.h"
#include "lcd.h"
#include "tef6686.h"

void tasks_init(I2C_HandleTypeDef *radio_I2Cx, I2C_HandleTypeDef *lcd_I2Cx, GPIO_TypeDef **ports, uint16_t *pins);

void fsm_task(void *pvParameters);

#endif  //  RADIO_TASKS_H__
