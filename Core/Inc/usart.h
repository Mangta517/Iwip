/**
  ******************************************************************************
  * @file    usart.h
  * @brief   USART1 debug serial initialization and formatted output.
  ******************************************************************************
  */

#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

extern UART_HandleTypeDef huart1;

/**
  * @brief  Initialize USART1 (PA9 TX, PA10 RX, 115200 8N1) for status logging.
  */
void MX_USART1_UART_Init(void);

/**
  * @brief  Create the FreeRTOS mutex protecting USART1 output.
  *         Must be called after the FreeRTOS scheduler starts (or after all
  *         boot-time HAL_Delay calls) so BASEPRI does not mask SysTick.
  */
void USART1_InitMutex(void);

/**
  * @brief  Format and transmit a log string over USART1.
  * @param  fmt: printf-style format string.
  */
void usart_printf(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */
