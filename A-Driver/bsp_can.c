#include "bsp_can.h"
#include "main.h"
#include "Motor.h"
#define get_motor_measure(ptr, data)                                    \
    {                                                                   \
        (ptr)->last_ecd = (ptr)->ecd;                                   \
        (ptr)->ecd = (uint16_t)((data)[0] << 8 | (data)[1]);            \
        (ptr)->speed_rpm = (uint16_t)((data)[2] << 8 | (data)[3]);      \
        (ptr)->given_current = (uint16_t)((data)[4] << 8 | (data)[5]);  \
        (ptr)->temperate = (data)[6];                                   \
    }
//===============变量区
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
motor_measure_t Can1_Rx_Data[8];
motor_measure_t Can2_Rx_Data[8];

//===============公共函数
void Can_Filter_Init(void)
{
    CAN_FilterTypeDef can_filter_st;
    can_filter_st.FilterActivation 	= ENABLE;
    can_filter_st.FilterMode 				= CAN_FILTERMODE_IDMASK;//CAN_FILTERMODE_IDLIST//CAN_FILTERMODE_IDMASK
    can_filter_st.FilterScale 			= CAN_FILTERSCALE_16BIT;
    can_filter_st.FilterIdHigh 			= 0x0000; //(0x207<<5)
    can_filter_st.FilterIdLow 			= 0x0000;
    can_filter_st.FilterMaskIdHigh 	= 0x0000;//(0x1FF<<5)
    can_filter_st.FilterMaskIdLow 	= 0x0000;
    can_filter_st.FilterBank 				= 0;
    can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;
    HAL_CAN_ConfigFilter(&hcan1, &can_filter_st);
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

    can_filter_st.SlaveStartFilterBank = 14;
    can_filter_st.FilterBank = 14;
	
    HAL_CAN_ConfigFilter(&hcan2, &can_filter_st);
    HAL_CAN_Start(&hcan2);
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);

}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef rx_header;
    uint8_t rx_data[8];

    // 读取接收到的消息
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
        return; // 安全检查

    // 只处理标准帧
    if (rx_header.IDE != CAN_ID_STD)
        return;

    // 根据 CAN 外设实例区分总线
    if (hcan == &hcan1)
    {
        switch (rx_header.StdId)
			{
					case 0x201:
							CAN1_M3508_DataProcess(0x201,rx_data);break;
					case 0x202:
							CAN1_M3508_DataProcess(0x202,rx_data);break;
					case 0x203:
							break;
					case 0x204:
							break;
					case 0x205:
							break;
					case 0x206:
							CAN1_M6020_DataProcess(0x206,rx_data);break;
					case 0x207:
							CAN1_M2006_DataProcess(0x207,rx_data);break;
					case 0x208:
							break;
					default:
					{
							break;
					}
			}
    }
    else if (hcan == &hcan2)
    {
				switch (rx_header.StdId)
			{
					case 0x201:
							break;
					case 0x202:
							break;
					case 0x203:
							break;
					case 0x204:
							break;
					case 0x205:
							CAN2_M6020_DataProcess(0x205,rx_data);break;
					case 0x206:
							CAN2_M6020_DataProcess(0x206,rx_data);break;
					case 0x207:
							break;
					case 0x208:
							break;
					default:
					{
							break;
					}
			}
    }
}
