/*
 * FreeRTOS V202212.01 (Kernel V10.5.1)
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * Target: STM32F407ZGT6 (野火霸天虎 V2, 168 MHz, Cortex-M4F)
 * Structure reference: D:\3251\f411ceu6\f411ceu6
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ((unsigned long)168000000) /* F407: HSE 25MHz / 25 * 336 / 2 = 168MHz */
#define configTICK_RATE_HZ                      1000                       /* 1ms 系统节拍，与 HAL_IncTick 同步 */
#define configMAX_PRIORITIES                    (32)                       /* 优先级 0..31，与参考例程一致并满足 lwIP 各线程分层 */
#define configMINIMAL_STACK_SIZE                ((unsigned short)128)
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   ((size_t)(32 * 1024))      /* 32KB heap_4 堆，位于主 SRAM (IRAM1) */
#define configMAX_TASK_NAME_LEN                 (16)
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TIME_SLICING                  1
#define configUSE_QUEUE_SETS                    1
#define configENABLE_BACKWARD_COMPATIBILITY     1

/* 同步原语与软件定时器 */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            (configMINIMAL_STACK_SIZE * 2)

/* 栈溢出检测（方法 2：检查栈指针与栈底哨兵），回调实现于 application/app_task.c */
#define configCHECK_FOR_STACK_OVERFLOW          2

#if defined(__CC_ARM) || defined(__ICCARM__) || defined(__GNUC__)
void Error_Handler(void);
#endif
#define configASSERT(condition) do { \
    if (!(condition)) \
    { \
        Error_Handler(); \
    } \
} while (0)

/* 可选 API 裁剪配置 */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xTaskAbortDelay                 1
#define INCLUDE_xTaskGetHandle                  1

/* STM32F407 使用 4 位 NVIC 抢占优先级 (NVIC_PRIORITYGROUP_4，逻辑优先级 0..15)。
 * configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY=5 表示逻辑优先级 5..15 的中断可调用 FromISR API；
 * 现有触摸 EXTI 优先级为 6，落在该受管区间内。 */
#define configPRIO_BITS                         4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY         (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY configLIBRARY_LOWEST_INTERRUPT_PRIORITY

/* 异常向量直接进入 FreeRTOS 汇编端口，保留硬件写入 LR 的 EXC_RETURN */
#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler

#endif /* FREERTOS_CONFIG_H */
