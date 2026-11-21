# lwip：STM32F407 裸机 lwIP、LCD 与触摸

本工程参考 `E:\BaiduNetdiskDownload\lwIP例程1 lwIP裸机移植.zip` 的分层，适配野火 STM32F407 霸天虎 V2。Keil 工程入口为 [MDK-ARM/lwip.uvprojx](MDK-ARM/lwip.uvprojx)，目标名为 `lwip`；CubeMX 工程为 [lwip.ioc](lwip.ioc)。

## 文件分层

Keil 分组与磁盘路径对应，lwIP 的 API、公共核心、IPv4、IPv6、网卡适配分别成组。

```text
Startup
  MDK-ARM/startup_stm32f407xx.s
Core/Inc
  main.h、gpio.h、HAL 配置、中断声明
Core/Src
  main.c、gpio.c、HAL MSP、中断、系统时钟
Drivers/BSP/ETHERNET
  ethernet.c/.h          F407 MAC、DMA 描述符、RMII 引脚和时钟
  ethernet_chip.c/.h     LAN8720A 识别、复位、协商和链路状态
Drivers/BSP/lcd
  bsp_nt35510_lcd.c/.h   NT35510、FSMC 16 位接口与绘图
Drivers/BSP/touch
  bsp_i2c_touch.c/.h     软件 I2C、复位与 EXTI 标志
  gt5xx.c/.h            Goodix 识别、坐标和触点状态
  palette.c/.h          配套触摸画板
Drivers/BSP/font
  fonts.c/.h            内置 ASCII 字体与外部 GB2312 字库读取
  resources/            外部 Flash 中文字模资源
Drivers/BSP/flash
  bsp_spi_flash.c/.h    SPI1 W25Q128 驱动
Drivers/BSP/sdram
  bsp_sdram.c/.h        保留导入路径，实际为 FSMC 外部 SRAM
Drivers/STM32F4xx_HAL_Driver
  ST 官方 HAL 外设驱动
Middlewares/Iwip/arch
  cc.h                  编译器适配
  lwipopts.h            协议栈和内存配置
  ethernetif.c/.h       pbuf 与 HAL 收发、MAC 速率和双工同步
  lwip_comm.c/.h        网卡、IP、DHCP 与裸机轮询入口
Middlewares/Iwip/src/api
  lwIP API 源文件；NO_SYS 模式关闭 netconn/socket
Middlewares/Iwip/src/core
  协议栈公共核心、内存池、TCP、UDP、定时器
Middlewares/Iwip/src/core/ipv4
  ARP、IPv4、ICMP、DHCP 等
Middlewares/Iwip/src/core/ipv6
  保留协议栈原文件，当前 LWIP_IPV6=0
Middlewares/Iwip/src/netif
  以太网帧协议入口
Readme
  README-lwIP.md
```

保留现有 CubeMX 的 `Core/Inc` 和 `Core/Src`。LCD、触摸、字体、SPI Flash 与外部 SRAM 分别使用各自的 BSP 分组。lwIP 使用自身的静态堆和内存池，位于内部 SRAM。现有 `Drivers/BSP/IIC` 文件保留在原位置，触摸屏使用专用的软件 I2C 驱动。

## F407 板级差异

引脚按本地《野火_F407霸天虎_原理图_V2.0.pdf》第 9 页 ETH+SPDIF 核对。

| 信号 | STM32F407 引脚 |
| --- | --- |
| RMII_REF_CLK | PA1 |
| MDIO | PA2 |
| CRS_DV | PA7 |
| MDC | PC1 |
| RXD0 / RXD1 | PC4 / PC5 |
| TX_EN | PG11 |
| TXD0 / TXD1 | PG13 / PG14 |
| PHY nRST | 与整板 NRST 相连，无独立 GPIO |

F407 使用现有 25 MHz HSE、168 MHz 系统时钟。LAN8720A 使用板上独立 25 MHz 晶振，经 REFCLKO 向 PA1 提供 50 MHz RMII 时钟，无需 MCU 输出 MCO。

