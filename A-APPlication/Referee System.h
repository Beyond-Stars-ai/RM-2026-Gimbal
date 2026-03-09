#ifndef REFEREE_SYSTEM_H
#define REFEREE_SYSTEM_H

#include "main.h"
#include "Referee_combine.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

/* ==================== 命令码定义 ==================== */

// 接收的命令码（下行）
#define CMD_GAME_HP                 0x0003  // 血量数据 3Hz
#define CMD_PROJECTILE_ALLOWANCE    0x0208  // 允许发弹量 10Hz
#define CMD_ROBOT_POSITION          0x020B  // 地面机器人位置 1Hz
#define CMD_SENTRY_INFO             0x020D  // 哨兵自主决策信息 1Hz

// 发送的命令码（上行）
#define CMD_SENTRY_CMD              0x0301  // 哨兵自主决策指令 30Hz
#define CMD_MAP_DATA                0x0307  // 哨兵路径数据 1Hz

// 子内容ID
#define SUB_CMD_SENTRY              0x0120  // 哨兵指令子内容ID

/* ==================== 姿态定义 ==================== */

typedef enum {
    POSTURE_ATTACK = 1,     // 进攻姿态
    POSTURE_DEFENSE = 2,    // 防御姿态
    POSTURE_MOVE = 3,       // 移动姿态
} sentry_posture_t;

/* ==================== 行为模式定义 ==================== */

typedef enum {
    SENTRY_MODE_IDLE,       // 待机
    SENTRY_MODE_ATTACK,     // 进攻
    SENTRY_MODE_DEFENSE,    // 防守
    SENTRY_MODE_PATROL,     // 巡逻
    SENTRY_MODE_FOLLOW,     // 跟随
    SENTRY_MODE_RETREAT,    // 撤退
} sentry_mode_t;

/* ==================== 数据结构定义 ==================== */

// 帧头结构
typedef __packed struct {
    uint8_t sof;            // 起始字节 0xA5
    uint16_t data_length;   // 数据长度
    uint8_t seq;            // 包序号
    uint8_t crc8;           // 帧头CRC8校验
} frame_header_t;

// 完整数据帧结构（最大128字节数据）
typedef __packed struct {
    frame_header_t header;   // 帧头
    uint16_t cmd_id;         // 命令码
    uint8_t data[128];       // 数据段
    uint16_t crc16;          // 整包CRC16校验
} referee_frame_t;

// 血量数据结构 (0x0003)
typedef __packed struct {
    uint16_t hero_hp;           // 英雄血量
    uint16_t engineer_hp;       // 工程血量
    uint16_t infantry3_hp;      // 3号步兵血量
    uint16_t infantry4_hp;      // 4号步兵血量
    uint16_t infantry5_hp;      // 5号步兵血量
    uint16_t plane_hp;          // 飞机血量
    uint16_t sentry_hp;         // 哨兵血量
    uint16_t rader_hp;          // 雷达血量
} game_hp_t;

// 允许发弹量数据结构 (0x0208)
typedef __packed struct {
    uint16_t projectile_allowance_17mm;     // 17mm弹丸允许发弹量
    uint16_t projectile_allowance_42mm;     // 42mm弹丸允许发弹量
    uint16_t remaining_gold_coin;           // 剩余金币数量
    uint16_t projectile_allowance_fortress; // 堡垒增益点储备发弹量
} projectile_allowance_t;

// 地面机器人位置数据结构 (0x020B)
typedef __packed struct {
    float hero_x;           // 己方英雄X坐标(m)
    float hero_y;           // 己方英雄Y坐标(m)
    float engineer_x;       // 己方工程X坐标(m)
    float engineer_y;       // 己方工程Y坐标(m)
    float infantry3_x;      // 己方3号步兵X坐标(m)
    float infantry3_y;      // 己方3号步兵Y坐标(m)
    float infantry4_x;      // 己方4号步兵X坐标(m)
    float infantry4_y;      // 己方4号步兵Y坐标(m)
    float infantry5_x;      // 己方5号步兵X坐标(m)
    float infantry5_y;      // 己方5号步兵Y坐标(m)
} ground_robot_position_t;

// 哨兵自主决策信息结构 (0x020D)
typedef __packed struct {
    uint32_t sentry_info;   // 哨兵信息（4字节）
    uint16_t sentry_info_2; // 哨兵信息2（2字节）
} sentry_info_t;

// 哨兵指令数据结构 (0x0301, 0x0120)
typedef __packed struct {
    uint16_t data_cmd_id;   // 子内容ID 0x0120
    uint16_t sender_id;     // 发送者ID
    uint16_t receiver_id;   // 接收者ID
    uint32_t sentry_cmd;    // 哨兵指令数据
} sentry_cmd_frame_t;

