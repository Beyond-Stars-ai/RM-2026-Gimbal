#include "Gimbal_CtoC.h"
extern const RC_ctrl_t *local_rc_ctrl;

void Gimbal_CtoC_Remote(void)
{
	CToC_MasterSendData(	local_rc_ctrl->rc.ch[0],local_rc_ctrl->rc.ch[1],
												local_rc_ctrl->rc.ch[2],local_rc_ctrl->rc.ch[3],&hcan2);
}

