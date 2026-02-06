#ifndef CTOC_H
#define CTOC_H
#include <stdint.h>
#include "can.h"


void CToC_MasterSendData(	int16_t data1, int16_t data2, 
													int16_t data3, int16_t data4, 
													CAN_HandleTypeDef *hcan);

#endif //CTOC_H
