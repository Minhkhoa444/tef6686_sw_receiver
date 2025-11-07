#ifndef TEF6686_H__
#define TEF6686_H__

#include <stdint.h>
#include <math.h>
#include "stm32f4xx.h"
#include "tef6686_patch_bytes.h"
#include "i2c_wrapper.h"

typedef enum rx_mode_t {
    FM_MODE,
    AM_MODE
} rx_mode_t;

typedef enum fm_deemphasis_t {
    FM_DEEMPHASIS_75,
    FM_DEEMPHASIS_50
} fm_deemphasis_t;

void tef6686_init(I2C_HandleTypeDef *I2Cx, uint8_t addr);

void tef6686_set_mode(rx_mode_t rx_mode);

void tef6686_set_fm_deemphasis(fm_deemphasis_t deemphasis);

void tef6686_set_mono(void);

void tef6686_set_stereo(void);

void tef6686_set_softmute(uint8_t enable, uint16_t attenuation);

void tef6686_set_mute(uint8_t enable);

void tef6686_set_volume(int16_t gain);

void tef6686_set_freq(uint16_t rx_freq);

void tef6686_get_signal_quality(int16_t *rssi, uint16_t *usn, uint16_t *wam, int16_t *offset);

uint8_t tef6686_get_fm_stereo(void);

uint8_t tef6686_get_rds_status(void);

void tef6686_get_rds(uint16_t *block_a, uint16_t *block_b, uint16_t *block_c, uint16_t *block_d, uint8_t *block_error);

#endif  //  TEF6686_H__
