/**
  ******************************************************************************
  * @file    usart.c
  * @brief   USART1 debug serial initialization and formatted output.
  ******************************************************************************
  */

#include "usart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>

UART_HandleTypeDef huart1;
static uint8_t s_usart1_ready = 0;
static SemaphoreHandle_t s_usart1_mutex = NULL;

void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    /* 不要在调度器启动前调用 FreeRTOS API（如 xSemaphoreCreateMutex），
     * 否则 vPortEnterCritical 会抬高 BASEPRI 且因 uxCriticalNesting!=0 不恢复，
     * 导致后续 NT35510_Init / GTP_Init_Panel 中的 HAL_Delay 死等。 */
    s_usart1_ready = 1;
}

void USART1_InitMutex(void)
{
    if (s_usart1_mutex == NULL)
    {
        s_usart1_mutex = xSemaphoreCreateMutex();
    }
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (uartHandle->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* PA9 -> USART1_TX, PA10 -> USART1_RX (野火霸天虎 V2 板载 CH340 调试串口) */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
    if (uartHandle->Instance == USART1)
    {
        s_usart1_ready = 0;
        __HAL_RCC_USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
    }
}

void usart_printf(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    int len;

    if (!s_usart1_ready || (fmt == NULL))
    {
        return;
    }

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len <= 0)
    {
        return;
    }
    if ((size_t)len >= sizeof(buf))
    {
        len = (int)(sizeof(buf) - 1);
    }

    if ((s_usart1_mutex != NULL) &&
        (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) &&
        (xPortIsInsideInterrupt() == pdFALSE))
    {
        if (xSemaphoreTake(s_usart1_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)len, 100);
            (void)xSemaphoreGive(s_usart1_mutex);
        }
        else
        {
            HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)len, 100);
        }
    }
    else
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)len, 100);
    }
}
