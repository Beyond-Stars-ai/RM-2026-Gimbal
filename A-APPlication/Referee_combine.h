#ifndef REFEREE_COMBINE_H
#define REFEREE_COMBINE_H

#include "main.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

/* ==================== 硬件配置 ==================== */

// 裁判系统UART配置
#define REFEREE_UART_BAUDRATE   115200
#define REFEREE_UART            USART6  // 根据实际硬件修改
#define REFEREE_RX_BUF_SIZE     256
#define REFEREE_TX_BUF_SIZE     128

// 机器人ID定义（高校联盟赛简化版）
#define ROBOT_ID_RED_SENTRY     7
#define ROBOT_ID_BLUE_SENTRY    107
#define SERVER_ID               0x8080

// 颜色定义
#define TEAM_RED                0
#define TEAM_BLUE               1

// 外部声明：需要在其他地方定义实际队伍颜色
extern uint8_t g_team_color;

/* ==================== 帧头结构 ==================== */

typedef __packed struct {
    uint8_t sof;            // 起始字节 0xA5
    uint16_t data_length;   // 数据长度
    uint8_t seq;            // 包序号
    uint8_t crc8;           // 帧头CRC8校验
} frame_header_t;

/* ==================== 硬件层接口函数 ==================== */

// UART初始化
void Referee_UART_Init(void);

// DMA初始化
void Referee_DMA_Init(void);

// 获取下一个包序号
uint8_t Referee_Get_Next_Seq(void);

// 计算CRC8
uint8_t Referee_Calc_CRC8(uint8_t *data, uint32_t len);

// 计算CRC16
uint16_t Referee_Calc_CRC16(uint8_t *data, uint32_t len);

// 验证CRC8
bool Referee_Verify_CRC8(uint8_t *data, uint32_t len, uint8_t crc);

// 验证CRC16
bool Referee_Verify_CRC16(uint8_t *data, uint32_t len, uint16_t crc);

// UART发送数据（底层）
void Referee_UART_Send(uint8_t *data, uint16_t len);

// 获取当前机器人ID
uint16_t Referee_Get_Robot_ID(void);

#endif // REFEREE_COMBINE_H
