#include "radio_tasks.h"

static const uint8_t radio_addr = 0x64 << 1;
static const uint8_t lcd_addr = 0x27 << 1;
static rx_mode_t rx_mode = FM_MODE;
static uint16_t input_freq = 0;
static uint16_t freq = 9100;
static uint8_t is_mono = 0;
static uint16_t scan_freq_list[211];
static uint8_t scan_mem_idx = 0;
static uint8_t scan_stations_found = 0;
static char scan_interrupted_key = '\0';
static int16_t rssi_threshold = 25;     //  dBuV
static uint16_t usn_threshold = 27;     //  %
static uint16_t wam_threshold = 23;     //  %
static int16_t offset_threshold = 10;   //  kHz

typedef enum state_t {
    DISPLAY_MODE,
    INPUT_MODE,
    SCAN_MODE
} state_t;

static state_t current_state = DISPLAY_MODE;

QueueHandle_t keypad_queue;

static void fm_radio_display_update(void) {
    char buffer[20];
    lcd_clear();
    lcd_goto(0, 0);
    sprintf(buffer, "%u.%u MHz", freq / 100, freq % 100);
    lcd_print(buffer);
    lcd_goto(0, 1);
    lcd_print("FM MODE");
    lcd_goto(9, 1);
    if(is_mono) {
        lcd_print("MONO");
    }
    else {
        lcd_print("STEREO");
    }
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

static void scan_mode_display_update() {
    char buffer[20];
    lcd_clear();
    lcd_goto(0, 0);
    sprintf(buffer, "IDX %u/%u", scan_mem_idx + 1, scan_stations_found);
    lcd_print(buffer);
    uint16_t mem_freq = scan_freq_list[scan_mem_idx];
    lcd_goto(0, 1);
    if(mem_freq == 0) {
        lcd_print("EMPTY");
    }
    else {
        sprintf(buffer, "%u.%u MHz", mem_freq / 100, mem_freq % 100);
        lcd_print(buffer);
    }
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

static void fm_scan(void) {
    scan_mem_idx = 0;
    scan_stations_found = 0;
    for(uint8_t i = 0; i < 211; i = i + 1) {
        scan_freq_list[i] = 0;
    }
    for(uint16_t scan_freq = 8700; scan_freq <= 10800; scan_freq = scan_freq + 10) {
        tef6686_set_freq(scan_freq);
        vTaskDelay(pdMS_TO_TICKS(50));
        int16_t rssi;
        uint16_t usn;
        uint16_t wam;
        int16_t offset;
        tef6686_get_signal_quality(&rssi, &usn, &wam, &offset);
        if(rssi >= rssi_threshold && usn < usn_threshold && wam < wam_threshold && offset < offset_threshold) {
            scan_freq_list[scan_mem_idx] = scan_freq;
            scan_stations_found += 1;
            scan_mem_idx += 1;
        }
        if(xQueueReceive(keypad_queue, &scan_interrupted_key, 0) == pdPASS) {
            return;
        }
    }
}

static void display_mode(char key) {
    if(key >= '0' && key <= '9') {
        lcd_clear();
        lcd_goto(0, 0);
        lcd_print_char(key);
        input_freq = (uint16_t )key - '0';
        current_state = INPUT_MODE;
    }
    else if(key == '*') {
        if(rx_mode == FM_MODE) {
            scan_mem_idx = 0;
            scan_mode_display_update();
            current_state = SCAN_MODE;
        }
    }
    else if(key == 'A') {
        if(rx_mode == FM_MODE) {
            tef6686_set_mode(AM_MODE);
            tef6686_set_freq(610);
            tef6686_set_softmute(0, 0);
            freq = 610;
            rx_mode = AM_MODE;
            am_radio_display_update();
        }
        else if(rx_mode == AM_MODE) {
            tef6686_set_mode(FM_MODE);
            tef6686_set_freq(9100);
            freq = 9100;
            rx_mode = FM_MODE;
            fm_radio_display_update();
        }
    }
    else if(key == 'B') {
        if(rx_mode == FM_MODE) {
            if(is_mono) {
                tef6686_set_stereo();
                is_mono = 0;
            }
            else {
                tef6686_set_mono();
                is_mono = 1;
            }
            fm_radio_display_update();
        }
    }
}

static void input_mode(char key) {
    if(key >= '0' && key <= '9') {
        lcd_print_char(key);
        uint16_t temp = (uint16_t )key - '0';
        input_freq = input_freq * 10 + temp;
    }
    else if(key == 'A') {
        if(rx_mode == FM_MODE) {
            freq = input_freq * 10;
        }
        else if(rx_mode == AM_MODE) {
            freq = input_freq;
        }
        if(rx_mode == FM_MODE && !is_in_fm_range()) {
            freq = 9100;
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
}

static void scan_mode(char key) {
    if(key == 'C') {        //      Next memory location
        if(scan_stations_found > 0) {
            if(scan_mem_idx + 1< scan_stations_found) {
                scan_mem_idx += 1;
            }
            else {
                scan_mem_idx = 0;
            }
            tef6686_set_freq(scan_freq_list[scan_mem_idx]);
        }
        scan_mode_display_update();
    }
    else if(key == 'D') {   //      Previous memory location
        if(scan_stations_found > 0) {
            if(scan_mem_idx > 0) {
                scan_mem_idx -= 1;
            }
            else {
                scan_mem_idx = scan_stations_found - 1;
            }
            tef6686_set_freq(scan_freq_list[scan_mem_idx]);
        }
        scan_mode_display_update();
    }
    else if(key == 'A') {   //      Start scan
        lcd_clear();
        lcd_goto(0, 0);
        lcd_print("Scanning...");
        fm_scan();
        lcd_clear();
        lcd_goto(0, 0);
        lcd_print("Scan finished");
        scan_mem_idx = 0;
        if(scan_freq_list[0] != 0) {
            freq = scan_freq_list[scan_mem_idx];
            tef6686_set_freq(freq);
        }
        scan_mode_display_update();
        if(scan_interrupted_key != '\0') {
            char next_key = scan_interrupted_key;
            scan_interrupted_key = '\0';
            scan_mode(next_key);
        }
    }
    else if(key == 'B') {   //      Set current freq in memory to DISPLAY_MODE
        if(scan_stations_found > 0) {
            freq = scan_freq_list[scan_mem_idx];
            tef6686_set_freq(freq);
        }
        fm_radio_display_update();
        current_state = DISPLAY_MODE;
    }
    else if(key == '*') {   //      Exit scan mode
        freq = 9100;
        tef6686_set_freq(9100);
        fm_radio_display_update();
        current_state = DISPLAY_MODE;
    }
}

void tasks_init(I2C_HandleTypeDef *radio_I2Cx, I2C_HandleTypeDef *lcd_I2Cx, GPIO_TypeDef **ports, uint16_t *pins) {
    keypad_queue = xQueueCreate(10, sizeof(char));
    keypad_init(ports, pins);
    tef6686_init(radio_I2Cx, radio_addr);
    tef6686_set_mode(FM_MODE);
    tef6686_set_fm_deemphasis(FM_DEEMPHASIS_50);
    tef6686_set_freq(freq);
    tef6686_set_mute(0);
    tef6686_set_stereo();
    for(uint8_t i = 0; i < 211; i = i + 1) {
        scan_freq_list[i] = 0;
    }
    HAL_Delay(43);
    rx_mode = FM_MODE;
    current_state = DISPLAY_MODE;
    lcd_init(lcd_I2Cx, lcd_addr);
    fm_radio_display_update();
}

void keypad_task(void *pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    char key = '\0';
    while(1) {
        key = keypad_get();
        if (key != '\0') {
            xQueueSend(keypad_queue, &key, 0);
        }
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10));        
    }
}

void fsm_task(void *pvParameters) {
    char key = '\0';
    while(1) {
        if(xQueueReceive(keypad_queue, &key, portMAX_DELAY) == pdPASS) {
            switch(current_state) {
                case DISPLAY_MODE:
                    display_mode(key);
                    break;
                case INPUT_MODE:
                    input_mode(key);
                    break;
                case SCAN_MODE:
                    scan_mode(key);
                    break;
                default:
                    break;
            }
        }
    }
}
