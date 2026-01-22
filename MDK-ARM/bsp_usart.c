#include "bsp_usart.h"
#include "usart.h"
#include "main.h"
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_tx;
void usart1_tx_dma_init(void)
{
    //enable the DMA transfer for the receiver request
    SET_BIT(huart1.Instance->CR3, USART_CR3_DMAT);
}
void usart1_tx_dma_enable(uint8_t *data, uint16_t len)
{

    //disable DMA
    __HAL_DMA_DISABLE(&hdma_usart1_tx);
    while(hdma_usart1_tx.Instance->CR & DMA_SxCR_EN)
    {
        __HAL_DMA_DISABLE(&hdma_usart1_tx);
    }

    //clear flag
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_HISR_TCIF7);
    __HAL_DMA_CLEAR_FLAG(&hdma_usart1_tx, DMA_HISR_HTIF7);

    //set data address
    hdma_usart1_tx.Instance->M0AR = (uint32_t)(data);
    //set data length
    hdma_usart1_tx.Instance->NDTR = len;

    //enable DMA
    __HAL_DMA_ENABLE(&hdma_usart1_tx);
}

void UART2_SendByte(uint8_t Byte)
{
		HAL_UART_Transmit(&huart1, &Byte, 1, 0xFFFF);//
}

void UART2_SendArray(uint8_t *Array,uint16_t Length)
{
	for(uint16_t i=0;i<Length;i++)
		UART2_SendByte(Array[i]);
}

void UART2_SendString(char *String)
{
	for(uint8_t i=0;String[i]!='\0';i++)
		UART2_SendByte(String[i]);//依次发送字符串的每一位
}

void UART2_SendNumber(uint32_t Number,uint8_t Length)
{
	for(uint8_t i=0;i<Length;i++)
		UART2_SendByte(Number/USART_Pow(10,Length-i-1)%10+'0');//以文本形式发送数字每一位
}

uint32_t USART_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while(Y--)
	{
		Result *= X;
	}
	return Result;
}



