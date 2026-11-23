#include "radio_tasks.h"
#include "main.h"
#include "i2c.h"
#include "gpio.h"
#include "lcd.h"

#define TASK_STACK_SIZE (configMINIMAL_STACK_SIZE)

TaskHandle_t fsm_task_handle = NULL;
TaskHandle_t keypad_task_handle = NULL;

void Error_Handler(void) {
    while(1) {
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if(htim->Instance == TIM1) {
        HAL_IncTick();
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

int main(void) {
    GPIO_TypeDef *ports_list[] = {
        GPIOA, GPIOA, GPIOA, GPIOA,
        GPIOA, GPIOA, GPIOA, GPIOA
    };
    uint16_t pins_list[] = {
        GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3,
        GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7
    };

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_I2C3_Init();

    tasks_init(&hi2c1, &hi2c3, ports_list, pins_list);

    xTaskCreate(
        keypad_task,
        "keypad_task",
        TASK_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        &keypad_task_handle
    );

    xTaskCreate(
        fsm_task,
        "fsm_task",
        TASK_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 2,
        &fsm_task_handle
    );

    vTaskStartScheduler();

    while(1) {
    }
    return 0;
}

