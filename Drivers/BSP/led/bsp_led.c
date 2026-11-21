/**
  ******************************************************************************
  * @file    bsp_led.c
  * @brief   板级 LED 驱动实现 (野火 STM32F407 霸天虎 V2)
  *          所有 LED 均为共阳/上拉接 3V3，低电平点亮、高电平熄灭。
  ******************************************************************************
  */
#include "bsp_led.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
} bsp_led_hw_t;

static const bsp_led_hw_t s_led_hw[BSP_LED_COUNT] =
{
    { BSP_LED_R_GPIO_PORT,    BSP_LED_R_GPIO_PIN    }, /* PF6: D6 LED_R */
    { BSP_LED_G_GPIO_PORT,    BSP_LED_G_GPIO_PIN    }, /* PF7: D6 LED_G */
    { BSP_LED_B_GPIO_PORT,    BSP_LED_B_GPIO_PIN    }, /* PF8: D6 LED_B */
    { BSP_LED_BLUE_GPIO_PORT, BSP_LED_BLUE_GPIO_PIN }  /* PC3: D48 BLUE */
};

void BSP_LED_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* 先置高电平保持熄灭，再切换推挽输出，避免上电瞬闪 */
    HAL_GPIO_WritePin(GPIOF,
                      BSP_LED_R_GPIO_PIN | BSP_LED_G_GPIO_PIN | BSP_LED_B_GPIO_PIN,
                      GPIO_PIN_SET);
    HAL_GPIO_WritePin(BSP_LED_BLUE_GPIO_PORT, BSP_LED_BLUE_GPIO_PIN, GPIO_PIN_SET);

    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    gpio.Pin = BSP_LED_R_GPIO_PIN | BSP_LED_G_GPIO_PIN | BSP_LED_B_GPIO_PIN;
    HAL_GPIO_Init(GPIOF, &gpio);

    gpio.Pin = BSP_LED_BLUE_GPIO_PIN;
    HAL_GPIO_Init(BSP_LED_BLUE_GPIO_PORT, &gpio);
}

void BSP_LED_Set(bsp_led_id_t led, uint8_t on)
{
    if (led >= BSP_LED_COUNT)
    {
        return;
    }

    HAL_GPIO_WritePin(s_led_hw[led].port,
                      s_led_hw[led].pin,
                      (on != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void BSP_LED_Toggle(bsp_led_id_t led)
{
    if (led >= BSP_LED_COUNT)
    {
        return;
    }

    HAL_GPIO_TogglePin(s_led_hw[led].port, s_led_hw[led].pin);
}

void BSP_LED_SetRgb(uint8_t r_on, uint8_t g_on, uint8_t b_on)
{
    BSP_LED_Set(BSP_LED_R, r_on);
    BSP_LED_Set(BSP_LED_G, g_on);
    BSP_LED_Set(BSP_LED_B, b_on);
}
