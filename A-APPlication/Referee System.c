#include "Referee System.h"

/* ==================== 全局变量 ==================== */

// 全局哨兵状态
sentry_state_t g_sentry_state = {0};

// 当前行为模式
sentry_mode_t g_current_mode = SENTRY_MODE_IDLE;

// 兑换发弹量记录（用于单调递增检查）
static uint16_t last_exchanged_ammo = 0;

// 远程兑换请求次数（用于单调递增）
static uint8_t remote_ammo_request_cnt = 0;
static uint8_t remote_hp_request_cnt = 0;

// 发送缓冲区
static uint8_t tx_buffer[128];

/* ==================== 初始化函数 ==================== */

/**
 * @brief 裁判系统初始化
 */
void Referee_System_Init(void)
{
    // 清零状态结构体
    memset(&g_sentry_state, 0, sizeof(g_sentry_state));
    
    // 初始化硬件层
    Referee_UART_Init();
    Referee_DMA_Init();
    
    // 初始化变量
    last_exchanged_ammo = 0;
    remote_ammo_request_cnt = 0;
    remote_hp_request_cnt = 0;
    g_current_mode = SENTRY_MODE_DEFENSE;  // 默认防御模式
}

/* ==================== 帧头解析 ==================== */

/**
 * @brief 解析帧头
 * @param data 数据指针
 * @param header 帧头结构体指针
 * @return true: 解析成功, false: 解析失败
 */
bool Referee_Parse_Frame_Header(uint8_t *data, frame_header_t *header)
{
    header->sof = data[0];
    if (header->sof != 0xA5) {
        return false;
    }
    
    header->data_length = data[1] | (data[2] << 8);
    header->seq = data[3];
    header->crc8 = data[4];
    
    // 验证CRC8
    return Referee_Verify_CRC8(data, 5, header->crc8);
}

/* ==================== 数据解析函数 ==================== */

/**
 * @brief 主数据解析函数
 * @param data 数据指针
 * @param len 数据长度
 */
void Referee_Data_Process(uint8_t *data, uint16_t len)
{
    frame_header_t header;
    uint16_t cmd_id;
    uint8_t *payload;
    uint16_t frame_len;
    
    // 检查最小长度
    if (len < 7) return;
    
    // 解析帧头
    if (!Referee_Parse_Frame_Header(data, &header)) return;
    
    // 计算帧总长度：帧头(5) + 命令码(2) + 数据 + CRC16(2)
    frame_len = 5 + 2 + header.data_length + 2;
    if (len < frame_len) return;
    
    // 验证整包CRC16
    uint16_t crc16_recv = data[frame_len - 2] | (data[frame_len - 1] << 8);
    if (!Referee_Verify_CRC16(data, frame_len - 2, crc16_recv)) return;
    
    // 获取命令码
    cmd_id = data[5] | (data[6] << 8);
    payload = &data[7];
    
    // 根据命令码分发处理
    switch (cmd_id) {
        case CMD_GAME_HP:
            Referee_Parse_Game_HP(payload);
            break;
        case CMD_PROJECTILE_ALLOWANCE:
            Referee_Parse_Projectile_Allowance(payload);
            break;
        case CMD_ROBOT_POSITION:
            Referee_Parse_Robot_Position(payload);
            break;
        case CMD_SENTRY_INFO:
            Referee_Parse_Sentry_Info(payload);
            break;
        default:
            break;
    }
}

/**
 * @brief 解析血量数据 (0x0003)
 * @param data 数据指针
 */
void Referee_Parse_Game_HP(uint8_t *data)
{
    game_hp_t *hp = (game_hp_t *)data;
    
    g_sentry_state.hero_hp = hp->hero_hp;
    g_sentry_state.engineer_hp = hp->engineer_hp;
    g_sentry_state.infantry3_hp = hp->infantry3_hp;
    g_sentry_state.infantry4_hp = hp->infantry4_hp;
    g_sentry_state.infantry5_hp = hp->infantry5_hp;
    g_sentry_state.sentry_hp = hp->sentry_hp;
    
    g_sentry_state.hp_updated = true;
}

