#include "Remote.h"

void Remote_Init(void)//遥控器初始化
{
  local_rc_ctrl = get_remote_control_point();
  remote_control_init();
}

