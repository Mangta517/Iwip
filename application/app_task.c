/**
  ******************************************************************************
  * @file    app_task.c
  * @brief   应用层-FreeRTOS 任务管理实现
  *          本工程所有 xTaskCreate / vTaskStartScheduler 统一收口在本文件。
  *          内核: Middlewares/FreeRTOS V10.5.1 (V202212.01, RVDS/ARM_CM4F port, heap_4)。
  ******************************************************************************
  */
#include "app_task.h"

#include "main.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"

#include "lwip_comm.h"
#include "bsp_led.h"
#include "gt5xx.h"

/*********************
 *  任务配置
 *********************/
/* configMAX_PRIORITIES=32，软件定时器任务占最高优先级 31；
 * lwIP 内部线程: eth_thread=6, TCP/IP=5, eth_link=3, eth_dhcp=2；
 * 应用层启动任务: start_task=4（初始化 lwIP 与业务任务后自删），touch=2，led=1。 */
#define APP_START_TASK_PRIO      4
#define APP_START_TASK_STK       512  /* 字 = 2KB */
#define APP_LWIP_INIT_MAX_RETRY  3

#define APP_TOUCH_TASK_PRIO      2    /* 触摸与画板任务 */
#define APP_TOUCH_TASK_STK       512  /* 字 = 2KB */
#define APP_TOUCH_TASK_PERIOD_MS 10   /* 10ms 轮询周期，与 GTP_POLL_TIME 一致 */

#define APP_LED_TASK_PRIO        1    /* 板载 LED 闪烁任务（最低业务优先级） */
#define APP_LED_TASK_STK         256  /* 字 = 1KB */
#define APP_LED_BLINK_PERIOD_MS  500  /* 500ms 闪烁周期 */

extern volatile uint8_t g_lwip_init_status;

static TaskHandle_t s_start_task_handle;
static TaskHandle_t s_touch_task_handle;
static TaskHandle_t s_led_task_handle;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void app_start_task(void *pvParameters);
static void app_touch_task(void *pvParameters);
static void app_led_task(void *pvParameters);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void App_Task_Create(void)
{
    if (xTaskCreate(app_start_task, "start_task", APP_START_TASK_STK, NULL,
                    APP_START_TASK_PRIO, &s_start_task_handle) != pdPASS)
    {
        Error_Handler(); /* FreeRTOS 堆不足 */
    }
}

void App_Task_Start(void)
{
    vTaskStartScheduler(); /* 正常永不返回 */
    Error_Handler();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
/* 启动任务：在 FreeRTOS 调度器启动后初始化 lwIP (NO_SYS=0) 并创建业务任务，完成后删除自身。 */
static void app_start_task(void *pvParameters)
{
    uint8_t retry;

    (void)pvParameters;

    /* 调度器已启动且尚未派生其他业务/网络线程，此时安全创建串口互斥锁。 */
    USART1_InitMutex();

    for (retry = 0; retry < APP_LWIP_INIT_MAX_RETRY; retry++)
    {
        g_lwip_init_status = lwip_comm_init();
        if (g_lwip_init_status == 0)
        {
            break;
        }
        usart_printf("[ETH] lwip_comm_init failed (%u), retry %u/%u...\r\n",
                     g_lwip_init_status, (unsigned int)(retry + 1U),
                     (unsigned int)APP_LWIP_INIT_MAX_RETRY);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    if (g_lwip_init_status != 0)
    {
        usart_printf("[ETH] lwip_comm_init aborted: %u\r\n", g_lwip_init_status);
    }

    if (xTaskCreate(app_touch_task, "touch", APP_TOUCH_TASK_STK, NULL,
                    APP_TOUCH_TASK_PRIO, &s_touch_task_handle) != pdPASS)
    {
        Error_Handler(); /* FreeRTOS 堆不足 */
    }

    if (xTaskCreate(app_led_task, "led", APP_LED_TASK_STK, NULL,
                    APP_LED_TASK_PRIO, &s_led_task_handle) != pdPASS)
    {
        Error_Handler(); /* FreeRTOS 堆不足 */
    }

    s_start_task_handle = NULL;
    vTaskDelete(NULL);
}

/* 触摸任务：独占软件 I2C 触摸读取与画板刷新 */
static void app_touch_task(void *pvParameters)
{
    (void)pvParameters;

    for (;;)
    {
        GTP_TouchProcess();
        vTaskDelay(pdMS_TO_TICKS(APP_TOUCH_TASK_PERIOD_MS));
    }
}

/* LED 任务：每 500ms 轮换 D6 RGB（红->绿->蓝）并翻转 D48 独立蓝灯（PC3） */
static void app_led_task(void *pvParameters)
{
    uint8_t step = 0U;

    (void)pvParameters;

    for (;;)
    {
        switch (step)
        {
            case 0U:
                BSP_LED_SetRgb(1U, 0U, 0U);
                break;
            case 1U:
                BSP_LED_SetRgb(0U, 1U, 0U);
                break;
            default:
                BSP_LED_SetRgb(0U, 0U, 1U);
                break;
        }
        step = (uint8_t)((step + 1U) % 3U);

        BSP_LED_Toggle(BSP_LED_BLUE);
        vTaskDelay(pdMS_TO_TICKS(APP_LED_BLINK_PERIOD_MS));
    }
}

/**********************
 *  栈溢出安全网 (configCHECK_FOR_STACK_OVERFLOW=2)
 *********************/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    Error_Handler();
}
