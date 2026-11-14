#ifndef __I2C_TOUCH_H
#define __I2C_TOUCH_H

#include "stm32f4xx_hal.h"

/* INT 在复位释放时拉低，选择 7 位地址 0x5D；总线接口传入左移后的 0xBA。 */
#define GTP_ADDRESS 0xBA
#define GTP_I2C_SCL_PIN GPIO_PIN_7
#define GTP_I2C_SCL_GPIO_PORT GPIOD
#define GTP_I2C_SDA_PIN GPIO_PIN_3
#define GTP_I2C_SDA_GPIO_PORT GPIOD
#define GTP_RST_GPIO_PORT GPIOD
#define GTP_RST_GPIO_PIN GPIO_PIN_6
#define GTP_INT_GPIO_PORT GPIOG
#define GTP_INT_GPIO_PIN GPIO_PIN_8
#define GTP_INT_EXTI_IRQ EXTI9_5_IRQn
#define GTP_IRQHandler EXTI9_5_IRQHandler

void I2C_Touch_Init(void);
uint32_t I2C_WriteBytes(uint8_t address, uint8_t *buffer, uint16_t length);
uint32_t I2C_ReadBytes(uint8_t address, uint8_t *buffer, uint16_t length);
uint32_t I2C_ReadRegister(uint8_t address, uint16_t reg, uint8_t *buffer, uint16_t length);
void I2C_ResetChip(void);
void I2C_GTP_IRQDisable(void);
void I2C_GTP_IRQEnable(uint8_t trigger);
uint8_t I2C_Touch_Pending(void);

#endif