/**
 * @brief 解析允许发弹量数据 (0x0208)
 * @param data 数据指针
 */
void Referee_Parse_Projectile_Allowance(uint8_t *data)
{
    projectile_allowance_t *allowance = (projectile_allowance_t *)data;
    
    g_sentry_state.ammo_17mm = allowance->projectile_allowance_17mm;
    g_sentry_state.ammo_42mm = allowance->projectile_allowance_42mm;
    g_sentry_state.gold_coin = allowance->remaining_gold_coin;
    
    g_sentry_state.allowance_updated = true;
}

/**
 * @brief 解析地面机器人位置数据 (0x020B)
 * @param data 数据指针
 */
void Referee_Parse_Robot_Position(uint8_t *data)
{
    ground_robot_position_t *pos = (ground_robot_position_t *)data;
    
    g_sentry_state.hero_pos[0] = pos->hero_x;
    g_sentry_state.hero_pos[1] = pos->hero_y;
    g_sentry_state.engineer_pos[0] = pos->engineer_x;
    g_sentry_state.engineer_pos[1] = pos->engineer_y;
    g_sentry_state.infantry3_pos[0] = pos->infantry3_x;
    g_sentry_state.infantry3_pos[1] = pos->infantry3_y;
    g_sentry_state.infantry4_pos[0] = pos->infantry4_x;
    g_sentry_state.infantry4_pos[1] = pos->infantry4_y;
    g_sentry_state.infantry5_pos[0] = pos->infantry5_x;
    g_sentry_state.infantry5_pos[1] = pos->infantry5_y;
    
    g_sentry_state.position_updated = true;
}

/**
 * @brief 解析哨兵自主决策信息 (0x020D)
 * @param data 数据指针
 */
void Referee_Parse_Sentry_Info(uint8_t *data)
{
    uint32_t sentry_info;
    uint16_t sentry_info_2;
    
    memcpy(&sentry_info, &data[0], 4);
    memcpy(&sentry_info_2, &data[4], 2);
    
    g_sentry_state.sentry_info = sentry_info;
    g_sentry_state.sentry_info_2 = sentry_info_2;
    
    // 解析 sentry_info 字段
    g_sentry_state.exchanged_ammo = sentry_info & 0x7FF;              // bit 0-10
    g_sentry_state.remote_ammo_cnt = (sentry_info >> 11) & 0x0F;      // bit 11-14
    g_sentry_state.remote_hp_cnt = (sentry_info >> 15) & 0x0F;        // bit 15-18
    g_sentry_state.can_free_respawn = (sentry_info >> 19) & 0x01;     // bit 19
    g_sentry_state.can_pay_respawn = (sentry_info >> 20) & 0x01;      // bit 20
    g_sentry_state.respawn_cost = (sentry_info >> 21) & 0x3FF;        // bit 21-30
    
    // 解析 sentry_info_2 字段
    g_sentry_state.is_out_of_combat = sentry_info_2 & 0x01;           // bit 0
    g_sentry_state.team_ammo_remain = (sentry_info_2 >> 1) & 0x7FF;   // bit 1-11
    g_sentry_state.posture = (sentry_info_2 >> 12) & 0x03;            // bit 12-13
    g_sentry_state.rune_activatable = (sentry_info_2 >> 14) & 0x01;   // bit 14
    
    g_sentry_state.info_updated = true;
}

/* ==================== 指令发送函数 ==================== */

/**
 * @brief 发送哨兵决策指令 (0x0301, 0x0120)
 * @param cmd_value 指令值
 */
