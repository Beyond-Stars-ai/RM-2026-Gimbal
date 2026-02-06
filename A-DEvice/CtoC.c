#include "CtoC.h"

void CToC_MasterSendData(	int16_t data1, int16_t data2, 
															int16_t data3, int16_t data4, 
															CAN_HandleTypeDef *hcan)
{
	CAN_TxHeaderTypeDef  Tx_Message;
	uint8_t	Can_Send_Data[8];
  uint32_t send_mail_box;
	
	Tx_Message.StdId = 0x149;
	Tx_Message.RTR=CAN_RTR_DATA;//数据帧
	Tx_Message.IDE=CAN_ID_STD;//标准格式
	Tx_Message.DLC=0x08;//8字节数据段
  Can_Send_Data[0] = data1 >> 8;
  Can_Send_Data[1] = data1;
  Can_Send_Data[2] = data2 >> 8;
  Can_Send_Data[3] = data2;
  Can_Send_Data[4] = data3 >> 8;
  Can_Send_Data[5] = data3;
  Can_Send_Data[6] = data4 >> 8;
  Can_Send_Data[7] = data4;
	
  HAL_CAN_AddTxMessage(hcan, &Tx_Message, Can_Send_Data, &send_mail_box);
}


	
