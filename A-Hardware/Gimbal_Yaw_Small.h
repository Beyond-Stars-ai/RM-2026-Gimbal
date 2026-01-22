#ifndef __GIMBAL_YAW_SMALL_H
#define __GIMBAL_YAW_SMALL_H

#include "PID.h"
#include "CAN_receive.h"
#include "remote_control.h"

void Gimbal_Yaw_Small_Init(void);
void Gimbal_Yaw_Small_Control(void);
#endif // __GIMBAL_YAW_SMALL_H