PHY 驱动扫描 MDIO 地址并匹配 LAN8720A ID（ID1 为 `0x0007`，ID2 屏蔽版本号后为 `0xC0F0`），不把状态寄存器的低位当成地址。链路状态连续读取两次 BSR，再按寄存器 31 的位 4:2 解码 10/100M、半/全双工。寄存器定义依据 [Microchip LAN8720A 数据手册](https://ww1.microchip.com/downloads/en/DeviceDoc/00002165B.pdf)。

DMA 描述符、RX 内存池和 TX 缓冲放在 `0x20000000` 起的 SRAM，不能迁入 F407 的 CCM。接收使用自定义 pbuf 池；发送将 pbuf 链合并到描述符独占的 SRAM 缓冲。IP、UDP、TCP、ICMP 采用全硬件校验（`CHECKSUM_GEN_*` / `CHECKSUM_CHECK_*` 均为 0，MAC `ChecksumOffload` 与 TX 描述符硬件校验插入均启用）。

## 裸机调用与网络参数

`main.c` 在 HAL、时钟和 GPIO 初始化之后，依次初始化外部 SRAM、LCD、画板、触摸，再调用 `lwip_comm_init()`。网络初始化失败时 `g_lwip_init_status=1`，进入 `Error_Handler()`；返回 0 表示协议栈和 PHY 初始化成功，不表示网线已连接。触摸初始化失败时 `g_touch_init_status=1`，网络轮询仍继续运行。

主循环持续执行：

```c
lwip_pkt_handle();
lwip_periodic_handle();
GTP_TouchProcess();
```

`lwip_periodic_handle()` 调用 `sys_check_timeouts()`，每 100 ms 检查 PHY 链路；MAC 在协商完成后按实际速率和双工启动。拔线后停止 MAC，重新接线并完成协商后再次启动。

当前默认配置与参考例程保持一致：

| 参数 | 默认值 |
| --- | --- |
| 本机 IPv4 | 192.168.1.30 |
| 子网掩码 | 255.255.255.0 |
| 网关 | 192.168.1.1 |
| DHCP | 关闭，`LWIP_DHCP=0` |
| MAC | `02` 开头，后 5 字节由 STM32 唯一 ID 派生 |

静态地址在 `lwip_comm_default_ip_set()` 修改。将 `lwipopts.h` 的 `LWIP_DHCP` 改为 1，或在工程宏中添加 `LWIP_DHCP=1` 后全量重编译，可启用 DHCP；申请超过设定重试次数后回退静态地址。插拔网线会停止并重新启动 DHCP。

调试时查看 `g_lwip_init_status`、`g_eth_handler`、`g_lwip_netif` 和 `g_lwipdev`。裸机环境使用 lwIP raw API，后续 TCP/UDP 应用需要在相应应用模块中实现。

本次为手工移植，`lwip.ioc` 保留原有外设生成配置。以后重新生成 CubeMX 工程时，需要核对 `HAL_ETH_MODULE_ENABLED`、`HAL_SPI_MODULE_ENABLED`、`HAL_SRAM_MODULE_ENABLED`，以及手工添加的 BSP/HAL/FSMC 源文件与分组；不要再生成一套重复的 ETH 句柄或 `HAL_ETH_MspInit()`。

含中文的 BSP 驱动和 lwIP 移植文件统一使用带 BOM 的 UTF-8，供 Keil 和编辑器识别。转换时逐文件确认了解码前后的文字和代码完全一致，保留 CRLF 换行。后续保存这些文件时继续使用 UTF-8 with BOM，避免再混入 GBK。

## LCD、触摸和存储

默认上电进入配套画板，使用 NT35510 的扫描模式 6、480×800 显示区域；触摸处理与 lwIP 收发都在主循环中执行。

| 模块 | 配置与引脚 |
| --- | --- |
| LCD 数据总线 | FSMC 16 位，PD0/1/8/9/10/14/15，PE7～15 |
| LCD 控制 | PG10/NE3，PF0/A0，PD4/NOE，PD5/NWE |
| LCD 复位、背光 | PF11、PF9，普通 GPIO 输出 |
| 触摸 | PD7/SCL、PD3/SDA、PD6/RST、PG8/INT |
| SPI Flash | SPI1，PB3/SCK、PB4/MISO、PB5/MOSI，PG6/CS |
| 外部 SRAM | FSMC NE4/PG12，地址 `0x6C000000`，容量 1 MiB |

LCD 使用 `0x68000000` 写命令、`0x68000002` 写数据。LCD 的 NE3 与 SRAM 的 NE4 共用 FSMC 数据线，分别配置片选时序；初始化不会复位整个 FSMC 总线。

Goodix 驱动识别 GT5688、GT911、GT9157。默认 `UPDATE_CONFIG=0`，读取屏幕现有配置，按分辨率缩放坐标；GT5688 使用 `0x8050` 配置区，GT9x 使用 `0x8047` 配置区，型号差异与 [Linux 上游 Goodix 驱动](https://github.com/torvalds/linux/blob/master/drivers/input/touchscreen/goodix.c)的寄存器布局核对。默认扫描模式 6 为原画板的坐标基准，其余扫描方向的实际触点朝向仍需实屏确认。

软件 I2C 使用 repeated START 读取寄存器，最后一个字节收完后发送 NACK，再发送 STOP。EXTI 中断只设置标志，主循环处理坐标与绘图；同时每 10 ms 轮询，覆盖控制器的电平触发配置。初始化和错误路径均不依赖尚未配置的调试串口。

ASCII 8×16、16×32、24×48 的三套字模数据保持原样。画板中的中文字符串使用显式 GB2312 字节，源文件注释使用 UTF-8 BOM。中文字库从外部 SPI Flash 的 `0x4E6000` 开始读取，32×32 字模每字 128 字节。

| 交付文件 | 用途与目标地址 |
| --- | --- |
| `MDK-ARM/lwip/lwip.bin` | STM32 内部 Flash，`0x08000000` |
| `Drivers/BSP/font/resources/GB2312_H3232.bin` | 外部 SPI Flash 字库区，`0x4E6000` |

字库资源来自本机野火 F407 HAL 配套资料包的 `GB2312_H3232.FON`，保留全部 87×94×128 字节字模，去掉文件末尾的 CRLF。固件读取字库，不自动擦写外部 Flash。板上该区域尚未写入正确字库时，中文显示需要先准备相应资源。

`SRAM_Test()` 会覆盖整个外部 SRAM，供手动验证；默认启动只初始化 SRAM 控制器。SRAM 的字节地址偏移、16 位半字接口和 32 位字接口已在头文件分别注明。

## 验证与上板检查

构建及主机测试记录位于 [work/lwip-port-20261112](work/lwip-port-20261112)。静态 IPv4 和 DHCP 两种配置均使用 ARMCC 5.06 全量编译。PHY 测试直接编译当前 `ethernet_chip.c`，覆盖地址扫描、ID 不匹配、复位超时、tick 回绕、四种协商结果、强制模式、链路锁存和 MDIO 错误。

主机 PHY 测试可在工程根目录执行：

```powershell
& 'C:\msys64\mingw64\bin\gcc.exe' -std=c99 -Wall -Wextra -Werror -I Drivers/BSP/ETHERNET Drivers/BSP/ETHERNET/ethernet_chip.c work/lwip-port-20261112/test_lan8720a.c -o work/lwip-port-20261112/test_lan8720a.exe
& .\work\lwip-port-20261112\test_lan8720a.exe
```

默认固件为 `MDK-ARM/lwip/lwip.bin`，链接起始地址为 `0x08000000`。实板运行需要另行烧录验证：

1. 让电脑与开发板处于同一网段，且 `192.168.1.30` 未被占用。
2. 上电后检查网口链路灯及调试器中的网卡链路标志。
3. 在电脑执行 `ping 192.168.1.30`，并进行网线拔插后的再次 ping。
4. 若启用 DHCP，在有 DHCP 服务的网络中确认实际租约地址。

目前尚未烧录，也未验证实板 RMII 波形、ping、DHCP 租约或 TCP/UDP 通信。

LCD/触摸/存储的本次移植记录位于 [work/lcd-touch-port-20261112](work/lcd-touch-port-20261112)。主机测试直接编译当前 Goodix 与字体源码，检查通信失败、触点边界、多指松开、坐标缩放、命令参数、两类配置地址及 GB2312 字库地址计算。实板显示、触摸朝向、I2C 时序、外部 Flash/SRAM 读写尚待验证。

## 备份

仅保留 `MDK-ARM/lwip.uvprojx` 一个主工程。DHCP 作为同一工程的编译选项，验证用的临时工程和独立输出已删除，检查结果保留在日志中。

原始源码、改名之前的工程与用户选项、编码转换前的文件集中保存在 `work/lwip-port-20261112/restore-original.zip`，压缩后已逐文件校验内容一致。工程根文件夹已改为 `lwip`（原为 `batiannhuv2`）。

LCD、触摸、字体、Flash 和 SRAM 本次接入前的文件保存在 `work/lcd-touch-port-20261112/before.zip`。所有备份均为压缩包，工作区只保留一个可构建主工程。
