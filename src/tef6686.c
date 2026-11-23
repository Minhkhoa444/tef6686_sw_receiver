#include "tef6686.h"

typedef enum cmd_t {
    Tune_To = 1,
    Set_Tune_Option = 2,
    Set_Bandwidth = 10,
    Set_RFAGC = 11,
    Set_Antenna = 12,
    Set_CoChannelDet = 14,
    Set_MphSuppression = 20,
    Set_ChannelEqualizer = 22,
    Set_NoiseBlanker = 23,
    Set_NoiseBlanker_Options = 24,
    Set_NoiseBlanker_Audio = 24,
    Set_DigitalRadio = 30,
    Set_Deemphasis = 31,
    Set_StereoImprovement = 32,
    Set_LevelStep = 38,
    Set_LevelOffset = 39,
    Set_SoftMute_Time = 40,
    Set_SoftMute_Mod = 41,
    Set_SoftMute_Level = 42,
    Set_SoftMute_Noise = 43,
    Set_SoftMute_Mph = 44,
    Set_SoftMute_Max = 45,
    Set_HighCut_Time = 50,
    Set_HighCut_Mod = 51,
    Set_HighCut_Level = 52,
    Set_HighCut_Noise = 53,
    Set_HighCut_Mph = 54,
    Set_HighCut_Max = 55,
    Set_HighCut_Min = 56,
    Set_LowCut_Max = 57,
    Set_LowCut_Min = 58,
    Set_HighCut_Options = 59,
    Set_Stereo_Time = 60,
    Set_Stereo_Mod = 61,
    Set_Stereo_Level = 62,
    Set_Stereo_Noise = 63,
    Set_Stereo_Mph = 64,
    Set_Stereo_Max = 65,
    Set_Stereo_Min = 66,
    Set_StHiBlend_Time = 70,
    Set_StHiBlend_Mod = 71,
    Set_StHiBlend_Level = 72,
    Set_StHiBlend_Noise = 73,
    Set_StHiBlend_Mph = 74,
    Set_StHiBlend_Max = 75,
    Set_StHiBlend_Min = 76,
    Set_Scaler = 80,
    Set_RDS = 81,
    Set_QualityStatus = 82,
    Set_DR_Blend = 83,
    Set_DR_Options = 84,
    Set_Specials = 85,
    Set_Bandwidth_Options = 86,
    Set_StBandBlend_Time = 90,
    Set_StBandBlend_Gain = 91,
    Set_StBandBlend_Bias = 92,
    Set_Volume = 10,
    Set_Mute = 11,
    Set_Input = 12,
    Set_Output_Source = 13,
    Set_OperationMode = 1,
    Set_GPIO = 3,
    Set_ReferenceClock = 4,
    Activate = 5,
    Set_Ana_Out = 21,
    Set_Dig_IO = 22,
    Set_Input_Scaler = 23,
    Set_WaveGen = 24,
    Get_Quality_Status = 128,
    Get_Quality_Data = 129,
    Get_RDS_Status = 130,
    Get_RDS_Data = 131,
    Get_AGC = 132,
    Get_Signal_Status = 133,
    Get_Processing_Status = 134,
    Get_Interface_Status = 135,
    Get_Operation_Status = 128,
    Get_GPIO_Status = 129,
    Get_Identification = 130,
    Get_LastWrite = 131    
} cmd_t;

typedef enum module_t {
    FM_RADIO = 32,
    AM_RADIO = 33,
    AUDIO = 48,
    APPL = 64
} module_t;

static uint8_t dev_addr = 0x00;
static I2C_HandleTypeDef *i2c_handle;
static rx_mode_t current_rx_mode;

