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

#include "gt5xx.h"
#include "bsp_i2c_touch.h"
#include "bsp_nt35510_lcd.h"
#include "palette.h"

TOUCH_IC touchIC;

typedef struct
{
    uint16_t config_register;
    uint16_t width;
    uint16_t height;
    uint16_t active_ids;
    int16_t previous_x[GTP_MAX_TOUCH];
    int16_t previous_y[GTP_MAX_TOUCH];
    uint32_t poll_ms;
    uint8_t trigger;
    uint8_t initialized;
} touch_controller_struct;

static touch_controller_struct touch_controller;

static int32_t GTP_I2C_Read(uint8_t address, uint8_t *buffer, uint16_t length)
{
    uint8_t attempt;

    for (attempt = 0; attempt < 5; attempt++)
    {
        if (I2C_ReadRegister(address, ((uint16_t)buffer[0] << 8) | buffer[1], buffer + GTP_ADDR_LENGTH, length - GTP_ADDR_LENGTH) == 0)
        {
            return 2;
        }
    }
    return -1;
}

static int32_t GTP_I2C_Write(uint8_t address, uint8_t *buffer, uint16_t length)
{
    uint8_t attempt;

    for (attempt = 0; attempt < 5; attempt++)
    {
        if (I2C_WriteBytes(address, buffer, length) == 0)
        {
            return 1;
        }
    }
    return -1;
}

int32_t GTP_I2C_Read_dbl_check(uint8_t address, uint16_t reg, uint8_t *buffer, int length)
{
    uint8_t first[16] = {0};
    uint8_t second[16] = {0};
    uint8_t attempt;
    int index;

    if ((length <= 0) || (length > 14))
    {
        return GTP_FAIL;
    }
    first[0] = (uint8_t)(reg >> 8);
    first[1] = (uint8_t)reg;
    second[0] = first[0];
    second[1] = first[1];
    for (attempt = 0; attempt < 3; attempt++)
    {
        if ((GTP_I2C_Read(address, first, (uint16_t)(length + 2)) != 2) || (GTP_I2C_Read(address, second, (uint16_t)(length + 2)) != 2))
        {
            continue;
        }
        for (index = 0; index < length; index++)
        {
            if (first[index + 2] != second[index + 2])
            {
                break;
            }
        }
        if (index == length)
        {
            for (index = 0; index < length; index++)
            {
                buffer[index] = first[index + 2];
            }
            return GTP_SUCCESS;
        }
    }
    return GTP_FAIL;
}

void GTP_IRQ_Disable(void)
{
    I2C_GTP_IRQDisable();
}

void GTP_IRQ_Enable(void)
{
    I2C_GTP_IRQEnable(touch_controller.trigger);
}

int8_t GTP_Reset_Guitar(void)
{
    I2C_ResetChip();
    return 0;
}

int8_t GTP_Send_Command(uint8_t command)
{
    uint8_t buffer[3] = {GTP_REG_COMMAND >> 8, GTP_REG_COMMAND & 0xFF, 0};

    buffer[2] = command;
    return (int8_t)GTP_I2C_Write(GTP_ADDRESS, buffer, sizeof(buffer));
}

int8_t GTP_WakeUp_Sleep(void)
{
    uint8_t buffer[3] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xFF, 0};

    return (int8_t)GTP_I2C_Read(GTP_ADDRESS, buffer, sizeof(buffer));
}

int32_t GTP_Read_Version(void)
{
    uint8_t buffer[8] = {GTP_REG_VERSION >> 8, GTP_REG_VERSION & 0xFF};

    if (GTP_I2C_Read(GTP_ADDRESS, buffer, sizeof(buffer)) != 2)
    {
        return -1;
    }
    if ((buffer[2] == '5') && (buffer[3] == '6') && (buffer[4] == '8') && (buffer[5] == '8'))
    {
        touchIC = GT5688;
        touch_controller.config_register = GTP_REG_CONFIG_DATA;
    }
    else if ((buffer[2] == '9') && (buffer[3] == '1') && (buffer[4] == '1'))
    {
        touchIC = GT911;
        touch_controller.config_register = GTP_REG_CONFIG_DATA_GT9X;
    }
    else if ((buffer[2] == '9') && (buffer[3] == '1') && (buffer[4] == '5') && (buffer[5] == '7'))
    {
        touchIC = GT9157;
        touch_controller.config_register = GTP_REG_CONFIG_DATA_GT9X;
    }
    else
    {
        return -1;
    }
    return 2;
}

