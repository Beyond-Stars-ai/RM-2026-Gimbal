#include "Remote.h"
#include "main.h"

//===============变量区
extern UART_HandleTypeDef huart3;
extern DMA_HandleTypeDef hdma_usart3_rx;
const RC_ctrl_t *local_rc_ctrl;
static uint8_t sbus_rx_buf[2][SBUS_RX_BUF_NUM];
RC_ctrl_t rc_ctrl;

//===============内部函数
const RC_ctrl_t *remote_GetControlPoint(void)
{
    return &rc_ctrl;
}

static void remote_SbusToRC(volatile const uint8_t *sbus_buf, RC_ctrl_t *rc_ctrl)
{
    if (sbus_buf == NULL || rc_ctrl == NULL)
    {
        return;
    }

    rc_ctrl->rc.ch[0] = (sbus_buf[0] | (sbus_buf[1] << 8)) & 0x07ff;        //!< Channel 0
    rc_ctrl->rc.ch[1] = ((sbus_buf[1] >> 3) | (sbus_buf[2] << 5)) & 0x07ff; //!< Channel 1
    rc_ctrl->rc.ch[2] = ((sbus_buf[2] >> 6) | (sbus_buf[3] << 2) |          //!< Channel 2
                         (sbus_buf[4] << 10)) &
                        0x07ff;
    rc_ctrl->rc.ch[3] = ((sbus_buf[4] >> 1) | (sbus_buf[5] << 7)) & 0x07ff; //!< Channel 3
    rc_ctrl->rc.s[0] = ((sbus_buf[5] >> 4) & 0x0003);                       //!< Switch left
    rc_ctrl->rc.s[1] = ((sbus_buf[5] >> 4) & 0x000C) >> 2;                  //!< Switch right
    rc_ctrl->mouse.x = sbus_buf[6] | (sbus_buf[7] << 8);                    //!< Mouse X axis
    rc_ctrl->mouse.y = sbus_buf[8] | (sbus_buf[9] << 8);                    //!< Mouse Y axis
    rc_ctrl->mouse.z = sbus_buf[10] | (sbus_buf[11] << 8);                  //!< Mouse Z axis
    rc_ctrl->mouse.press_l = sbus_buf[12];                                  //!< Mouse Left Is Press ?
    rc_ctrl->mouse.press_r = sbus_buf[13];                                  //!< Mouse Right Is Press ?
    rc_ctrl->key.v = sbus_buf[14] | (sbus_buf[15] << 8);                    //!< KeyBoard value
    rc_ctrl->rc.ch[4] = sbus_buf[16] | (sbus_buf[17] << 8);                 // NULL

    rc_ctrl->rc.ch[0] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[1] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[2] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[3] -= RC_CH_VALUE_OFFSET;
    rc_ctrl->rc.ch[4] -= RC_CH_VALUE_OFFSET;
}

//===============公共函数
void Remote_Init(void)
{
    local_rc_ctrl = remote_GetControlPoint();
    Dbus_Dma_Init(sbus_rx_buf[0], sbus_rx_buf[1], SBUS_RX_BUF_NUM);
}

void Remote_UART_IDLE_Callback(void)
{
    if (huart3.Instance->SR & UART_FLAG_RXNE)
    {
        __HAL_UART_CLEAR_PEFLAG(&huart3);
    }
    else if (USART3->SR & UART_FLAG_IDLE)
    {
        static uint16_t this_time_rx_len = 0;

        __HAL_UART_CLEAR_PEFLAG(&huart3);

        if ((hdma_usart3_rx.Instance->CR & DMA_SxCR_CT) == RESET)
        {
            /* Current memory buffer used is Memory 0 */

            // disable DMA
            __HAL_DMA_DISABLE(&hdma_usart3_rx);

            // get receive data length, length = set_data_length - remain_length
            this_time_rx_len = SBUS_RX_BUF_NUM - hdma_usart3_rx.Instance->NDTR;

            // reset set_data_lenght
            hdma_usart3_rx.Instance->NDTR = SBUS_RX_BUF_NUM;

            // set memory buffer 1
            hdma_usart3_rx.Instance->CR |= DMA_SxCR_CT;

            // enable DMA
            __HAL_DMA_ENABLE(&hdma_usart3_rx);

            if (this_time_rx_len == RC_FRAME_LENGTH)
            {
                remote_SbusToRC(sbus_rx_buf[0], &rc_ctrl);
            }
        }
        else
        {
            /* Current memory buffer used is Memory 1 */
            // disable DMA
            __HAL_DMA_DISABLE(&hdma_usart3_rx);

            // get receive data length, length = set_data_length - remain_length
            this_time_rx_len = SBUS_RX_BUF_NUM - hdma_usart3_rx.Instance->NDTR;

            // reset set_data_lenght
            hdma_usart3_rx.Instance->NDTR = SBUS_RX_BUF_NUM;

            // set memory buffer 0
            DMA1_Stream1->CR &= ~(DMA_SxCR_CT);

            // enable DMA
            __HAL_DMA_ENABLE(&hdma_usart3_rx);

            if (this_time_rx_len == RC_FRAME_LENGTH)
            {
                remote_SbusToRC(sbus_rx_buf[1], &rc_ctrl);
            }
        }
    }
}
void CToC_MasterSendData(int16_t data1, int16_t data2,
                         int16_t data3, int16_t data4,
                         int16_t data5, int16_t data6,
                         CAN_HandleTypeDef *hcan)
{
    CAN_TxHeaderTypeDef Tx_Message;
    uint8_t Can_Send_Data_1[8];
    uint8_t Can_Send_Data_2[8];
    uint32_t send_mail_box;

    Tx_Message.StdId = 0x149;
    Tx_Message.RTR = CAN_RTR_DATA; // 数据帧
    Tx_Message.IDE = CAN_ID_STD;   // 标准格式
    Tx_Message.DLC = 0x08;         // 8字节数据段
    Can_Send_Data_1[0] = data1 >> 8;
    Can_Send_Data_1[1] = data1;
    Can_Send_Data_1[2] = data2 >> 8;
    Can_Send_Data_1[3] = data2;
    Can_Send_Data_1[4] = data3 >> 8;
    Can_Send_Data_1[5] = data3;
    Can_Send_Data_1[6] = data4 >> 8;
    Can_Send_Data_1[7] = data4;
    HAL_CAN_AddTxMessage(hcan, &Tx_Message, Can_Send_Data_1, &send_mail_box);

    Tx_Message.StdId = 0x189;  // 使用不同的ID
    Can_Send_Data_2[0] = data5 >> 8;
    Can_Send_Data_2[1] = data5;
    Can_Send_Data_2[2] = data6 >> 8;
    Can_Send_Data_2[3] = data6;
    Can_Send_Data_2[4] = 0;
    Can_Send_Data_2[5] = 0;
    Can_Send_Data_2[6] = 0;
    Can_Send_Data_2[7] = 0;
    HAL_CAN_AddTxMessage(hcan, &Tx_Message, Can_Send_Data_2, &send_mail_box);

}


// else if(ID==0x189)//接收遥控器控制数据
// 	{
// 		Remote_RxData.Remote_RS=(int16_t)((uint16_t)Data[2]<<8 | Data[3]);
// 		Remote_RxData.Remote_LS=(int16_t)((uint16_t)Data[0]<<8 | Data[1]);
// 	}
