#ifndef GIMBAL_YAW_BIG_H
#define GIMBAL_YAW_BIG_H
#include "PID.h"
#include "bsp_can.h"
#include "Remote.h"
#include "Motor.h"

void Gimbal_YawBig_Init(void);
void Gimbal_YawBig_Control(void);
#endif // __GIMBAL_YAW_SMALL_H