void Referee_Send_Sentry_Cmd(uint32_t cmd_value)
{
    uint16_t idx = 0;
    
    // 帧头
    tx_buffer[idx++] = 0xA5;
    tx_buffer[idx++] = 10;  // data_length低字节 (6+4=10)
    tx_buffer[idx++] = 0;   // data_length高字节
    tx_buffer[idx++] = Referee_Get_Next_Seq();
    tx_buffer[idx++] = 0;   // CRC8占位
    
    // 命令码 0x0301
    tx_buffer[idx++] = 0x01;
    tx_buffer[idx++] = 0x03;
    
    // 数据段头 (子内容ID 0x0120)
    tx_buffer[idx++] = 0x20;
    tx_buffer[idx++] = 0x01;
    
    // 发送者ID (哨兵ID: 7或107)
    uint16_t sender_id = Referee_Get_Robot_ID();
    tx_buffer[idx++] = sender_id & 0xFF;
    tx_buffer[idx++] = (sender_id >> 8) & 0xFF;
    
    // 接收者ID (服务器: 0x8080)
    tx_buffer[idx++] = 0x80;
    tx_buffer[idx++] = 0x80;
    
    // 哨兵指令数据 (4字节)
    memcpy(&tx_buffer[idx], &cmd_value, 4);
    idx += 4;
    
    // 计算CRC8
    tx_buffer[4] = Referee_Calc_CRC8(tx_buffer, 5);
    
    // 计算CRC16
    uint16_t crc16 = Referee_Calc_CRC16(tx_buffer, idx);
    tx_buffer[idx++] = crc16 & 0xFF;
    tx_buffer[idx++] = (crc16 >> 8) & 0xFF;
    
    // 发送
    Referee_UART_Send(tx_buffer, idx);
}

/**
 * @brief 发送哨兵路径数据 (0x0307)
 * @param intention 意图: 1=攻击, 2=防守, 3=移动
 * @param path 路径点数组
 * @param point_num 路径点数量（最多50个）
 */
void Referee_Send_Sentry_Path(uint8_t intention, path_point_t *path, uint8_t point_num)
{
    uint16_t idx = 0;
    
    if (point_num > 50) point_num = 50;
    if (point_num == 0) return;
    
    // 帧头
    tx_buffer[idx++] = 0xA5;
    tx_buffer[idx++] = 103; // data_length = 103
    tx_buffer[idx++] = 0;
    tx_buffer[idx++] = Referee_Get_Next_Seq();
    tx_buffer[idx++] = 0;   // CRC8占位
    
    // 命令码 0x0307
    tx_buffer[idx++] = 0x07;
    tx_buffer[idx++] = 0x03;
    
    // 意图
    tx_buffer[idx++] = intention;
    
    // 起点坐标 (单位: dm，即0.1m)
    uint16_t start_x = (uint16_t)(path[0].x * 10.0f);
    uint16_t start_y = (uint16_t)(path[0].y * 10.0f);
    tx_buffer[idx++] = start_x & 0xFF;
    tx_buffer[idx++] = (start_x >> 8) & 0xFF;
    tx_buffer[idx++] = start_y & 0xFF;
    tx_buffer[idx++] = (start_y >> 8) & 0xFF;
    
    // X轴增量数组 (49个，每个1字节，单位dm)
    for (int i = 1; i < 50 && i < point_num; i++) {
        int16_t delta_x = (int16_t)((path[i].x - path[i-1].x) * 10.0f);
        // 限制在-128~+127范围内
        if (delta_x > 127) delta_x = 127;
        if (delta_x < -128) delta_x = -128;
        tx_buffer[idx++] = (int8_t)delta_x;
    }
    // 填充剩余X增量
    for (int i = point_num; i < 50; i++) {
        tx_buffer[idx++] = 0;
    }
    
    // Y轴增量数组 (49个)
    for (int i = 1; i < 50 && i < point_num; i++) {
        int16_t delta_y = (int16_t)((path[i].y - path[i-1].y) * 10.0f);
        if (delta_y > 127) delta_y = 127;
        if (delta_y < -128) delta_y = -128;
        tx_buffer[idx++] = (int8_t)delta_y;
    }
    // 填充剩余Y增量
    for (int i = point_num; i < 50; i++) {
        tx_buffer[idx++] = 0;
    }
    
    // 发送者ID
    uint16_t sender_id = Referee_Get_Robot_ID();
    tx_buffer[idx++] = sender_id & 0xFF;
    tx_buffer[idx++] = (sender_id >> 8) & 0xFF;
    
    // 计算CRC
    tx_buffer[4] = Referee_Calc_CRC8(tx_buffer, 5);
    uint16_t crc16 = Referee_Calc_CRC16(tx_buffer, idx);
    tx_buffer[idx++] = crc16 & 0xFF;
    tx_buffer[idx++] = (crc16 >> 8) & 0xFF;
    
    Referee_UART_Send(tx_buffer, idx);
}

