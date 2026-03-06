#include "Gimbal_Yaw_Big.h"
#include "task.h"

PID_PositionInitTypedef BigYaw_PositionPID;
PID_PositionInitTypedef BigYaw_SpeedPID;
extern BMI088_Init_typedef BMI088_Data;					
extern BMI088_Init_typedef Can_BMI088_Data;			
extern BMI088_Init_typedef BigYaw_BMI088_Data;		
extern BMI088_Init_typedef SmallYaw_BMI088_Data;	
extern M6020_Motor Can1_M6020_MotorStatus[7];//GM6020电机状态数组
extern M6020_Motor Can2_M6020_MotorStatus[7];//GM6020电机状态数组
extern RC_ctrl_t *local_rc_ctrl;
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;



void Gimbal_YawBig_Init(void)
{
	PID_PositionStructureInit (&BigYaw_PositionPID,2432);        //外环小yaw电机位置环
  PID_PositionSetParameter  (&BigYaw_PositionPID,0.5,0,0);
	PID_PositionSetEkRange		(&BigYaw_PositionPID,-50,50);		//位置式PID设置误差为0阈值
  PID_PositionSetOUTRange   (&BigYaw_PositionPID,-20000,20000);
  // PID_PositionSetNeedValueRange(&BigYaw_PositionPID,4848,0);

	PID_PositionStructureInit (&BigYaw_SpeedPID,0);              //内环速度环
  PID_PositionSetParameter  (&BigYaw_SpeedPID,45,0,0);
	PID_PositionSetEkRange		(&BigYaw_PositionPID,-2,2);
  PID_PositionSetOUTRange   (&BigYaw_SpeedPID,-15000,15000);
	
	//====新加的
	// 开机防摔：等待CAN数据更新后，将目标设为当前实际编码器位置
	// 注意：需要延时一小段时间确保CAN数据已接收
	// HAL_Delay(100);  // 等待100ms确保电机数据已更新
  PROCESSOR(100);
	PID_PositionSetNeedValue(&BigYaw_PositionPID, Can2_M6020_MotorStatus[0].Angle);
	//====
}

void Gimbal_YawBig_Control(void)
{
//	
//		(Can2_M6020_MotorStatus[0].ANgle - 104) = BigYaw_BMI088_Data.Yaw
		
    // ============ 1. 更新位置目标============
    // ============ 2. 位置环计算 =========================
		PID_PositionCalc_Encoder(&BigYaw_PositionPID, Can2_M6020_MotorStatus[1].Angle);
    // ===================================================

    // ============ 3. 速度环计算 =========================
    PID_PositionSetNeedValue(&BigYaw_SpeedPID, BigYaw_PositionPID.OUT + 20);
    PID_PositionCalc				(&BigYaw_SpeedPID, Can2_M6020_MotorStatus[0].Speed);
    // ===================================================

    // ============ 4. 发送输出 ===========================
//    Motor_6020_Voltage1((int16_t)BigYaw_SpeedPID.OUT, 0, 0, 0, &hcan2);
	
}
