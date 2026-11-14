/**
 ****************************************************************************************************
 * @file        lwip_comm.c
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

#include "lwip_comm.h"
#include "ethernetif.h"
#include "usart.h"
#include "stm32f4xx_hal.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/dhcp.h"
#include "netif/ethernet.h"

#define LWIP_LINK_CHECK_INTERVAL_MS     100

__lwip_dev g_lwipdev;
struct netif g_lwip_netif;
static uint32_t lwip_link_timer;

#if LWIP_DHCP
static uint32_t lwip_dhcp_timer;
static uint8_t g_lwip_dhcp_state = LWIP_DHCP_OFF;

static void lwip_dhcp_process(struct netif *netif)
{
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gateway;
    struct dhcp *dhcp;

    switch (g_lwip_dhcp_state)
    {
        case LWIP_DHCP_START:
            ip4_addr_set_zero(&ipaddr);
            netif_set_addr(netif, &ipaddr, &ipaddr, &ipaddr);
            if (dhcp_start(netif) == ERR_OK)
            {
                g_lwipdev.dhcpstatus = 1;
                g_lwip_dhcp_state = LWIP_DHCP_WAIT_ADDRESS;
                usart_printf("[NET] DHCP started, waiting for IP...\r\n");
            }
            break;

        case LWIP_DHCP_WAIT_ADDRESS:
        case LWIP_DHCP_ADDRESS_ASSIGNED:
            if (dhcp_supplied_address(netif))
            {
                uint8_t newly_assigned = (g_lwip_dhcp_state != LWIP_DHCP_ADDRESS_ASSIGNED);
                g_lwip_dhcp_state = LWIP_DHCP_ADDRESS_ASSIGNED;
                g_lwipdev.dhcpstatus = 2;
                g_lwipdev.ip[0] = ip4_addr1(netif_ip4_addr(netif));
                g_lwipdev.ip[1] = ip4_addr2(netif_ip4_addr(netif));
                g_lwipdev.ip[2] = ip4_addr3(netif_ip4_addr(netif));
                g_lwipdev.ip[3] = ip4_addr4(netif_ip4_addr(netif));
                g_lwipdev.netmask[0] = ip4_addr1(netif_ip4_netmask(netif));
                g_lwipdev.netmask[1] = ip4_addr2(netif_ip4_netmask(netif));
                g_lwipdev.netmask[2] = ip4_addr3(netif_ip4_netmask(netif));
                g_lwipdev.netmask[3] = ip4_addr4(netif_ip4_netmask(netif));
                g_lwipdev.gateway[0] = ip4_addr1(netif_ip4_gw(netif));
                g_lwipdev.gateway[1] = ip4_addr2(netif_ip4_gw(netif));
                g_lwipdev.gateway[2] = ip4_addr3(netif_ip4_gw(netif));
                g_lwipdev.gateway[3] = ip4_addr4(netif_ip4_gw(netif));
                if (newly_assigned)
                {
                    usart_printf("[NET] DHCP IP: %u.%u.%u.%u, Mask: %u.%u.%u.%u, GW: %u.%u.%u.%u\r\n",
                                 g_lwipdev.ip[0], g_lwipdev.ip[1], g_lwipdev.ip[2], g_lwipdev.ip[3],
                                 g_lwipdev.netmask[0], g_lwipdev.netmask[1], g_lwipdev.netmask[2], g_lwipdev.netmask[3],
                                 g_lwipdev.gateway[0], g_lwipdev.gateway[1], g_lwipdev.gateway[2], g_lwipdev.gateway[3]);
                }
            }
            else
            {
                g_lwipdev.dhcpstatus = 1;
                dhcp = netif_dhcp_data(netif);
                if ((dhcp != NULL) && (dhcp->tries > LWIP_MAX_DHCP_TRIES))
                {
                    dhcp_stop(netif);
                    lwip_comm_default_ip_set(&g_lwipdev);
                    IP4_ADDR(&ipaddr, g_lwipdev.ip[0], g_lwipdev.ip[1], g_lwipdev.ip[2], g_lwipdev.ip[3]);
                    IP4_ADDR(&netmask, g_lwipdev.netmask[0], g_lwipdev.netmask[1], g_lwipdev.netmask[2], g_lwipdev.netmask[3]);
                    IP4_ADDR(&gateway, g_lwipdev.gateway[0], g_lwipdev.gateway[1], g_lwipdev.gateway[2], g_lwipdev.gateway[3]);
                    netif_set_addr(netif, &ipaddr, &netmask, &gateway);
                    g_lwipdev.dhcpstatus = 0xFF;
                    g_lwip_dhcp_state = LWIP_DHCP_TIMEOUT;
                    usart_printf("[NET] DHCP timeout, static IP: %u.%u.%u.%u, Mask: %u.%u.%u.%u, GW: %u.%u.%u.%u\r\n",
                                 g_lwipdev.ip[0], g_lwipdev.ip[1], g_lwipdev.ip[2], g_lwipdev.ip[3],
                                 g_lwipdev.netmask[0], g_lwipdev.netmask[1], g_lwipdev.netmask[2], g_lwipdev.netmask[3],
                                 g_lwipdev.gateway[0], g_lwipdev.gateway[1], g_lwipdev.gateway[2], g_lwipdev.gateway[3]);
                }
            }
            break;

        case LWIP_DHCP_LINK_DOWN:
            dhcp_stop(netif);
            ip4_addr_set_zero(&ipaddr);
            netif_set_addr(netif, &ipaddr, &ipaddr, &ipaddr);
            g_lwipdev.dhcpstatus = 0;
            g_lwip_dhcp_state = LWIP_DHCP_OFF;
            usart_printf("[NET] DHCP stopped (link down)\r\n");
            break;

        default:
            break;
    }
}
#endif

void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
    uint32_t uid = HAL_GetUIDw0() ^ HAL_GetUIDw1() ^ HAL_GetUIDw2();

    /* 使用本地管理的单播 MAC，后 5 字节由 STM32 唯一 ID 派生。 */
    lwipx->mac[0] = 0x02;
    lwipx->mac[1] = (uint8_t)(HAL_GetUIDw1() >> 8);
    lwipx->mac[2] = (uint8_t)(uid >> 24);
    lwipx->mac[3] = (uint8_t)(uid >> 16);
    lwipx->mac[4] = (uint8_t)(uid >> 8);
    lwipx->mac[5] = (uint8_t)uid;

    lwipx->remoteip[0] = 192;
    lwipx->remoteip[1] = 168;
    lwipx->remoteip[2] = 1;
    lwipx->remoteip[3] = 134;
    lwipx->ip[0] = 192;
    lwipx->ip[1] = 168;
    lwipx->ip[2] = 1;
    lwipx->ip[3] = 30;
    lwipx->netmask[0] = 255;
    lwipx->netmask[1] = 255;
    lwipx->netmask[2] = 255;
    lwipx->netmask[3] = 0;
    lwipx->gateway[0] = 192;
    lwipx->gateway[1] = 168;
    lwipx->gateway[2] = 1;
    lwipx->gateway[3] = 1;
    lwipx->dhcpstatus = 0;
}

