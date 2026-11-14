/**
 ****************************************************************************************************
 * @file        ethernet.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-08-01
 * @brief       ETHERNET 驱动代码
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
 * V1.0 20211014
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "ethernet.h"

ETH_HandleTypeDef g_eth_handler;

/* F407 的 ETH DMA 使用 0x20000000 起的 SRAM，不能放入 0x10000000 的 CCM。 */
static ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __ALIGNED(4);
static ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __ALIGNED(4);

uint8_t ethernet_init(uint8_t *mac_address)
{
    /* PHY nRST 接整板 NRST；等待上电稳定后由 PHY 驱动执行软件复位。 */
    HAL_Delay(50);

    g_eth_handler.Instance = ETH;
    g_eth_handler.Init.MACAddr = mac_address;
    g_eth_handler.Init.MediaInterface = HAL_ETH_RMII_MODE;
    g_eth_handler.Init.RxDesc = DMARxDscrTab;
    g_eth_handler.Init.TxDesc = DMATxDscrTab;
    g_eth_handler.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;

    if (HAL_ETH_Init(&g_eth_handler) != HAL_OK)
    {
        return 1;
    }

    return 0;
}

void HAL_ETH_MspInit(ETH_HandleTypeDef *heth)
{
    GPIO_InitTypeDef gpio_init_struct = {0};

    if (heth->Instance != ETH)
    {
        return;
    }

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_ETH_CLK_ENABLE();

    /* PA1 接 LAN8720A 的 50 MHz REFCLKO，不使用 MCU MCO 输出时钟。 */
    gpio_init_struct.Pin = ETH_CLK_GPIO_PIN | ETH_MDIO_GPIO_PIN | ETH_CRS_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;
    gpio_init_struct.Pull = GPIO_NOPULL;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init_struct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);

    gpio_init_struct.Pin = ETH_MDC_GPIO_PIN | ETH_RXD0_GPIO_PIN | ETH_RXD1_GPIO_PIN;
    HAL_GPIO_Init(GPIOC, &gpio_init_struct);

    gpio_init_struct.Pin = ETH_TX_EN_GPIO_PIN | ETH_TXD0_GPIO_PIN | ETH_TXD1_GPIO_PIN;
    HAL_GPIO_Init(GPIOG, &gpio_init_struct);
}
