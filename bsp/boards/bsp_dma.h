#ifndef BSP_DMA_H
#define BSP_DMA_H
#include <stdint.h> 

extern void Remote_DMA_init(uint8_t *rx1_buf, uint8_t *rx2_buf, uint16_t dma_buf_num);

// extern void RC_unable(void);
// extern void RC_restart(uint16_t dma_buf_num);

#endif
