#ifndef BSP_USART_H
#define BSP_USART_H
#include <stdint.h> 
extern void usart1_tx_dma_init(void);
extern void usart1_tx_dma_enable(uint8_t *data, uint16_t len);

//extern uint8_t UART1_RxData;
//extern uint8_t UART1_RxFlag;
//extern uint8_t UART2_RxData;
//extern uint8_t UART2_RxFlag;

//void UART1_SendInit(void);
//void UART1_ReceiveInit(void);
//void UART1_Init(void);
//void UART1_SendByte(uint8_t Byte);
//void UART1_SendArray(uint8_t *Array,uint16_t Length);
//void UART1_SendString(char *String);
//void UART1_SendNumber(uint32_t Number,uint8_t Length);
//void UART1_Printf(char *format,...);
//uint8_t UART1_GetRxFlag(void);

void UART2_SendByte(uint8_t Byte);
void UART2_SendArray(uint8_t *Array,uint16_t Length);
void UART2_SendString(char *String);
void UART2_SendNumber(uint32_t Number,uint8_t Length);
//void UART2_Printf(char *format,...);
uint32_t USART_Pow(uint32_t X, uint32_t Y);
//uint8_t UART2_GetRxFlag(void);

#endif
