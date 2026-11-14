/**
 * @brief 野火 F407 霸天虎 V2 电容触摸屏软件 I2C。
 * 基于野火 bsp_i2c_touch 驱动移植；PD7 SCL、PD3 SDA、PD6 RST、PG8 INT。
 */
#include "bsp_i2c_touch.h"

static uint32_t i2c_delay_cycles;
static volatile uint8_t touch_pending;

static void i2c_delay(void)
{
    uint32_t start = DWT->CYCCNT;

    /* 使用当前 HCLK 计时，避免空循环被优化后改变 I2C 时序。 */
    while ((uint32_t)(DWT->CYCCNT - start) < i2c_delay_cycles)
    {
    }
}

static void i2c_scl(GPIO_PinState state)
{
    HAL_GPIO_WritePin(GTP_I2C_SCL_GPIO_PORT, GTP_I2C_SCL_PIN, state);
}

static void i2c_sda(GPIO_PinState state)
{
    HAL_GPIO_WritePin(GTP_I2C_SDA_GPIO_PORT, GTP_I2C_SDA_PIN, state);
}

static void i2c_start(void)
{
    i2c_sda(GPIO_PIN_SET);
    i2c_scl(GPIO_PIN_SET);
    i2c_delay();
    i2c_sda(GPIO_PIN_RESET);
    i2c_delay();
    i2c_scl(GPIO_PIN_RESET);
}

static void i2c_stop(void)
{
    i2c_sda(GPIO_PIN_RESET);
    i2c_delay();
    i2c_scl(GPIO_PIN_SET);
    i2c_delay();
    i2c_sda(GPIO_PIN_SET);
    i2c_delay();
}

static uint8_t i2c_write_byte(uint8_t value)
{
    uint8_t bit;
    uint8_t nack;

    for (bit = 0; bit < 8; bit++)
    {
        if ((value & 0x80) != 0)
        {
            i2c_sda(GPIO_PIN_SET);
        }
        else
        {
            i2c_sda(GPIO_PIN_RESET);
        }
        i2c_delay();
        i2c_scl(GPIO_PIN_SET);
        i2c_delay();
        i2c_scl(GPIO_PIN_RESET);
        value <<= 1;
    }

    i2c_sda(GPIO_PIN_SET);
    i2c_delay();
    i2c_scl(GPIO_PIN_SET);
    i2c_delay();
    nack = (uint8_t)HAL_GPIO_ReadPin(GTP_I2C_SDA_GPIO_PORT, GTP_I2C_SDA_PIN);
    i2c_scl(GPIO_PIN_RESET);
    return nack;
}

static uint8_t i2c_read_byte(uint8_t last)
{
    uint8_t value = 0;
    uint8_t bit;

    i2c_sda(GPIO_PIN_SET);
    for (bit = 0; bit < 8; bit++)
    {
        value <<= 1;
        i2c_delay();
        i2c_scl(GPIO_PIN_SET);
        i2c_delay();
        if (HAL_GPIO_ReadPin(GTP_I2C_SDA_GPIO_PORT, GTP_I2C_SDA_PIN) == GPIO_PIN_SET)
        {
            value |= 1;
        }
        i2c_scl(GPIO_PIN_RESET);
    }

    /* 先收完数据，再应答；最后一个字节发 NACK，随后统一发送 STOP。 */
    if (last != 0)
    {
        i2c_sda(GPIO_PIN_SET);
    }
    else
    {
        i2c_sda(GPIO_PIN_RESET);
    }
    i2c_delay();
    i2c_scl(GPIO_PIN_SET);
    i2c_delay();
    i2c_scl(GPIO_PIN_RESET);
    i2c_sda(GPIO_PIN_SET);
    return value;
}

uint32_t I2C_WriteBytes(uint8_t address, uint8_t *buffer, uint16_t length)
{
    uint16_t index;

    i2c_start();
    if (i2c_write_byte(address & 0xFE) != 0)
    {
        i2c_stop();
        return 1;
    }
    for (index = 0; index < length; index++)
    {
        if (i2c_write_byte(buffer[index]) != 0)
        {
            i2c_stop();
            return 1;
        }
    }
    i2c_stop();
    return 0;
}

