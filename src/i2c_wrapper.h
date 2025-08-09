#ifndef I2C_WRAPPER_H__
#define I2C_WRAPPER_H__

#include <stddef.h>
#include <stdint.h>
#include "stm32f4xx_hal.h"

void i2c_write(I2C_HandleTypeDef *I2Cx, uint8_t dev_addr, uint8_t *data, size_t data_size);

void i2c_read(I2C_HandleTypeDef *I2Cx, uint8_t dev_addr, uint8_t *data, size_t data_size);

void i2c_scan(I2C_HandleTypeDef *I2Cx, uint8_t *has_addr);

#endif  //  I2C_WRAPPER_H__