static void tef6686_power_up(void) {
    HAL_Delay(100);
    uint8_t cmd[13];
    //  Send required patch
    cmd[0] = 0x1C;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    cmd[0] = 0x1C;
    cmd[1] = 0x00;
    cmd[2] = 0x74;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    size_t patch_size = sizeof(patch_bytes);
    const uint8_t *patch_ptr = patch_bytes;
    while(patch_size) {
        cmd[0] = 0x1B;
        size_t len = (patch_size > 12) ? 12 : patch_size;
        patch_size -= len;
        for(size_t i = 0; i < len; i += 1) {
            cmd[i + 1] = patch_ptr[i];
        }
        patch_ptr += len;
        i2c_write(i2c_handle, dev_addr, cmd, len + 1);
    }
    cmd[0] = 0x1C;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    cmd[0] = 0x1C;
    cmd[1] = 0x00;
    cmd[2] = 0x75;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    patch_size = sizeof(lut_bytes);
    const uint8_t *lut_ptr = lut_bytes;
    while(patch_size) {
        cmd[0] = 0x1B;
        size_t len = (patch_size > 12) ? 12 : patch_size;
        patch_size -= len;
        for(size_t i = 0; i < len; i += 1) {
            cmd[i + 1] = lut_ptr[i];
        }
        lut_ptr += len;
        i2c_write(i2c_handle, dev_addr, cmd, len + 1);
    }
    cmd[0] = 0x1C;
    cmd[1] = 0x00;
    cmd[2] = 0x00;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    //  Patch sent
    cmd[0] = 0x14;
    cmd[1] = 0x00;
    cmd[2] = 0x01;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    HAL_Delay(100);
    //cmd[0] = APPL;
    //cmd[1] = Set_ReferenceClock;
    //cmd[2] = 0x01;
    //cmd[3] = 0x00;
    //cmd[4] = 0x8C;
    //cmd[5] = 0xA0;
    //cmd[6] = 0x00;
    //cmd[7] = 0x00;
    //cmd[8] = 0x00;  //  no need to config osc when using 9.216 MHz
    //i2c_write(i2c_handle, dev_addr, cmd, 9);
    cmd[0] = APPL;
    cmd[1] = Activate;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x01;
    i2c_write(i2c_handle, dev_addr, cmd, 5);
    HAL_Delay(100);
    cmd[0] = APPL;
    cmd[1] = Set_OperationMode;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    i2c_write(i2c_handle, dev_addr, cmd, 5);
}