static int32_t GTP_Get_Info(void)
{
    uint8_t buffer[8] = {0};

    buffer[0] = (uint8_t)((touch_controller.config_register + 1) >> 8);
    buffer[1] = (uint8_t)(touch_controller.config_register + 1);

    if (GTP_I2C_Read(GTP_ADDRESS, buffer, sizeof(buffer)) != 2)
    {
        return GTP_FAIL;
    }
    touch_controller.width = (uint16_t)buffer[2] | ((uint16_t)buffer[3] << 8);
    touch_controller.height = (uint16_t)buffer[4] | ((uint16_t)buffer[5] << 8);
    touch_controller.trigger = buffer[7] & 0x03;
    if ((touch_controller.width < 2) || (touch_controller.height < 2))
    {
        return GTP_FAIL;
    }
    return GTP_SUCCESS;
}

#if UPDATE_CONFIG
const uint8_t CTP_CFG_GT5688[] = {
    0x96, 0xE0, 0x01, 0x56, 0x03, 0x05, 0x35, 0x00, 0x01, 0x00, 0x00, 0x05, 0x50, 0x3C, 0x53, 0x11, 0x00, 0x00, 0x22, 0x22, 0x14, 0x18,
    0x1A, 0x1D, 0x0A, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x00, 0x14, 0x00, 0x00, 0x84, 0x00, 0x00, 0x3C, 0x19, 0x19, 0x64,
    0x1E, 0x28, 0x88, 0x29, 0x0A, 0x2D, 0x2F, 0x29, 0x0C, 0x20, 0x33, 0x60, 0x13, 0x02, 0x24, 0x00, 0x00, 0x20, 0x3C, 0xC0, 0x14, 0x02,
    0x00, 0x00, 0x54, 0xAC, 0x24, 0x9C, 0x29, 0x8C, 0x2D, 0x80, 0x32, 0x77, 0x37, 0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0,
    0x50, 0x3C, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x02, 0x14, 0x14, 0x03, 0x04, 0x00, 0x21, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x32, 0x20,
    0x50, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x06, 0x0C, 0x05, 0x0B, 0x04, 0x0A, 0x03, 0x09, 0x02, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x05, 0x1E,
    0x00, 0x02, 0x2A, 0x1E, 0x19, 0x14, 0x02, 0x00, 0x03, 0x0A, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0x86,
    0x22, 0x03, 0x00, 0x00, 0x33, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x50, 0x3C, 0x50, 0x00, 0x00, 0x00, 0x1A, 0x64, 0x01

};

static int32_t gtp_send_config(void)
{
    uint8_t buffer[sizeof(CTP_CFG_GT5688) + 2];
    uint16_t checksum = 0;
    uint16_t index;

    if (touchIC != GT5688)
    {
        return -1;
    }
    buffer[0] = GTP_REG_CONFIG_DATA >> 8;
    buffer[1] = GTP_REG_CONFIG_DATA & 0xFF;
    for (index = 0; index < sizeof(CTP_CFG_GT5688); index++)
    {
        buffer[index + 2] = CTP_CFG_GT5688[index];
    }
    buffer[3] = (uint8_t)LCD_X_LENGTH;
    buffer[4] = (uint8_t)(LCD_X_LENGTH >> 8);
    buffer[5] = (uint8_t)LCD_Y_LENGTH;
    buffer[6] = (uint8_t)(LCD_Y_LENGTH >> 8);
    for (index = 2; index < sizeof(buffer) - 3; index += 2)
    {
        checksum += ((uint16_t)buffer[index] << 8) | buffer[index + 1];
    }
    checksum = (uint16_t)(0 - checksum);
    buffer[sizeof(buffer) - 3] = (uint8_t)(checksum >> 8);
    buffer[sizeof(buffer) - 2] = (uint8_t)checksum;
    buffer[sizeof(buffer) - 1] = 1;
    if (GTP_I2C_Write(GTP_ADDRESS, buffer, sizeof(buffer)) != 1)
    {
        return -1;
    }
    HAL_Delay(10);
    return 0;
}
#endif

