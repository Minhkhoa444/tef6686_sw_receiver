#include "i2c_wrapper.h"

void i2c_write(I2C_HandleTypeDef *I2Cx, uint8_t dev_addr, uint8_t *data, size_t data_size) {
    HAL_I2C_Master_Transmit(I2Cx, dev_addr,(uint8_t *)data, data_size, 1000);    
}

void i2c_read(I2C_HandleTypeDef *I2Cx, uint8_t dev_addr, uint8_t *data, size_t data_size) {
    HAL_I2C_Master_Receive(I2Cx, dev_addr, (uint8_t *)data, data_size, 1000);    
}

void i2c_scan(I2C_HandleTypeDef *I2Cx, uint8_t *has_addr) {
    for(uint8_t addr = 1; addr < 128; addr = addr + 1) {
        if(HAL_I2C_IsDeviceReady(I2Cx, addr, 1, 1000) == HAL_OK) {
            has_addr[addr] = 1;
        }
    }
}