void tef6686_init(I2C_HandleTypeDef *I2Cx, uint8_t addr) {
    dev_addr = addr;
    i2c_handle = I2Cx;
    uint8_t cmd[11];
    uint8_t rec[2];
    cmd[0] = APPL;
    cmd[1] = Get_Operation_Status;
    cmd[2] = 0x01;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    i2c_read(i2c_handle, dev_addr, rec, 2);
    if(rec[1] != 0x00 && rec[1] != 0x01) {
        return; //  Radio already powered up
    }
    tef6686_power_up();
    cmd[0] = FM_RADIO;
    cmd[1] = Set_ChannelEqualizer;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x01;
    i2c_write(i2c_handle, dev_addr, cmd, 5);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_SoftMute_Time;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x78;
    cmd[5] = 0x01;
    cmd[6] = 0xF4;
    cmd[7] = 0x00;
    cmd[8] = 0x0A;
    cmd[9] = 0x00;
    cmd[10] = 0x14;
    i2c_write(i2c_handle, dev_addr, cmd, 11);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_HighCut_Time;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0xC8;
    cmd[5] = 0x01;
    cmd[6] = 0xF4;
    cmd[7] = 0x00;
    cmd[8] = 0x0A;
    cmd[9] = 0x00;
    cmd[10] = 0x50;
    i2c_write(i2c_handle, dev_addr, cmd, 11);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_HighCut_Noise;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x03;
    cmd[5] = 0x00;
    cmd[6] = 0x96;
    cmd[7] = 0x00;
    cmd[8] = 0xC8;
    i2c_write(i2c_handle, dev_addr, cmd, 9);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_HighCut_Mph;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x03;
    cmd[5] = 0x00;
    cmd[6] = 0x78;
    cmd[7] = 0x00;
    cmd[8] = 0xA0;
    i2c_write(i2c_handle, dev_addr, cmd, 9);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_HighCut_Max;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x01;
    cmd[5] = 0x09;
    cmd[6] = 0x60;
    i2c_write(i2c_handle, dev_addr, cmd, 7);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_LowCut_Max;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x01;
    cmd[5] = 0x00;
    cmd[6] = 0x64;
    i2c_write(i2c_handle, dev_addr, cmd, 7);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_Stereo_Time;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0xC8;
    cmd[5] = 0x0F;
    cmd[6] = 0xA0;
    cmd[7] = 0x00;
    cmd[8] = 0x14;
    cmd[9] = 0x00;
    cmd[10] = 0x50;
    i2c_write(i2c_handle, dev_addr, cmd, 11);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_Stereo_Noise;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x03;
    cmd[5] = 0x00;
    cmd[6] = 0x78;
    cmd[7] = 0x00;
    cmd[8] = 0xA0;
    i2c_write(i2c_handle, dev_addr, cmd, 9);
    cmd[0] = FM_RADIO;
    cmd[1] = Set_Stereo_Mph;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x03;
    cmd[5] = 0x00;
    cmd[6] = 0x64;
    cmd[7] = 0x00;
    cmd[8] = 0x96;
    i2c_write(i2c_handle, dev_addr, cmd, 9);
    //cmd[1] = Set_StHiBlend_Noise;
    //cmd[1] = Set_StHiBlend_Mph;
    cmd[0] = AM_RADIO;
    cmd[1] = Set_HighCut_Level;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x02;
    cmd[5] = 0x01;
    cmd[6] = 0xD6;
    cmd[7] = 0x00;
    cmd[8] = 0xC8;
    i2c_write(i2c_handle, dev_addr, cmd, 9);
}

void tef6686_set_mode(rx_mode_t rx_mode) {
    current_rx_mode = rx_mode;
}

void tef6686_set_fm_deemphasis(fm_deemphasis_t deemphasis) {
    uint8_t cmd[5];
    cmd[0] = FM_RADIO;
    cmd[1] = Set_Deemphasis;
    cmd[2] = 0x01;
    if(deemphasis == FM_DEEMPHASIS_75) {
        cmd[3] = 0x02;
        cmd[4] = 0xEE;
    }
    else if(deemphasis == FM_DEEMPHASIS_50) {
        cmd[3] = 0x01;
        cmd[4] = 0xF4;
    }
    i2c_write(i2c_handle, dev_addr, cmd, 5);
}

void tef6686_set_mono(void) {
    uint8_t cmd[7];
    cmd[0] = FM_RADIO;
    cmd[1] = Set_Stereo_Min;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x02;
    cmd[5] = 0x01;
    cmd[6] = 0x90;
    i2c_write(i2c_handle, dev_addr, cmd, 7);
}

void tef6686_set_stereo(void) {
    uint8_t cmd[7];
    cmd[0] = FM_RADIO;
    cmd[1] = Set_Stereo_Min;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x01;
    cmd[6] = 0x90;
    i2c_write(i2c_handle, dev_addr, cmd, 7);
}

void tef6686_set_softmute(uint8_t enable, uint16_t attenuation) {
    uint8_t cmd[7];
    if(current_rx_mode == FM_MODE) {
        cmd[0] = FM_RADIO;
    }
    else if(current_rx_mode == AM_MODE) {
        cmd[0] = AM_RADIO;
    }
    cmd[1] = Set_SoftMute_Max;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = enable;
    cmd[5] = attenuation >> 8;
    cmd[6] = attenuation;
    i2c_write(i2c_handle, dev_addr, cmd, 7);
}

