#ifndef __GIMBAL_YAW_SMALL_H
#define __GIMBAL_YAW_SMALL_H

#include "PID.h"
#include "Motor.h"
#include "Remote.h"
#include "BSP_CAN.h"

void Gimbal_Yaw_Small_Init(void);
void Gimbal_Yaw_Small_Control(void);
#endif // __GIMBAL_YAW_SMALL_H
