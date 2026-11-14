/* drivers/input/touchscreen/gt9xx.h
 *
 * 2010 - 2013 Goodix Technology.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be a reference
 * to you, when you are integrating the GOODiX's CTP IC into your system,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#ifndef _GOODIX_GT9XX_H
#define _GOODIX_GT9XX_H

#include <stdint.h>

/* 默认读取屏幕原有配置；置 1 时只支持发送配套的 GT5688 配置。 */
#define UPDATE_CONFIG 0
#define GTP_MAX_TOUCH 5
#define GTP_POLL_TIME 10
#define GTP_ADDR_LENGTH 2
#define GTP_READ_COOR_ADDR 0x814E
#define GTP_REG_CONFIG_DATA 0x8050
#define GTP_REG_CONFIG_DATA_GT9X 0x8047
#define GTP_REG_VERSION 0x8140
#define GTP_REG_COMMAND 0x8040
#define GTP_SUCCESS 1
#define GTP_FAIL 0

typedef enum
{
    GT9157 = 0,
    GT911 = 1,
    GT5688 = 2
} TOUCH_IC;

extern TOUCH_IC touchIC;

/* 保留 Goodix 接口：初始化 0 成功，负值失败；I2C 读取返回 2，写入返回 1。 */
int32_t GTP_Init_Panel(void);
int32_t GTP_Read_Version(void);
int32_t GTP_I2C_Read_dbl_check(uint8_t address, uint16_t reg, uint8_t *buffer, int length);
int8_t GTP_Reset_Guitar(void);
int8_t GTP_Send_Command(uint8_t command);
int8_t GTP_WakeUp_Sleep(void);
void GTP_IRQ_Disable(void);
void GTP_IRQ_Enable(void);
/* 主循环调用，IRQ 只置标志；同时每 10 ms 检查一次状态。 */
void GTP_TouchProcess(void);

#endif