void tef6686_set_mute(uint8_t enable) {
    uint8_t cmd[5];
    cmd[0] = AUDIO;
    cmd[1] = Set_Mute;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = enable;
    i2c_write(i2c_handle, dev_addr, cmd, 5);
}

void tef6686_set_volume(int16_t gain) {
    gain *= 10;
    uint8_t cmd[5];
    cmd[0] = AUDIO;
    cmd[1] = Set_Volume;
    cmd[2] = 0x01;
    cmd[3] = gain >> 8;
    cmd[4] = gain;
    i2c_write(i2c_handle, dev_addr, cmd, 5);
}

void tef6686_set_freq(uint16_t rx_freq) {
    uint8_t cmd[7];
    if(current_rx_mode == FM_MODE) {
        cmd[0] = FM_RADIO;
    }
    else if(current_rx_mode == AM_MODE) {
        cmd[0] = AM_RADIO;
    }
    cmd[1] = Tune_To;
    cmd[2] = 0x01;
    cmd[3] = 0x00;
    cmd[4] = 0x01;
    cmd[5] = rx_freq >> 8;
    cmd[6] = rx_freq;
    i2c_write(i2c_handle, dev_addr, cmd, 7);
}

void tef6686_get_signal_quality(int16_t *rssi, uint16_t *usn, uint16_t *wam, int16_t *offset) {
    uint8_t cmd[3];
    uint8_t rec[14];
    int16_t level;
    uint16_t usn_noise;
    uint16_t wam_raw;
    int16_t offset_raw;
    if(current_rx_mode == FM_MODE) {
        cmd[0] = FM_RADIO;
    }
    else if(current_rx_mode == AM_MODE) {
        cmd[0] = AM_RADIO;
    }
    cmd[1] = Get_Quality_Data;
    cmd[2] = 0x01;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    i2c_read(i2c_handle, dev_addr, rec, 14);
    level = (uint16_t)rec[2] << 8 | rec[3];
    usn_noise = (uint16_t)rec[4] << 8 | rec[5];
    wam_raw = (uint16_t)rec[6] << 8 | rec[7];
    offset_raw = (uint16_t)rec[8] << 8 | rec[9];
    if(offset_raw < 0) {
        offset_raw = -offset_raw;
    }
    *rssi = level / 10;
    *usn = usn_noise / 10;
    *wam = wam_raw / 10;
    *offset = offset_raw / 10;
}

uint8_t tef6686_get_fm_stereo(void) {
    uint8_t cmd[3];
    uint8_t rec[2];
    cmd[0] = FM_RADIO;
    cmd[1] = Get_Signal_Status;
    cmd[2] = 0x01;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    i2c_read(i2c_handle, dev_addr, rec, 2);
    return rec[0] >> 7;
}

uint8_t tef6686_get_rds_status(void) {
    uint8_t cmd[3];
    uint8_t rec[2];
    cmd[0] = FM_RADIO;
    cmd[1] = Get_RDS_Status;
    cmd[2] = 0x01;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    i2c_read(i2c_handle, dev_addr, rec, 2);
    return rec[0] >> 7;
}

void tef6686_get_rds(uint16_t *block_a, uint16_t *block_b, uint16_t *block_c, uint16_t *block_d, uint8_t *block_error) {
    uint8_t cmd[3];
    uint8_t rec[12];
    cmd[0] = FM_RADIO;
    cmd[1] = Get_RDS_Data;
    cmd[2] = 0x01;
    i2c_write(i2c_handle, dev_addr, cmd, 3);
    i2c_read(i2c_handle, dev_addr, rec, 12);
    *block_a = (uint16_t)rec[2] << 8 | rec[3];
    *block_b = (uint16_t)rec[4] << 8 | rec[5];
    *block_c = (uint16_t)rec[6] << 8 | rec[7];
    *block_d = (uint16_t)rec[8] << 8 | rec[9];
    *block_error = rec[10];
}