uint8_t lwip_comm_init(void)
{
    ip4_addr_t ipaddr;
    ip4_addr_t netmask;
    ip4_addr_t gateway;

    lwip_comm_default_ip_set(&g_lwipdev);
    lwip_init();

#if LWIP_DHCP
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);
    ip4_addr_set_zero(&gateway);
#else
    IP4_ADDR(&ipaddr, g_lwipdev.ip[0], g_lwipdev.ip[1], g_lwipdev.ip[2], g_lwipdev.ip[3]);
    IP4_ADDR(&netmask, g_lwipdev.netmask[0], g_lwipdev.netmask[1], g_lwipdev.netmask[2], g_lwipdev.netmask[3]);
    IP4_ADDR(&gateway, g_lwipdev.gateway[0], g_lwipdev.gateway[1], g_lwipdev.gateway[2], g_lwipdev.gateway[3]);
#endif

    if (netif_add(&g_lwip_netif, &ipaddr, &netmask, &gateway, NULL, ethernetif_init, ethernet_input) == NULL)
    {
        return 1;
    }

    netif_set_default(&g_lwip_netif);
    netif_set_link_callback(&g_lwip_netif, lwip_link_status_updated);
    netif_set_up(&g_lwip_netif);
    ethernetif_link_check_state(&g_lwip_netif);
    lwip_link_timer = HAL_GetTick();

    usart_printf("[ETH] lwIP init ok, MAC: %02X:%02X:%02X:%02X:%02X:%02X, Link: %s\r\n",
                 g_lwipdev.mac[0], g_lwipdev.mac[1], g_lwipdev.mac[2],
                 g_lwipdev.mac[3], g_lwipdev.mac[4], g_lwipdev.mac[5],
                 netif_is_link_up(&g_lwip_netif) ? "UP" : "DOWN");
#if LWIP_DHCP
    usart_printf("[NET] Mode: DHCP\r\n");
#else
    usart_printf("[NET] Mode: Static, IP: %u.%u.%u.%u, Mask: %u.%u.%u.%u, GW: %u.%u.%u.%u\r\n",
                 g_lwipdev.ip[0], g_lwipdev.ip[1], g_lwipdev.ip[2], g_lwipdev.ip[3],
                 g_lwipdev.netmask[0], g_lwipdev.netmask[1], g_lwipdev.netmask[2], g_lwipdev.netmask[3],
                 g_lwipdev.gateway[0], g_lwipdev.gateway[1], g_lwipdev.gateway[2], g_lwipdev.gateway[3]);
#endif
    return 0;
}

void lwip_pkt_handle(void)
{
    if (netif_is_link_up(&g_lwip_netif))
    {
        ethernetif_input(&g_lwip_netif);
    }
}

void lwip_link_status_updated(struct netif *netif)
{
#if LWIP_DHCP
    if (netif_is_link_up(netif))
    {
        g_lwip_dhcp_state = LWIP_DHCP_START;
    }
    else
    {
        g_lwip_dhcp_state = LWIP_DHCP_LINK_DOWN;
    }
    lwip_dhcp_process(netif);
#else
    LWIP_UNUSED_ARG(netif);
#endif
}

void lwip_periodic_handle(void)
{
    uint32_t now = HAL_GetTick();

    if ((now - lwip_link_timer) >= LWIP_LINK_CHECK_INTERVAL_MS)
    {
        lwip_link_timer = now;
        ethernetif_link_check_state(&g_lwip_netif);
    }

    sys_check_timeouts();
#if LWIP_DHCP
    if ((now - lwip_dhcp_timer) >= DHCP_FINE_TIMER_MSECS)
    {
        lwip_dhcp_timer = now;
        lwip_dhcp_process(&g_lwip_netif);
    }
#endif
}
