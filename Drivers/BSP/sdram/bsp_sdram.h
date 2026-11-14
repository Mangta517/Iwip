#ifndef __SDRAM_H
#define __SDRAM_H

#include "stm32f4xx_hal.h"

/* 霸天虎 V2 的外部 SRAM：FSMC Bank1/NE4，1 MiB，不是 F429 的 SDRAM。 */
#define Bank1_SRAM4_ADDR 0x6C000000
#define IS62WV51216_SIZE 0x100000

void FSMC_SRAM_Init(void);
void FSMC_SRAM_MspInit(void);
/* 地址偏移单位为字节；NumHalfwordToWrite/Read 为 16 位半字数量。 */
void FSMC_SRAM_WriteBuffer(uint8_t *pBuffer, uint32_t WriteAddr, uint32_t NumHalfwordToWrite);
void FSMC_SRAM_ReadBuffer(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumHalfwordToRead);
/* 保留原例程的 32 位接口，数量单位为 uint32_t。 */
void SRAM_WriteBuffer(uint32_t *pBuffer, uint32_t address, uint32_t word_count);
void SRAM_ReadBuffer(uint32_t *pBuffer, uint32_t address, uint32_t word_count);
/* 手动调用会覆盖整个外部 SRAM；0 成功，1 失败，启动时不自动运行。 */
uint8_t SRAM_Test(void);

#endif
