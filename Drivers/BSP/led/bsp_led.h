/**
  ******************************************************************************
  * @file    bsp_led.h
  * @brief   板级 LED 驱动 (野火 STM32F407 霸天虎 V2)
  *          依据原理图《野火_F407霸天虎_原理图_V2.0.pdf》Sheet 10 (SENSOR/KEY/LED):
  *            - D6 (RGB_LED, 共阳经跳线帽 J73 接 3V3, 低电平点亮):
  *                LED_R -> PF6 (R35 470Ω)
  *                LED_G -> PF7 (R36 330Ω)
  *                LED_B -> PF8 (R37 100Ω)
  *            - D48 (BLUE, 独立蓝灯直连 3V3, 低电平点亮):
  *                LED_BLUE -> PC3 (R18 1KΩ)
  ******************************************************************************
  */
#ifndef __BSP_LED_H
#define __BSP_LED_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_LED_R_GPIO_PORT     GPIOF
#define BSP_LED_R_GPIO_PIN      GPIO_PIN_6

#define BSP_LED_G_GPIO_PORT     GPIOF
#define BSP_LED_G_GPIO_PIN      GPIO_PIN_7

#define BSP_LED_B_GPIO_PORT     GPIOF
#define BSP_LED_B_GPIO_PIN      GPIO_PIN_8

#define BSP_LED_BLUE_GPIO_PORT  GPIOC
#define BSP_LED_BLUE_GPIO_PIN   GPIO_PIN_3

typedef enum
{
    BSP_LED_R = 0,
    BSP_LED_G,
    BSP_LED_B,
    BSP_LED_BLUE,
    BSP_LED_COUNT
} bsp_led_id_t;

void BSP_LED_Init(void);
void BSP_LED_Set(bsp_led_id_t led, uint8_t on);
void BSP_LED_Toggle(bsp_led_id_t led);
void BSP_LED_SetRgb(uint8_t r_on, uint8_t g_on, uint8_t b_on);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_LED_H */
