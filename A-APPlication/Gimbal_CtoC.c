#include "Gimbal_CtoC.h"
extern const RC_ctrl_t *local_rc_ctrl;
extern BMI088_Init_typedef Can_BMI088_Data;
extern BMI088_Init_typedef BigYaw_BMI088_Data;
extern BMI088_Init_typedef SmallYaw_BMI088_Data;

void Gimbal_CtoC_Remote(void)
{
	CToC_MasterSendData(local_rc_ctrl->rc.ch[0], local_rc_ctrl->rc.ch[1],
						local_rc_ctrl->rc.ch[2], local_rc_ctrl->rc.ch[3],
						local_rc_ctrl->rc.s[0],  local_rc_ctrl->rc.s[1],
						&hcan2);
}

void CToC_AngleProcess(uint32_t ID, uint8_t *Data, BMI088_Init_typedef *data)
{
	int16_t yaw = (int16_t)((Data[0] << 8) | Data[1]);
	int16_t pitch = (int16_t)((Data[2] << 8) | Data[3]);
	int16_t roll = (int16_t)((Data[4] << 8) | Data[5]);
	int16_t tmp = (int16_t)((Data[6] << 8) | Data[7]);

	data->Yaw = yaw / 100.0f; // 转回 rad/s
	data->Pitch = pitch / 100.0f;
	data->Roll = roll / 100.0f;
	data->Temp = tmp / 10.0f; // 转回 °C
}

// void CToC_GyroProcess(uint32_t ID,uint8_t *Data)
//{
//	int16_t gx = (int16_t)((Data[0] << 8) | Data[1]);
//	int16_t gy = (int16_t)((Data[2] << 8) | Data[3]);
//	int16_t gz = (int16_t)((Data[4] << 8) | Data[5]);
//	int16_t tmp = (int16_t)((Data[6] << 8) | Data[7]);
//
//	g_bmi088_rx_gyro[0] = gx / 100.0f;      // 转回 rad/s
//	g_bmi088_rx_gyro[1] = gy / 100.0f;
//	g_bmi088_rx_gyro[2] = gz / 100.0f;
//	g_bmi088_rx_temp = tmp / 10.0f;         // 转回 °C
// }

// void CToC_AccelProcess(uint32_t ID,uint8_t *Data)
//{
//	int16_t ax = (int16_t)((Data[0] << 8) | Data[1]);
//	int16_t ay = (int16_t)((Data[2] << 8) | Data[3]);
//	int16_t az = (int16_t)((Data[4] << 8) | Data[5]);
//	int16_t tmp = (int16_t)((Data[6] << 8) | Data[7]);
//
//	g_bmi088_rx_accel[0] = ax / 100.0f;     // 转回 m/s²
//	g_bmi088_rx_accel[1] = ay / 100.0f;
//	g_bmi088_rx_accel[2] = az / 100.0f;
//	g_bmi088_rx_temp = tmp / 10.0f;
// }