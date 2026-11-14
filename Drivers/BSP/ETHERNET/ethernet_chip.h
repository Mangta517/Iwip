/**
 ****************************************************************************************************
 * @file        ethernet_chip.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-08-01
 * @brief       PHY芯片配置驱动
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
 * V1.0 20220420
 * 第一次发布
 *
 ****************************************************************************************************
 */
#ifndef ETHERENT_CHIP_H
#define ETHERENT_CHIP_H
#include <stdint.h>



#ifdef __cplusplus
 extern "C" {
#endif   

/* PHY芯片寄存器映射表 */ 
#define ETH_CHIP_BCR                            ((uint16_t)0x0000U)
#define ETH_CHIP_BSR                            ((uint16_t)0x0001U)

/* 操作SCR寄存器的值（一般不需要修改） */
#define ETH_CHIP_BCR_SOFT_RESET                 ((uint16_t)0x8000U)
#define ETH_CHIP_BCR_LOOPBACK                   ((uint16_t)0x4000U)
#define ETH_CHIP_BCR_SPEED_SELECT               ((uint16_t)0x2000U)
#define ETH_CHIP_BCR_AUTONEGO_EN                ((uint16_t)0x1000U)
#define ETH_CHIP_BCR_POWER_DOWN                 ((uint16_t)0x0800U)
#define ETH_CHIP_BCR_ISOLATE                    ((uint16_t)0x0400U)
#define ETH_CHIP_BCR_RESTART_AUTONEGO           ((uint16_t)0x0200U)
#define ETH_CHIP_BCR_DUPLEX_MODE                ((uint16_t)0x0100U) 

/* 操作BSR寄存器的值（一般不需要修改） */   
#define ETH_CHIP_BSR_100BASE_T4                 ((uint16_t)0x8000U)
#define ETH_CHIP_BSR_100BASE_TX_FD              ((uint16_t)0x4000U)
#define ETH_CHIP_BSR_100BASE_TX_HD              ((uint16_t)0x2000U)
#define ETH_CHIP_BSR_10BASE_T_FD                ((uint16_t)0x1000U)
#define ETH_CHIP_BSR_10BASE_T_HD                ((uint16_t)0x0800U)
#define ETH_CHIP_BSR_100BASE_T2_FD              ((uint16_t)0x0400U)
#define ETH_CHIP_BSR_100BASE_T2_HD              ((uint16_t)0x0200U)
#define ETH_CHIP_BSR_EXTENDED_STATUS            ((uint16_t)0x0100U)
#define ETH_CHIP_BSR_AUTONEGO_CPLT              ((uint16_t)0x0020U)
#define ETH_CHIP_BSR_REMOTE_FAULT               ((uint16_t)0x0010U)
#define ETH_CHIP_BSR_AUTONEGO_ABILITY           ((uint16_t)0x0008U)
#define ETH_CHIP_BSR_LINK_STATUS                ((uint16_t)0x0004U)
#define ETH_CHIP_BSR_JABBER_DETECT              ((uint16_t)0x0002U)
#define ETH_CHIP_BSR_EXTENDED_CAP               ((uint16_t)0x0001U)


/* PHY芯片进程状态 */
#define  ETH_CHIP_STATUS_READ_ERROR             ((int32_t)-5)
#define  ETH_CHIP_STATUS_WRITE_ERROR            ((int32_t)-4)
#define  ETH_CHIP_STATUS_ADDRESS_ERROR          ((int32_t)-3)
#define  ETH_CHIP_STATUS_RESET_TIMEOUT          ((int32_t)-2)
#define  ETH_CHIP_STATUS_ERROR                  ((int32_t)-1)
#define  ETH_CHIP_STATUS_OK                     ((int32_t) 0)
#define  ETH_CHIP_STATUS_LINK_DOWN              ((int32_t) 1)
#define  ETH_CHIP_STATUS_100MBITS_FULLDUPLEX    ((int32_t) 2)
#define  ETH_CHIP_STATUS_100MBITS_HALFDUPLEX    ((int32_t) 3)
#define  ETH_CHIP_STATUS_10MBITS_FULLDUPLEX     ((int32_t) 4)
#define  ETH_CHIP_STATUS_10MBITS_HALFDUPLEX     ((int32_t) 5)
#define  ETH_CHIP_STATUS_AUTONEGO_NOTDONE       ((int32_t) 6)

/* LAN8720A PHY ID 与寄存器 31 的协商结果，低 4 位芯片版本不参与匹配。 */
#define ETH_CHIP_PHYID1                         0x02
#define ETH_CHIP_PHYID2                         0x03
#define ETH_CHIP_PHYID1_VALUE                   0x0007
#define ETH_CHIP_PHYID2_VALUE                   0xC0F0
#define ETH_CHIP_PHYID2_MASK                    0xFFF0
#define ETH_CHIP_PHYSCSR_HCDSPEED_MASK          0x001C
#define ETH_CHIP_PHYSCSR_10M_HALF               0x0004
#define ETH_CHIP_PHYSCSR_10M_FULL               0x0014
#define ETH_CHIP_PHYSCSR_100M_HALF              0x0008
#define ETH_CHIP_PHYSCSR_100M_FULL              0x0018
#define ETH_CHIP_PHYSCSR_AUTONEGO_DONE          0x1000

/* PHY地址 */
#define ETH_CHIP_ADDR                           ((uint16_t)0x0000U)
/* PHY寄存器的数量 */
#define ETH_CHIP_PHY_COUNT                      ((uint16_t)0x001FU)

/* 选择PHY芯片 */
#define LAN8720                                 0
#define SR8201F                                 1
#define YT8512C                                 2
#define RTL8201                                 3
#define PHY_TYPE                                LAN8720


#if(PHY_TYPE == LAN8720) 
#define ETH_CHIP_PHYSCSR                        ((uint16_t)0x1F)                       /*!< tranceiver status register */
#define ETH_CHIP_SPEED_STATUS                   ((uint16_t)0x0004)                     /* bit2=1: 10M, bit2=0: 100M */
#define ETH_CHIP_DUPLEX_STATUS                  ((uint16_t)0x0010)                     /*!< configured information of duplex: full-duplex */
#elif(PHY_TYPE == SR8201F)
#define ETH_CHIP_PHYSCSR                        ((uint16_t)0x00)                       /*!< tranceiver status register */
#define ETH_CHIP_SPEED_STATUS                   ((uint16_t)0x2020)                     /*!< configured information of speed: 100Mbit/s */
#define ETH_CHIP_DUPLEX_STATUS                  ((uint16_t)0x0100)                     /*!< configured information of duplex: full-duplex */
#elif(PHY_TYPE == YT8512C)
#define ETH_CHIP_PHYSCSR                        ((uint16_t)0x11)                       /*!< tranceiver status register */
#define ETH_CHIP_SPEED_STATUS                   ((uint16_t)0x4010)                     /*!< configured information of speed: 100Mbit/s */
#define ETH_CHIP_DUPLEX_STATUS                  ((uint16_t)0x2000)                     /*!< configured information of duplex: full-duplex */
#elif(PHY_TYPE == RTL8201)
#define PHY_SR                                  ((uint16_t)0x10)                       /*!< tranceiver status register */
#define ETH_CHIP_SPEED_STATUS                   ((uint16_t)0x0022)                     /*!< configured information of speed: 100Mbit/s */
#define ETH_CHIP_DUPLEX_STATUS                  ((uint16_t)0x0004)                     /*!< configured information of duplex: full-duplex */
#endif /* PHY_TYPE */

/* 定义函数指针 */ 
typedef int32_t  (*eth_chip_init_func)          (void); 
typedef int32_t  (*eth_chip_deinit_func)        (void);
typedef int32_t  (*eth_chip_readreg_func)       (uint32_t, uint32_t, uint32_t *);
typedef int32_t  (*eth_chip_writereg_func)      (uint32_t, uint32_t, uint32_t);
typedef int32_t  (*eth_chip_gettick_func)       (void);

/* PHY共用函数结构体 */ 
typedef struct 
{
    eth_chip_init_func          init;                   /* 指向PHY初始化函数 */ 
    eth_chip_deinit_func        deinit;                 /* 指向PHY反初始化函数 */ 
    eth_chip_writereg_func      writereg;               /* 指向PHY写寄存器函数 */ 
    eth_chip_readreg_func       readreg;                /* 指向PHY读寄存器函数 */ 
    eth_chip_gettick_func       gettick;                /* 指向节拍函数 */ 
} eth_chip_ioc_tx_t;  

/* 注册到组件对象结构体 */
typedef struct 
{
    uint32_t            devaddr;                        /* PHY地址 */
    uint32_t            is_initialized;                 /* 描述该设备是否初始化 */
    eth_chip_ioc_tx_t   io;                             /* 设备调用的函数入口 */
    void                *pdata;                         /* 传入的形参 */
}eth_chip_object_t;



int32_t eth_chip_regster_bus_io(eth_chip_object_t *pobj, eth_chip_ioc_tx_t *ioctx);             /* 将IO函数注册到组件对象 */
int32_t eth_chip_init(eth_chip_object_t *pobj);                                                 /* 初始化ETH_CHIP并配置所需的硬件资源 */
int32_t eth_chip_deinit(eth_chip_object_t *pobj);                                               /* 反初始化ETH_CHIP及其硬件资源 */
int32_t eth_chip_disable_power_down_mode(eth_chip_object_t *pobj);                              /* 关闭ETH_CHIP的下电模式 */
int32_t eth_chip_enable_power_down_mode(eth_chip_object_t *pobj);                               /* 使能ETH_CHIP的下电模式 */
int32_t eth_chip_start_auto_nego(eth_chip_object_t *pobj);                                      /* 启动自动协商过程 */
int32_t eth_chip_get_link_state(eth_chip_object_t *pobj);                                       /* 获取ETH_CHIP设备的链路状态 */
int32_t eth_chip_set_link_state(eth_chip_object_t *pobj, uint32_t LinkState);                   /* 设置ETH_CHIP设备的链路状态 */
int32_t eth_chip_enable_loop_back_mode(eth_chip_object_t *pobj);                                /* 启用环回模式 */
int32_t eth_chip_disable_loop_back_mode(eth_chip_object_t *pobj);                               /* 禁用环回模式 */

#ifdef __cplusplus
}
#endif

#endif
