#include "Gimbal_Yaw_Small.h"
#define SMALLYAW_MID 2424          //小yaw轴中位值
#define SMALLYAW_LEFT 1650          //小yaw轴左侧最大偏移
#define SMALLYAW_RIGHT 1650          //小yaw轴右侧最大偏移（顺时针减小）

PID_PositionInitTypedef SmallYaw_GyroscopePID;
PID_PositionInitTypedef SmallYaw_PositionPID;
PID_PositionInitTypedef SmallYaw_SpeedPID;
extern BMI088_Init_typedef BMI088_Data;
extern BMI088_Init_typedef Can_BMI088_Data;
extern BMI088_Init_typedef BigYaw_BMI088_Data;
extern BMI088_Init_typedef SmallYaw_BMI088_Data;
extern M6020_Motor Can1_M6020_MotorStatus[7];//GM6020电机状态数组
extern M6020_Motor Can2_M6020_MotorStatus[7];//GM6020电机状态数组
extern RC_ctrl_t *local_rc_ctrl;

float gyro_needvalue = 0;




void Gimbal_YawSmall_Init(void)
{
	PID_PositionStructureInit (&SmallYaw_GyroscopePID,0);        //外环小yaw角度环
  PID_PositionSetParameter  (&SmallYaw_GyroscopePID,40,0,0);
  PID_PositionSetOUTRange   (&SmallYaw_GyroscopePID,-20000,20000);

	PID_PositionStructureInit (&SmallYaw_SpeedPID,0);              //内环速度环
  PID_PositionSetParameter  (&SmallYaw_SpeedPID,100,0,0);
  PID_PositionSetOUTRange   (&SmallYaw_SpeedPID,-20000,20000);
  PID_PositionSetEkRange    (&SmallYaw_SpeedPID, -3.0f, 3.0f);
	
	//====新加的
	// 【零偏差启动】推翻预设位置(0°)，当前位置即目标位置
	gyro_needvalue = SmallYaw_BMI088_Data.Yaw;
	PID_PositionSetNeedValue(&SmallYaw_GyroscopePID, SmallYaw_BMI088_Data.Yaw);
	//====
}

void Gimbal_YawSmall_Control(void)
{
	// ============获取目标角度============
	gyro_needvalue  -= 0.0007 * local_rc_ctrl->rc.ch[2]; //-= 0.0007 * local_rc_ctrl->rc.ch[2];// = 0;
	if(gyro_needvalue > 180) gyro_needvalue -=360 ;
	else if(gyro_needvalue < -180) gyro_needvalue +=360 ;
	
	adjustAngle3(gyro_needvalue , SmallYaw_BMI088_Data.Yaw , &SmallYaw_GyroscopePID.Need_Value);
	
//	SmallYaw_GyroscopePID.Need_Value  -= 0.0007 * local_rc_ctrl->rc.ch[2]; //-= 0.0007 * local_rc_ctrl->rc.ch[2];// = 0;
//	if(SmallYaw_GyroscopePID.Need_Value > 180) SmallYaw_GyroscopePID.Need_Value -=360 ;
//	else if(SmallYaw_GyroscopePID.Need_Value < -180) SmallYaw_GyroscopePID.Need_Value +=360 ;
	// ============角度环计算============
	PID_PositionCalc_IMU(&SmallYaw_GyroscopePID, SmallYaw_BMI088_Data.Yaw);
	// ============ 速度环计算 =========================
	PID_PositionSetNeedValue(&SmallYaw_SpeedPID, SmallYaw_GyroscopePID.OUT);//SmallYaw_GyroscopePID.OUT
	PID_PositionCalc				(&SmallYaw_SpeedPID, Can2_M6020_MotorStatus[1].Speed);
	// if(Can2_M6020_MotorStatus[1].ANgle>-74 && Can2_M6020_MotorStatus[1].ANgle<8) SmallYaw_SpeedPID.OUT=1111;//两个愚蠢的办法解决超限位问题
	// if(Can2_M6020_MotorStatus[1].ANgle>-154 && Can2_M6020_MotorStatus[1].ANgle<-74) SmallYaw_SpeedPID.OUT=-1111;
    if(Can2_M6020_MotorStatus[1].ANgle>-74 && Can2_M6020_MotorStatus[1].ANgle<8) SmallYaw_SpeedPID.OUT=400;//两个愚蠢的办法解决超限位问题
	if(Can2_M6020_MotorStatus[1].ANgle>-154 && Can2_M6020_MotorStatus[1].ANgle<-74) SmallYaw_SpeedPID.OUT=-400;
	// ============ 发送输出 ===========================
//    Motor_6020_Voltage1			(0, (int16_t)SmallYaw_SpeedPID.OUT, 0, 0, &hcan2);
}

//简介：根据用户目标角度修正小yaw目标角度
//参数：angle1：用户目标角度
//			angle2：小yaw目标角度
static void adjustAngle3(float angle1, float angle2, float *angle3) {
    // 1. 计算angle1与angle2的归一化差值（-180~180°）
    float diff = angle1 - angle2;
    diff = fmod(diff, 360.0f);
    if (diff > 180.0f) {
        diff -= 360.0f;
    } else if (diff < -180.0f) {
        diff += 360.0f;
    }

    // 2. 判断angle1是否在angle2的±70°范围内，调整angle3
    if (fabs(diff) < 70.0f) {
        // 在范围内：angle3 = angle1
        *angle3 = angle1;
    } else {
        // 超出范围：angle3 = angle1靠近的angle2边界（angle2±70°）
        if (diff > 0) {
            // diff为正 → angle1在angle2右侧，靠近的边界是angle2 + 70°
            *angle3 = angle2 + 70.0f;
        } else {
            // diff为负 → angle1在angle2左侧，靠近的边界是angle2 - 70°
            *angle3 = angle2 - 70.0f;
        }

        // 3. 将调整后的angle3归一化到-180~180°（避免超出范围）
        *angle3 = fmod(*angle3, 360.0f);
        if (*angle3 > 180.0f) {
            *angle3 -= 360.0f;
        } else if (*angle3 < -180.0f) {
            *angle3 += 360.0f;
        }
    }
}