uint32_t I2C_ReadBytes(uint8_t address, uint8_t *buffer, uint16_t length)
{
    uint16_t index;
    uint8_t last;

    i2c_start();
    if (i2c_write_byte(address | 0x01) != 0)
    {
        i2c_stop();
        return 1;
    }
    for (index = 0; index < length; index++)
    {
        last = 0;
        if (index + 1 == length)
        {
            last = 1;
        }
        buffer[index] = i2c_read_byte(last);
    }
    i2c_stop();
    return 0;
}

/* 寄存器地址和读数据之间使用 repeated START，不插入 STOP。 */
uint32_t I2C_ReadRegister(uint8_t address, uint16_t reg, uint8_t *buffer, uint16_t length)
{
    i2c_start();
    if ((i2c_write_byte(address & 0xFE) != 0) || (i2c_write_byte((uint8_t)(reg >> 8)) != 0) || (i2c_write_byte((uint8_t)reg) != 0))
    {
        i2c_stop();
        return 1;
    }
    return I2C_ReadBytes(address, buffer, length);
}

void I2C_GTP_IRQDisable(void)
{
    HAL_NVIC_DisableIRQ(GTP_INT_EXTI_IRQ);
    __HAL_GPIO_EXTI_CLEAR_IT(GTP_INT_GPIO_PIN);
}

void I2C_GTP_IRQEnable(uint8_t trigger)
{
    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = GTP_INT_GPIO_PIN;
    gpio.Mode = GPIO_MODE_IT_RISING;
    if (trigger == 1)
    {
        gpio.Mode = GPIO_MODE_IT_FALLING;
    }
    else if (trigger >= 2)
    {
        /* F407 EXTI 使用边沿；控制器的电平触发配置由主循环定时读状态补充。 */
        gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
    }
    gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GTP_INT_GPIO_PORT, &gpio);
    __HAL_GPIO_EXTI_CLEAR_IT(GTP_INT_GPIO_PIN);
    HAL_NVIC_ClearPendingIRQ(GTP_INT_EXTI_IRQ);
    HAL_NVIC_SetPriority(GTP_INT_EXTI_IRQ, 6, 0);
    HAL_NVIC_EnableIRQ(GTP_INT_EXTI_IRQ);
}

void I2C_ResetChip(void)
{
    GPIO_InitTypeDef gpio = {0};

    I2C_GTP_IRQDisable();
    gpio.Pin = GTP_INT_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_WritePin(GTP_INT_GPIO_PORT, GTP_INT_GPIO_PIN, GPIO_PIN_RESET);
    HAL_GPIO_Init(GTP_INT_GPIO_PORT, &gpio);
    HAL_GPIO_WritePin(GTP_RST_GPIO_PORT, GTP_RST_GPIO_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(GTP_RST_GPIO_PORT, GTP_RST_GPIO_PIN, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GTP_INT_GPIO_PORT, GTP_INT_GPIO_PIN, GPIO_PIN_RESET);
    HAL_Delay(50);
    gpio.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GTP_INT_GPIO_PORT, &gpio);
    touch_pending = 0;
}

void I2C_Touch_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    i2c_delay_cycles = HAL_RCC_GetHCLKFreq() / 500000;

    gpio.Pin = GTP_I2C_SCL_PIN | GTP_I2C_SDA_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_WritePin(GPIOD, gpio.Pin, GPIO_PIN_SET);
    HAL_GPIO_Init(GPIOD, &gpio);
    gpio.Pin = GTP_RST_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GTP_RST_GPIO_PORT, &gpio);
    I2C_ResetChip();
}

void GTP_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(GTP_INT_GPIO_PIN) != RESET)
    {
        __HAL_GPIO_EXTI_CLEAR_IT(GTP_INT_GPIO_PIN);
        touch_pending = 1;
    }
}

uint8_t I2C_Touch_Pending(void)
{
    uint32_t primask = __get_PRIMASK();
    uint8_t pending;

    __disable_irq();
    pending = touch_pending;
    touch_pending = 0;
    __set_PRIMASK(primask);
    return pending;
}
