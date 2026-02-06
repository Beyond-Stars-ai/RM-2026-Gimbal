#include "Gimbal_Yaw_Big.h"

PID_PositionInitTypedef BigYaw_PositionPID;
PID_PositionInitTypedef BigYaw_SpeedPID;
extern motor_measure_t Can1_Rx_Data[7];
extern motor_measure_t Can2_Rx_Data[7];
extern RC_ctrl_t *local_rc_ctrl;
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;



void Gimbal_YawBig_Init(void)
{
	PID_PositionStructureInit (&BigYaw_PositionPID,0);        //外环位置环
  PID_PositionSetParameter  (&BigYaw_PositionPID,0.4,0,8);
  PID_PositionSetOUTRange   (&BigYaw_PositionPID,-20000,20000);
  // PID_PositionSetNeedValueRange(&BigYaw_PositionPID,4848,0);

	PID_PositionStructureInit (&BigYaw_SpeedPID,0);              //内环速度环
  PID_PositionSetParameter  (&BigYaw_SpeedPID,100,0,0);
  PID_PositionSetOUTRange   (&BigYaw_SpeedPID,-15000,15000);
}

void Gimbal_YawBig_Control(void)
{
    // ============速度环pid调节代码============
//  static uint32_t tick = 0;
//   static int i = 0;
//   float target_speed = 0.0f;

//   // 每 1000ms 切换一次状态（1秒）
//   if (HAL_GetTick() - tick > 1000) {
//       tick = HAL_GetTick();
//       i++;
//   }

//   // i=0: 0 RPM, i=1: +100, i=2: 0, i=3: -100, 然后循环
//   switch (i % 4) {
//       case 0: target_speed = 0.0f;    break;   // 停
//       case 1: target_speed = 60.0f;  break;   // 正转
//       case 2: target_speed = 0.0f;    break;   // 停
//       case 3: target_speed = 60.0f; break;   // 反转
//   }
//  
//  
//   PID_PositionSetNeedValue(&BigYaw_SpeedPID, target_speed);
//   PID_PositionCalc(&BigYaw_SpeedPID, Can2_Rx_Data[4].speed_rpm);
//  
//    Motor_6020_Voltage1((int16_t)BigYaw_SpeedPID.OUT, 0, 0, 0, &hcan2);
    // ========================

	
	
	
	
	
	
    #define RC_DEADBAND 10
    float target_speed = 0.0f;
    int16_t ch0 = local_rc_ctrl->rc.ch[0];

    // ============ 1. 更新位置目标（仅打杆时）============
    if (ch0 > RC_DEADBAND || ch0 < -RC_DEADBAND) {
        BigYaw_PositionPID.Need_Value -= 0.015f * ch0;

			if (BigYaw_PositionPID.Need_Value > 8191.0f)
					BigYaw_PositionPID.Need_Value -= 8192.0f;
			else if (BigYaw_PositionPID.Need_Value < 0.0f)
					BigYaw_PositionPID.Need_Value += 8192.0f;
    }
//		BigYaw_PositionPID.Need_Value = 2048.0f;
    // ===================================================

    // ============ 2. 位置环计算 =========================
    PID_PositionCalc(&BigYaw_PositionPID, Can2_Rx_Data[4].ecd);
    // ===================================================

    // ============ 3 =====
//    if (ch0 > RC_DEADBAND || ch0 < -RC_DEADBAND) {
        target_speed = BigYaw_PositionPID.OUT;
//    }
    // ===================================================

    // ============ 4. 速度环计算 =========================
    BigYaw_SpeedPID.Need_Value = target_speed;
    PID_PositionCalc(&BigYaw_SpeedPID, Can2_Rx_Data[4].speed_rpm);
    // ===================================================

    // ============ 5. 发送输出 ===========================
    Motor_6020_Voltage1((int16_t)BigYaw_SpeedPID.OUT, 0, 0, 0, &hcan2);
	
}
