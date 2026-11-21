/**
  ******************************************************************************
  * @file    app_task.h
  * @brief   应用层-FreeRTOS 任务管理 (全工程唯一的任务创建 / 调度器启动入口)
  *          参考结构: D:\3251\f411ceu6\f411ceu6\application\app_task.h
  *          分层约束: 业务与驱动模块不直接调用 xTaskCreate / vTaskStartScheduler，
  *          所有任务创建与调度器启动统一归口于 app_task.c。
  *
  *          当前任务清单 (configMAX_PRIORITIES=32):
  *            start_task  prio 4, 2KB 栈 — 启动后初始化 lwIP (NO_SYS=0) 并创建应用任务，完成后自删
  *            touch       prio 2, 2KB 栈 — 电容触摸采集与画板绘制轮询
  *            led         prio 1, 1KB 栈 — 板载 LED 闪烁（D6 RGB 轮换 + D48 蓝灯心跳）
  *          lwIP 协议栈内部任务 (由 lwip_comm_init / tcpip_init / ethernetif_init 创建):
  *            eth_thread  prio 6, 2KB 栈 — ETH RX 中断信号量驱动收包
  *            TCP/IP      prio 5, 4KB 栈 — lwIP tcpip_thread 核心协议栈线程
  *            eth_link    prio 3, 1.5KB 栈 — PHY 链路状态监测与 MAC 速率/双工同步
  *            eth_dhcp    prio 2, 1.5KB 栈 — DHCP 状态机（仅 LWIP_DHCP=1 时启用）
  ******************************************************************************
  */
#ifndef __APP_TASK_H
#define __APP_TASK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief 创建应用层初始任务（调度器启动前调用一次；创建失败进入 Error_Handler）
  */
void App_Task_Create(void);

/**
  * @brief 启动 FreeRTOS 调度器（正常情况下永不返回）
  */
void App_Task_Start(void);

#ifdef __cplusplus
}
#endif

#endif /* __APP_TASK_H */
