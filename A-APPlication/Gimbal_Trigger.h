#ifndef GIMBAL_TRIGGER_H
#define GIMBAL_TRIGGER_H

#include "PID.h"
#include "Motor.h"
#include "Remote.h"
#include "BSP_CAN.h"

void Gimbal_Trigger_Init(void);
void Gimbal_Trigger_Control(void);

#endif //GIMBAL_TRIGGER_H
