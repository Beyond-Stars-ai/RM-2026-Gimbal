#ifndef GIMBAL_YAW_BIG_H
#define GIMBAL_YAW_BIG_H
#include "PID.h"
#include "bsp_can.h"
#include "Remote.h"
#include "Motor.h"
#include "BMI088.h"

void Gimbal_YawBig_Init(float initial_position);
void Gimbal_YawBig_Control(void);
#endif // __GIMBAL_YAW_SMALL_H
