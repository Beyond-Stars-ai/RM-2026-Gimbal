#include "Gimbal_Yaw_Small.h"
#define SMALLYAW_MID 2424          //小yaw轴中位值
#define SMALLYAW_LEFT 1650          //小yaw轴左侧最大偏移
#define SMALLYAW_RIGHT 1650          //小yaw轴右侧最大偏移（顺时针减小）

PID_PositionInitTypedef SmallYaw_PositionPID;
PID_PositionInitTypedef SmallYaw_SpeedPID;
extern motor_measure_t Can2_Rx_Data[7];
extern RC_ctrl_t *local_rc_ctrl;



void Gimbal_Yaw_Small_Init(void)
{
	PID_PositionStructureInit (&SmallYaw_PositionPID,2424);        //外环位置环
  PID_PositionSetParameter  (&SmallYaw_PositionPID,1.5,0,0);
  PID_PositionSetOUTRange   (&SmallYaw_PositionPID,-400,400);
  // PID_PositionSetNeedValueRange(&SmallYaw_PositionPID,4848,0);

	PID_PositionStructureInit (&SmallYaw_SpeedPID,0);              //内环速度环
  PID_PositionSetParameter  (&SmallYaw_SpeedPID,100,0,0);
  PID_PositionSetOUTRange   (&SmallYaw_SpeedPID,-20000,20000);
  PID_PositionSetEkRange    (&SmallYaw_SpeedPID, -3.0f, 3.0f);
}

void Gimbal_Yaw_Small_Control(void)
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
  //       case 1: target_speed = 100.0f;  break;   // 正转
  //       case 2: target_speed = 0.0f;    break;   // 停
  //       case 3: target_speed = 150.0f; break;   // 反转
  //   }
    
    
  //   PID_PositionSetNeedValue(&SmallYaw_SpeedPID, target_speed);
  //   PID_PositionCalc(&SmallYaw_SpeedPID, motor_gimbal[5].speed_rpm);
    
  //   CAN_cmd_gimbal(0, (int16_t)SmallYaw_SpeedPID.OUT, 0, 0);
	

    // ============ 更新位置目标（仅打杆时）============
    SmallYaw_PositionPID.Need_Value -= 0.025f * local_rc_ctrl->rc.ch[2];
    // 限幅 [0, 4848]
    if (SmallYaw_PositionPID.Need_Value > 4848.0f)
        SmallYaw_PositionPID.Need_Value = 4848.0f;
    else if (SmallYaw_PositionPID.Need_Value < 0.0f)
        SmallYaw_PositionPID.Need_Value = 0.0f;
    // ============ 位置环计算 =========================
		if(Can2_Rx_Data[5].last_ecd - Can2_Rx_Data[5].ecd > 4096) 
    PID_PositionCalc				(&SmallYaw_PositionPID, Can2_Rx_Data[5].ecd);
    // ============ 速度环计算 =========================
    PID_PositionSetNeedValue(&SmallYaw_SpeedPID, SmallYaw_PositionPID.OUT);
    PID_PositionCalc				(&SmallYaw_SpeedPID, Can2_Rx_Data[5].speed_rpm);
    // ============ 发送输出 ===========================
    Motor_6020_Voltage1			(0, (int16_t)SmallYaw_SpeedPID.OUT, 0, 0, &hcan2);
}