/* ==================== 便捷指令函数 ==================== */

/**
 * @brief 切换姿态
 * @param posture 1=进攻, 2=防御, 3=移动
 */
void Sentry_Set_Posture(uint8_t posture)
{
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.posture_cmd = posture;
    Referee_Send_Sentry_Cmd(cmd.value);
}

/**
 * @brief 确认复活（免费）
 */
void Sentry_Confirm_Respawn(void)
{
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.confirm_respawn = 1;
    Referee_Send_Sentry_Cmd(cmd.value);
}

/**
 * @brief 确认付费复活
 */
void Sentry_Confirm_Pay_Respawn(void)
{
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.confirm_pay_respawn = 1;
    Referee_Send_Sentry_Cmd(cmd.value);
}

/**
 * @brief 兑换发弹量（单调递增）
 * @param target_ammo 目标发弹量值
 */
void Sentry_Exchange_Ammo(uint16_t target_ammo)
{
    // 确保单调递增
    if (target_ammo <= last_exchanged_ammo) return;
    
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.exchange_ammo = target_ammo;
    last_exchanged_ammo = target_ammo;
    Referee_Send_Sentry_Cmd(cmd.value);
}

/**
 * @brief 请求远程兑换发弹量（单调递增，每次+1）
 */
void Sentry_Request_Remote_Ammo(void)
{
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.remote_ammo_req = ++remote_ammo_request_cnt;
    Referee_Send_Sentry_Cmd(cmd.value);
}

/**
 * @brief 请求远程兑换血量（单调递增，每次+1）
 */
void Sentry_Request_Remote_HP(void)
{
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.remote_hp_req = ++remote_hp_request_cnt;
    Referee_Send_Sentry_Cmd(cmd.value);
}

/**
 * @brief 确认激活能量机关
 */
void Sentry_Confirm_Rune(void)
{
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.confirm_rune = 1;
    Referee_Send_Sentry_Cmd(cmd.value);
}

/* ==================== 战术策略函数 ==================== */

/**
 * @brief 设置行为模式
 * @param mode 模式
 */
void Sentry_Set_Mode(sentry_mode_t mode)
{
    g_current_mode = mode;
}

/**
 * @brief 获取当前行为模式
 * @return 当前模式
 */
sentry_mode_t Sentry_Get_Mode(void)
{
    return g_current_mode;
}

/**
 * @brief 计算两点距离
 * @param x1 点1 X坐标
 * @param y1 点1 Y坐标
 * @param x2 点2 X坐标
 * @param y2 点2 Y坐标
 * @return 距离
 */