int32_t GTP_Init_Panel(void)
{
    uint8_t index;

    touch_controller.initialized = 0;
    I2C_Touch_Init();
    if (GTP_Read_Version() != 2)
    {
        return -1;
    }
#if UPDATE_CONFIG
    if (gtp_send_config() != 0)
    {
        return -1;
    }
#endif
    if (GTP_Get_Info() != GTP_SUCCESS)
    {
        return -1;
    }
    for (index = 0; index < GTP_MAX_TOUCH; index++)
    {
        touch_controller.previous_x[index] = -1;
        touch_controller.previous_y[index] = -1;
    }
    touch_controller.active_ids = 0;
    touch_controller.poll_ms = HAL_GetTick();
    touch_controller.initialized = 1;
    GTP_IRQ_Enable();
    return 0;
}

static void Goodix_TS_Work_Func(void)
{
    uint8_t status[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};
    uint8_t points[2 + 8 * GTP_MAX_TOUCH] = {(GTP_READ_COOR_ADDR + 1) >> 8, (GTP_READ_COOR_ADDR + 1) & 0xFF};
    uint16_t active = 0;
    uint16_t x;
    uint16_t y;
    uint16_t limit_x = touch_controller.width;
    uint16_t limit_y = touch_controller.height;
    uint16_t swap;
    uint8_t count;
    uint8_t index;
    uint8_t id;
    uint8_t *point;

    if ((GTP_I2C_Read(GTP_ADDRESS, status, sizeof(status)) != 2) || ((status[2] & 0x80) == 0))
    {
        return;
    }
    count = status[2] & 0x0F;
    if (count > GTP_MAX_TOUCH)
    {
        status[2] = 0;
        GTP_I2C_Write(GTP_ADDRESS, status, sizeof(status));
        return;
    }
    if ((count != 0) && (GTP_I2C_Read(GTP_ADDRESS, points, (uint16_t)(2 + count * 8)) != 2))
    {
        return;
    }
    /* 尽早释放控制器的数据就绪标志，绘图在主循环中完成。 */
    status[2] = 0;
    if (GTP_I2C_Write(GTP_ADDRESS, status, sizeof(status)) != 1)
    {
        return;
    }
    for (index = 0; index < count; index++)
    {
        point = &points[2 + index * 8];
        id = point[0] & 0x0F;
        x = (uint16_t)point[1] | ((uint16_t)point[2] << 8);
        y = (uint16_t)point[3] | ((uint16_t)point[4] << 8);
        if ((id >= GTP_MAX_TOUCH) || (x >= touch_controller.width) || (y >= touch_controller.height))
        {
            continue;
        }
        limit_x = touch_controller.width;
        limit_y = touch_controller.height;
        /* 触摸原始 X/Y 与扫描模式 6 的显示轴相反，换轴时同步交换量程。 */
        if ((LCD_SCAN_MODE & 1) == 0)
        {
            swap = x;
            x = y;
            y = swap;
            swap = limit_x;
            limit_x = limit_y;
            limit_y = swap;
        }
        x = (uint16_t)((uint32_t)x * (LCD_X_LENGTH - 1) / (limit_x - 1));
        y = (uint16_t)((uint32_t)y * (LCD_Y_LENGTH - 1) / (limit_y - 1));
        if ((LCD_SCAN_MODE & 2) == 0)
        {
            x = LCD_X_LENGTH - 1 - x;
        }
        /* 触摸原始 Y 方向与扫描模式 6 的 MY 极性相反。 */
        if ((LCD_SCAN_MODE & 4) != 0)
        {
            y = LCD_Y_LENGTH - 1 - y;
        }
        active |= (uint16_t)(1 << id);
        Touch_Button_Down(x, y);
        Draw_Trail(touch_controller.previous_x[id], touch_controller.previous_y[id], x, y, &brush);
        touch_controller.previous_x[id] = (int16_t)x;
        touch_controller.previous_y[id] = (int16_t)y;
    }
    for (id = 0; id < GTP_MAX_TOUCH; id++)
    {
        if (((touch_controller.active_ids & (1 << id)) != 0) && ((active & (1 << id)) == 0))
        {
            Touch_Button_Up(touch_controller.previous_x[id], touch_controller.previous_y[id]);
            touch_controller.previous_x[id] = -1;
            touch_controller.previous_y[id] = -1;
        }
    }
    touch_controller.active_ids = active;
}

void GTP_TouchProcess(void)
{
    uint32_t now = HAL_GetTick();
    uint8_t pending;

    if (touch_controller.initialized == 0)
    {
        return;
    }
    pending = I2C_Touch_Pending();
    if ((pending == 0) && ((now - touch_controller.poll_ms) < GTP_POLL_TIME))
    {
        return;
    }
    touch_controller.poll_ms = now;
    Goodix_TS_Work_Func();
}
