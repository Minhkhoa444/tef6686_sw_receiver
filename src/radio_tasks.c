#include "radio_tasks.h"

static const uint8_t radio_addr = 0x64 << 1;
static const uint8_t lcd_addr = 0x27 << 1;
static rx_mode_t rx_mode = FM_MODE;
static uint16_t input_freq = 0;
static uint16_t freq = 9400;

typedef enum state_t {
    DISPLAY_MODE,
    INPUT_MODE
} state_t;

static state_t current_state = DISPLAY_MODE;

static void fm_radio_display_update(void) {
    char buffer[20];
    lcd_clear();
    lcd_goto(0, 0);
    sprintf(buffer, "%u.%u MHz", freq / 100, freq % 100);
    lcd_print(buffer);
    lcd_goto(0, 1);
    lcd_print("FM MODE");
}

static void am_radio_display_update(void) {
    char buffer[20];
    lcd_clear();
    lcd_goto(0, 0);
    sprintf(buffer, "%u kHz", freq);
    lcd_print(buffer);
    lcd_goto(0, 1);
    lcd_print("AM MODE");
}

static uint8_t is_in_fm_range(void) {
    return (6500 <= freq && freq <= 10800);
}

static uint8_t is_in_lw_range(void) {
    return (144 <= freq && freq <= 288);
}

static uint8_t is_in_mw_range(void) {
    return (522 <= freq && freq <= 1710);
}

static uint8_t is_in_sw_range(void) {
    return (2300 <= freq && freq <= 27000);
}

static uint8_t is_in_am_range(void) {
    return is_in_lw_range() || is_in_mw_range() || is_in_sw_range();
}

void tasks_init(I2C_HandleTypeDef *radio_I2Cx, I2C_HandleTypeDef *lcd_I2Cx, GPIO_TypeDef **ports, uint16_t *pins) {
    keypad_init(ports, pins);
    lcd_init(lcd_I2Cx, lcd_addr);
    tef6686_init(radio_I2Cx, radio_addr);
    tef6686_set_mode(FM_MODE);
    tef6686_set_fm_deemphasis(FM_DEEMPHASIS_50);
    tef6686_set_freq(freq);
    tef6686_set_mute(0);
    HAL_Delay(43);
    rx_mode = FM_MODE;
    current_state = DISPLAY_MODE;
    fm_radio_display_update();
}

void fsm_task(void *pvParameters) {
    while(1) {
        char key = keypad_get();
        switch(current_state) {
            case DISPLAY_MODE:
                if(key >= '0' && key <= '9') {
                    lcd_clear();
                    lcd_goto(0, 0);
                    lcd_print_char(key);
                    input_freq = (uint16_t )key - '0';
                    current_state = INPUT_MODE;
                }
                else if(key == 'A') {
                    if(rx_mode == FM_MODE) {
                        tef6686_set_mode(AM_MODE);
                        tef6686_set_freq(610);
                        freq = 610;
                        rx_mode = AM_MODE;
                        am_radio_display_update();
                    }
                    else if(rx_mode == AM_MODE) {
                        tef6686_set_mode(FM_MODE);
                        tef6686_set_freq(9400);
                        freq = 9400;
                        rx_mode = FM_MODE;
                        fm_radio_display_update();
                    }
                }
                break;
            case INPUT_MODE:
                if(key >= '0' && key <= '9') {
                    lcd_print_char(key);
                    uint16_t temp = (uint16_t )key - '0';
                    input_freq = input_freq * 10 + temp;
                }
                else if(key == 'A') {
                    freq = input_freq;
                    if(rx_mode == FM_MODE && !is_in_fm_range()) {
                        freq = 9400;
                    }
                    else if(rx_mode == AM_MODE && !is_in_am_range()) {
                        freq = 610;
                    }
                    tef6686_set_freq(freq);
                    current_state = DISPLAY_MODE;
                    if(rx_mode == FM_MODE) {
                        fm_radio_display_update();
                    }
                    else if(rx_mode == AM_MODE) {
                        am_radio_display_update();
                    }
                }
                break;
            default:
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