float Sentry_Calc_Distance(float x1, float y1, float x2, float y2)
{
    return sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

/**
 * @brief 处理复活逻辑
 */
void Sentry_Handle_Respawn(void)
{
    // 检查是否可以免费复活
    if (g_sentry_state.can_free_respawn) {
        Sentry_Confirm_Respawn();
        return;
    }
    
    // 检查是否可以付费复活
    if (g_sentry_state.can_pay_respawn) {
        if (g_sentry_state.gold_coin >= g_sentry_state.respawn_cost) {
            Sentry_Confirm_Pay_Respawn();
        }
    }
}

/**
 * @brief 处理资源管理
 */
void Sentry_Handle_Resource(void)
{
    // 如果发弹量不足，请求远程兑换
    if (g_sentry_state.ammo_17mm < 100 && g_sentry_state.gold_coin > 0) {
        Sentry_Request_Remote_Ammo();
    }
}

/**
 * @brief 进攻策略：主动寻找敌方进攻
 */
void Sentry_Strategy_Attack(void)
{
    // 切换为进攻姿态
    Sentry_Set_Posture(POSTURE_ATTACK);
    
    // TODO: 根据敌方位置规划进攻路径
    // 这里需要配合雷达或其他感知系统
    
    // 示例：向敌方基地方向移动
    path_point_t attack_path[3] = {
        {g_sentry_state.hero_pos[0], g_sentry_state.hero_pos[1]},  // 起点
        {g_sentry_state.hero_pos[0] + 2.0f, g_sentry_state.hero_pos[1]},  // 中间点
        {g_sentry_state.hero_pos[0] + 4.0f, g_sentry_state.hero_pos[1]}   // 终点
    };
    
    // 发送路径（高校联盟赛可能没有客户端显示，但仍可发送）
    Referee_Send_Sentry_Path(1, attack_path, 3);  // 1=攻击
}

/**
 * @brief 防守策略：保护己方要点
 */
void Sentry_Strategy_Defense(void)
{
    // 切换为防御姿态
    Sentry_Set_Posture(POSTURE_DEFENSE);
    
    // TODO: 在关键位置巡逻
    // 示例：在基地附近巡逻
    path_point_t patrol_path[4] = {
        {1.0f, 1.0f},
        {1.0f, 3.0f},
        {3.0f, 3.0f},
        {3.0f, 1.0f}
    };
    
    Referee_Send_Sentry_Path(2, patrol_path, 4);  // 2=防守
}

/**
 * @brief 跟随策略：跟随己方单位
 * @param target_type 1=英雄, 2=工程, 3=3号步兵, 4=4号步兵, 5=5号步兵
 */
void Sentry_Strategy_Follow(uint8_t target_type)
{
    float *target_pos = NULL;
    
    switch (target_type) {
        case 1: target_pos = g_sentry_state.hero_pos; break;
        case 2: target_pos = g_sentry_state.engineer_pos; break;
        case 3: target_pos = g_sentry_state.infantry3_pos; break;
        case 4: target_pos = g_sentry_state.infantry4_pos; break;
        case 5: target_pos = g_sentry_state.infantry5_pos; break;
        default: return;
    }
    
    // 切换为移动姿态
    Sentry_Set_Posture(POSTURE_MOVE);
    
    // 计算跟随路径（保持距离）
    path_point_t follow_path[2] = {
        {g_sentry_state.hero_pos[0], g_sentry_state.hero_pos[1]},  // 当前位置
        {target_pos[0] - 1.0f, target_pos[1]}  // 目标后方1米
    };
    
    Referee_Send_Sentry_Path(3, follow_path, 2);  // 3=移动
}

/**
 * @brief 巡逻策略
 */
void Sentry_Strategy_Patrol(void)
{
    // 切换为移动姿态
    Sentry_Set_Posture(POSTURE_MOVE);
    
    // TODO: 实现巡逻路径
}

/**
 * @brief 撤退策略
 */
void Sentry_Strategy_Retreat(void)
{
    // 切换为移动姿态
    Sentry_Set_Posture(POSTURE_MOVE);
    
    // TODO: 实现撤退路径（向己方基地）
}

/**
 * @brief 主决策循环
 * @note 应在主循环中定期调用
 */
void Sentry_Decision_Loop(void)
{
    // 检查血量状态
    if (g_sentry_state.sentry_hp == 0) {
        Sentry_Handle_Respawn();
        return;
    }
    
    // 资源管理
    Sentry_Handle_Resource();
    
    // 根据当前模式执行对应策略
    switch (g_current_mode) {
        case SENTRY_MODE_ATTACK:
            Sentry_Strategy_Attack();
            break;
        case SENTRY_MODE_DEFENSE:
            Sentry_Strategy_Defense();
            break;
        case SENTRY_MODE_FOLLOW:
            Sentry_Strategy_Follow(1);  // 默认跟随英雄
            break;
        case SENTRY_MODE_PATROL:
            Sentry_Strategy_Patrol();
            break;
        case SENTRY_MODE_RETREAT:
            Sentry_Strategy_Retreat();
            break;
        case SENTRY_MODE_IDLE:
        default:
            // 待机状态，保持当前姿态
            break;
    }
}
