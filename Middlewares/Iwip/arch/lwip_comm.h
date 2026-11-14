/**
 ****************************************************************************************************
 * @file        lwip_comm.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-08-01
 * @brief       lwIP配置驱动
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 参考平台:正点原子 阿波罗 F429开发板
 * 移植目标:野火 STM32F407 霸天虎 V2，LAN8720A
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211202
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef _LWIP_COMM_H
#define _LWIP_COMM_H

#include <stdint.h>
#include "lwip/netif.h"

#define LWIP_DHCP_OFF                   0
#define LWIP_DHCP_START                 1
#define LWIP_DHCP_WAIT_ADDRESS          2
#define LWIP_DHCP_ADDRESS_ASSIGNED      3
#define LWIP_DHCP_TIMEOUT               4
#define LWIP_DHCP_LINK_DOWN             5
#define LWIP_MAX_DHCP_TRIES             4

typedef struct
{
    uint8_t mac[6];
    uint8_t remoteip[4];
    uint8_t ip[4];
    uint8_t netmask[4];
    uint8_t gateway[4];
    uint8_t dhcpstatus;                 /* 0: 静态/未启动，1: 申请中，2: 已获取，0xFF: 超时回退。 */
} __lwip_dev;

extern __lwip_dev g_lwipdev;
extern struct netif g_lwip_netif;

void lwip_comm_default_ip_set(__lwip_dev *lwipx);
uint8_t lwip_comm_init(void);
void lwip_pkt_handle(void);
void lwip_periodic_handle(void);
void lwip_link_status_updated(struct netif *netif);

#endif