// 决策指令联合体（用于位操作）
typedef union {
    uint32_t value;
    struct {
        uint32_t confirm_respawn : 1;       // bit 0: 确认复活
        uint32_t confirm_pay_respawn : 1;   // bit 1: 确认付费复活
        uint32_t exchange_ammo : 11;        // bit 2-12: 兑换发弹量值
        uint32_t remote_ammo_req : 4;       // bit 13-16: 远程兑换发弹量请求次数
        uint32_t remote_hp_req : 4;         // bit 17-20: 远程兑换血量请求次数
        uint32_t posture_cmd : 2;           // bit 21-22: 姿态指令
        uint32_t confirm_rune : 1;          // bit 23: 确认激活能量机关
        uint32_t reserved : 8;              // bit 24-31: 保留
    } bits;
} sentry_cmd_u;

// 路径点结构
typedef struct {
    float x;
    float y;
} path_point_t;

// 哨兵状态数据结构
typedef struct {
    // 位置信息（从裁判系统接收）
    float hero_pos[2];          // 英雄位置 [x, y]
    float engineer_pos[2];      // 工程位置 [x, y]
    float infantry3_pos[2];     // 3号步兵位置 [x, y]
    float infantry4_pos[2];     // 4号步兵位置 [x, y]
    float infantry5_pos[2];     // 5号步兵位置 [x, y]
    
    // 血量信息
    uint16_t hero_hp;
    uint16_t engineer_hp;
    uint16_t infantry3_hp;
    uint16_t infantry4_hp;
    uint16_t infantry5_hp;
    uint16_t sentry_hp;
    uint16_t sentry_max_hp;
    
    // 资源信息
    uint16_t ammo_17mm;         // 17mm允许发弹量
    uint16_t ammo_42mm;         // 42mm允许发弹量
    uint16_t gold_coin;         // 金币数量
    
    // 决策相关信息
    uint32_t sentry_info;       // 哨兵信息
    uint16_t sentry_info_2;     // 哨兵信息2
    
    // 姿态
    uint8_t posture;            // 1=进攻, 2=防御, 3=移动
    bool is_out_of_combat;      // 是否脱战
    bool can_free_respawn;      // 是否可免费复活
    bool can_pay_respawn;       // 是否可付费复活
    uint16_t respawn_cost;      // 复活所需金币
    
    // 兑换相关
    uint16_t exchanged_ammo;    // 已兑换发弹量（bit 0-10）
    uint8_t remote_ammo_cnt;    // 远程兑换发弹量次数（bit 11-14）
    uint8_t remote_hp_cnt;      // 远程兑换血量次数（bit 15-18）
    
    // 能量机关
    bool rune_activatable;      // 能量机关是否可激活
    uint16_t team_ammo_remain;  // 队伍17mm剩余可兑换数（bit 1-11）
    
    // 数据更新标志
    bool hp_updated;
    bool position_updated;
    bool info_updated;
    bool allowance_updated;
} sentry_state_t;

/* ==================== 外部变量 ==================== */

// 全局哨兵状态
extern sentry_state_t g_sentry_state;

// 当前行为模式
extern sentry_mode_t g_current_mode;

/* ==================== 软件层接口函数 ==================== */

// 初始化函数
void Referee_System_Init(void);

// 数据解析函数
void Referee_Data_Process(uint8_t *data, uint16_t len);
bool Referee_Parse_Frame_Header(uint8_t *data, frame_header_t *header);

// 具体数据解析
void Referee_Parse_Game_HP(uint8_t *data);
void Referee_Parse_Projectile_Allowance(uint8_t *data);
void Referee_Parse_Robot_Position(uint8_t *data);
void Referee_Parse_Sentry_Info(uint8_t *data);

// 指令发送函数
void Referee_Send_Sentry_Cmd(uint32_t cmd_value);
void Referee_Send_Sentry_Path(uint8_t intention, path_point_t *path, uint8_t point_num);

// 便捷指令函数
void Sentry_Set_Posture(uint8_t posture);
void Sentry_Confirm_Respawn(void);
void Sentry_Confirm_Pay_Respawn(void);
void Sentry_Exchange_Ammo(uint16_t target_ammo);
void Sentry_Request_Remote_Ammo(void);
void Sentry_Request_Remote_HP(void);
void Sentry_Confirm_Rune(void);

// 战术策略函数
void Sentry_Decision_Loop(void);
void Sentry_Strategy_Attack(void);
void Sentry_Strategy_Defense(void);
void Sentry_Strategy_Follow(uint8_t target_type);
void Sentry_Strategy_Patrol(void);
void Sentry_Strategy_Retreat(void);
void Sentry_Handle_Respawn(void);
void Sentry_Handle_Resource(void);

// 辅助函数
float Sentry_Calc_Distance(float x1, float y1, float x2, float y2);
void Sentry_Set_Mode(sentry_mode_t mode);
sentry_mode_t Sentry_Get_Mode(void);

#endif // REFEREE_SYSTEM_H
