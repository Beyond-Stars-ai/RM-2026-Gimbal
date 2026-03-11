# RoboMaster 2026 哨兵裁判系统通信协议分析报告

## 一、哨兵相关数据帧概览

### 1.1 哨兵接收的裁判系统数据（下行）

| 命令码 | 数据长度 | 发送频率 | 说明 |
|--------|----------|----------|------|
| 0x0003 | 16字节 | 3Hz | 己方全体机器人血量（含哨兵） |
| 0x0208 | 6字节 | 10Hz | 允许发弹量（含哨兵） |
| 0x020B | 40字节 | 1Hz | **地面机器人位置数据（专供哨兵）** |
| 0x020D | 6字节 | 1Hz | **哨兵自主决策信息同步** |
| 0x0301(0x0120) | 4字节 | 30Hz | **哨兵自主决策指令（发送给服务器）** |

### 1.2 哨兵发送的裁判系统数据（上行）

| 命令码 | 数据长度 | 发送频率 | 说明 |
|--------|----------|----------|------|
| 0x0307 | 103字节 | 1Hz | **哨兵路径数据（发送给选手端）** |
| 0x0308 | 34字节 | 3Hz | 自定义消息（可选） |

---

## 二、哨兵裁判系统数据详解（给人看的说明）

### 2.1 地面机器人位置数据 (0x020B) - 核心数据

**发送方**: 服务器 → 己方哨兵机器人  
**频率**: 1Hz  
**数据长度**: 40字节

**数据内容**:
```c
typedef __packed struct {
    float hero_x;        // 己方英雄X坐标(m)
    float hero_y;        // 己方英雄Y坐标(m)
    float engineer_x;    // 己方工程X坐标(m)
    float engineer_y;    // 己方工程Y坐标(m)
    float standard_3_x;  // 己方3号步兵X坐标(m)
    float standard_3_y;  // 己方3号步兵Y坐标(m)
    float standard_4_x;  // 己方4号步兵X坐标(m)
    float standard_4_y;  // 己方4号步兵Y坐标(m)
    float reserved;      // 保留位
    float reserved;      // 保留位
} ground_robot_position_t;
```

**用途说明**:
- 哨兵通过此数据获取己方所有地面机器人的实时位置
- 坐标系: 场地围挡在红方补给站附近的交点为原点，沿场地长边向蓝方为X轴正方向，沿场地短边向红方停机坪为Y轴正方向
- 哨兵可基于此数据进行战术决策（如跟随、防守、进攻等）

---

### 2.2 哨兵自主决策信息同步 (0x020D) - 关键状态数据

**发送方**: 服务器 → 己方哨兵机器人  
**频率**: 1Hz  
**数据长度**: 6字节

**数据内容**:
```c
typedef __packed struct {
    uint32_t sentry_info;   // 哨兵信息（4字节）
    uint16_t sentry_info_2; // 哨兵信息2（2字节）
} sentry_info_t;
```

**sentry_info 字段详解**:

| 位域 | 说明 |
|------|------|
| bit 0-10 | 哨兵成功兑换的允许发弹量（开局为0） |
| bit 11-14 | 哨兵成功远程兑换发弹量的次数（开局为0） |
| bit 15-18 | 哨兵成功远程兑换血量的次数（开局为0） |
| bit 19 | 是否可以确认免费复活（1=可以） |
| bit 20 | 是否可以兑换立即复活（1=可以） |
| bit 21-30 | 兑换立即复活所需金币数 |
| bit 31 | 保留位 |

**sentry_info_2 字段详解**:

| 位域 | 说明 |
|------|------|
| bit 0 | 哨兵是否处于脱战状态（1=脱战） |
| bit 1-11 | 队伍17mm允许发弹量的剩余可兑换数 |
| bit 12-13 | 哨兵当前姿态（1=进攻, 2=防御, 3=移动） |
| bit 14 | 己方能量机关是否可激活（1=可激活） |
| bit 15 | 保留位 |

**用途说明**:
- 哨兵通过此数据获取自身状态和可操作权限
- 判断是否可复活、可兑换发弹量/血量
- 获取当前姿态状态

---

### 2.3 哨兵自主决策指令 (0x0301 子内容ID 0x0120) - 哨兵主动控制

**发送方**: 哨兵机器人 → 服务器  
**频率**: 最高30Hz  
**数据长度**: 4字节

**数据内容**:
```c
typedef __packed struct {
    uint32_t sentry_cmd;  // 哨兵自主决策指令
} sentry_cmd_t;
```

**指令字段详解**:

| 位域 | 功能 | 说明 |
|------|------|------|
| bit 0 | 确认复活 | 1=确认复活（读条完成后立即复活） |
| bit 1 | 确认兑换立即复活 | 1=确认消耗金币立即复活 |
| bit 2-12 | 兑换发弹量值 | 哨兵将要兑换的发弹量值（需单调递增） |
| bit 13-16 | 远程兑换发弹量请求次数 | 需单调递增，每次+1 |
| bit 17-20 | 远程兑换血量请求次数 | 需单调递增，每次+1 |
| bit 21-22 | 修改姿态指令 | 1=进攻, 2=防御, 3=移动 |
| bit 23 | 确认激活能量机关 | 1=确认使能量机关进入正在激活状态 |
| bit 24-31 | 保留位 | - |

**重要规则**:
1. **兑换发弹量值必须单调递增**: 开局为0，只能增加不能减少
2. **远程兑换请求次数必须单调递增且每次+1**: 开局为0，每次请求+1
3. **指令处理顺序**: 服务器从低位到高位依次处理，直到全部成功或不能处理

---

### 2.4 哨兵路径数据 (0x0307) - 哨兵发送给操作手

**发送方**: 哨兵机器人 → 对应操作手选手端  
**频率**: 最高1Hz  
**数据长度**: 103字节

**数据内容**:
```c
typedef __packed struct {
    uint8_t intention;           // 意图: 1=攻击, 2=防守, 3=移动
    uint16_t start_position_x;   // 路径起点X(dm)
    uint16_t start_position_y;   // 路径起点Y(dm)
    int8_t delta_x[49];          // X轴增量数组(dm)
    int8_t delta_y[49];          // Y轴增量数组(dm)
    uint16_t sender_id;          // 发送者ID
} map_data_t;
```

**用途说明**:
- 哨兵将规划的路径发送给操作手，在小地图上显示
- 操作手可实时查看哨兵的移动意图和路径规划
- 共49个路径点，通过增量方式存储节省带宽

---

### 2.5 允许发弹量数据 (0x0208) - 资源管理

**发送方**: 服务器 → 己方英雄/步兵/哨兵/空中机器人  
**频率**: 10Hz  
**数据长度**: 6字节

**数据内容**:
```c
typedef __packed struct {
    uint16_t projectile_allowance_17mm;    // 17mm弹丸允许发弹量
    uint16_t projectile_allowance_42mm;    // 42mm弹丸允许发弹量
    uint16_t remaining_gold_coin;          // 剩余金币数量
    uint16_t projectile_allowance_fortress; // 堡垒增益点储备发弹量
} projectile_allowance_t;
```

---

## 三、自定义客户端协议中的哨兵相关指令

### 3.1 哨兵姿态同步 (SentryStatusSync)

**发送方**: 服务器 → 自定义客户端  
**频率**: 1Hz

```protobuf
message SentryStatusSync {
    uint32 posture_id = 1;    // 姿态ID: 1=进攻, 2=防御, 3=移动
    bool is_weakened = 2;     // 是否弱化
}
```

### 3.2 哨兵控制指令请求 (SentryCtrlCommand)

**发送方**: 自定义客户端 → 服务器  
**频率**: 1Hz

```protobuf
message SentryCtrlCommand {
    uint32 command_id = 1;    // 指令编号
}
```

**指令编号枚举**:
- 1: 补血点补弹
- 2: 补给站实体补弹
- 3: 远程补弹
- 4: 远程回血
- 5: 确认复活
- 6: 确认花费金币复活
- 7: 地图标点
- 8: 切换为进攻姿态
- 9: 切换为防御姿态
- 10: 切换为移动姿态

### 3.3 哨兵控制指令结果反馈 (SentryCtrlResult)

**发送方**: 服务器 → 自定义客户端  
**频率**: 1Hz

```protobuf
message SentryCtrlResult {
    uint32 command_id = 1;    // 对应的指令编号
    uint32 result_code = 2;   // 执行结果: 0=成功, 其他=失败
}
```

### 3.4 哨兵轨迹规划信息 (RobotPathPlanInfo)

**发送方**: 服务器 → 自定义客户端  
**频率**: 1Hz

```protobuf
message RobotPathPlanInfo {
    uint32 intention = 1;           // 意图: 1=攻击, 2=防守, 3=移动
    uint32 start_pos_x = 2;         // 起始点X(dm)
    uint32 start_pos_y = 3;         // 起始点Y(dm)
    repeated int32 offset_x = 4;    // X增量数组
    repeated int32 offset_y = 5;    // Y增量数组
    uint32 sender_id = 6;           // 发送者ID
}
```

---

## 四、哨兵代码对策与策略（C语言/STM32）

### 4.1 数据接收处理框架

```c
/* ==================== 数据结构定义 ==================== */

// 帧头结构
typedef __packed struct {
    uint8_t sof;           // 起始字节 0xA5
    uint16_t data_length;  // 数据长度
    uint8_t seq;           // 包序号
    uint8_t crc8;          // 帧头CRC8校验
} frame_header_t;

// 完整数据帧结构
typedef __packed struct {
    frame_header_t header;   // 帧头
    uint16_t cmd_id;         // 命令码
    uint8_t data[128];       // 数据段
    uint16_t crc16;          // 整包CRC16校验
} referee_frame_t;

// 哨兵状态数据结构
typedef struct {
    // 位置信息
    float hero_pos[2];       // 英雄位置 [x, y]
    float engineer_pos[2];   // 工程位置 [x, y]
    float infantry3_pos[2];  // 3号步兵位置 [x, y]
    float infantry4_pos[2];  // 4号步兵位置 [x, y]
    
    // 自身状态
    uint16_t current_hp;     // 当前血量
    uint16_t max_hp;         // 最大血量
    uint16_t ammo_17mm;      // 17mm发弹量
    uint16_t gold_coin;      // 金币数量
    
    // 决策相关信息
    uint32_t sentry_info;    // 哨兵信息
    uint16_t sentry_info_2;  // 哨兵信息2
    
    // 姿态
    uint8_t posture;         // 1=进攻, 2=防御, 3=移动
    bool is_out_of_combat;   // 是否脱战
    bool can_free_respawn;   // 是否可免费复活
    bool can_pay_respawn;    // 是否可付费复活
    uint32_t respawn_cost;   // 复活所需金币
    
    // 兑换相关
    uint16_t exchanged_ammo; // 已兑换发弹量
    uint8_t remote_ammo_cnt; // 远程兑换发弹量次数
    uint8_t remote_hp_cnt;   // 远程兑换血量次数
    
    // 能量机关
    bool rune_activatable;   // 能量机关是否可激活
} sentry_state_t;

// 全局哨兵状态
sentry_state_t g_sentry_state;
```

### 4.2 UART接收与解析

```c
/* ==================== UART配置 ==================== */

// 裁判系统串口配置
#define REFEREE_UART_BAUDRATE  115200
#define REFEREE_UART           USART1
#define REFEREE_RX_BUF_SIZE    256

// 接收缓冲区
uint8_t rx_buffer[REFEREE_RX_BUF_SIZE];
uint16_t rx_data_len = 0;

void referee_uart_init(void) {
    // 初始化USART1: 115200, 8N1
    // 配置DMA接收或中断接收
    // ...
}

/* ==================== 数据解析 ==================== */

// 解析帧头
bool parse_frame_header(uint8_t *data, frame_header_t *header) {
    header->sof = data[0];
    if (header->sof != 0xA5) return false;
    
    header->data_length = (data[2] << 8) | data[1];
    header->seq = data[3];
    header->crc8 = data[4];
    
    // 验证CRC8
    return verify_crc8(data, 5, header->crc8);
}

// 主解析函数
void referee_data_process(uint8_t *data, uint16_t len) {
    frame_header_t header;
    uint16_t cmd_id;
    uint8_t *payload;
    
    // 解析帧头
    if (!parse_frame_header(data, &header)) return;
    
    // 获取命令码
    cmd_id = (data[6] << 8) | data[5];
    payload = &data[7];
    
    switch (cmd_id) {
        case 0x0003:  // 血量数据
            parse_game_hp(payload);
            break;
        case 0x0208:  // 允许发弹量
            parse_projectile_allowance(payload);
            break;
        case 0x020B:  // 地面机器人位置
            parse_ground_robot_position(payload);
            break;
        case 0x020D:  // 哨兵自主决策信息
            parse_sentry_info(payload);
            break;
        default:
            break;
    }
}

// 解析地面机器人位置
void parse_ground_robot_position(uint8_t *data) {
    memcpy(&g_sentry_state.hero_pos[0], &data[0], 4);      // hero_x
    memcpy(&g_sentry_state.hero_pos[1], &data[4], 4);      // hero_y
    memcpy(&g_sentry_state.engineer_pos[0], &data[8], 4);  // engineer_x
    memcpy(&g_sentry_state.engineer_pos[1], &data[12], 4); // engineer_y
    memcpy(&g_sentry_state.infantry3_pos[0], &data[16], 4);// standard_3_x
    memcpy(&g_sentry_state.infantry3_pos[1], &data[20], 4);// standard_3_y
    memcpy(&g_sentry_state.infantry4_pos[0], &data[24], 4);// standard_4_x
    memcpy(&g_sentry_state.infantry4_pos[1], &data[28], 4);// standard_4_y
}

// 解析哨兵自主决策信息
void parse_sentry_info(uint8_t *data) {
    uint32_t sentry_info;
    uint16_t sentry_info_2;
    
    memcpy(&sentry_info, &data[0], 4);
    memcpy(&sentry_info_2, &data[4], 2);
    
    g_sentry_state.sentry_info = sentry_info;
    g_sentry_state.sentry_info_2 = sentry_info_2;
    
    // 解析各字段
    g_sentry_state.exchanged_ammo = sentry_info & 0x7FF;        // bit 0-10
    g_sentry_state.remote_ammo_cnt = (sentry_info >> 11) & 0x0F; // bit 11-14
    g_sentry_state.remote_hp_cnt = (sentry_info >> 15) & 0x0F;   // bit 15-18
    g_sentry_state.can_free_respawn = (sentry_info >> 19) & 0x01;// bit 19
    g_sentry_state.can_pay_respawn = (sentry_info >> 20) & 0x01; // bit 20
    g_sentry_state.respawn_cost = (sentry_info >> 21) & 0x3FF;   // bit 21-30
    
    g_sentry_state.is_out_of_combat = sentry_info_2 & 0x01;      // bit 0
    g_sentry_state.posture = (sentry_info_2 >> 12) & 0x03;       // bit 12-13
    g_sentry_state.rune_activatable = (sentry_info_2 >> 14) & 0x01;// bit 14
}
```

### 4.3 哨兵自主决策指令发送

```c
/* ==================== 决策指令发送 ==================== */

// 决策指令结构
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

// 发送决策指令
void send_sentry_command(uint32_t cmd_value) {
    uint8_t tx_buffer[128];
    uint16_t idx = 0;
    
    // 帧头
    tx_buffer[idx++] = 0xA5;
    tx_buffer[idx++] = 10;  // data_length低字节 (6+4=10)
    tx_buffer[idx++] = 0;   // data_length高字节
    tx_buffer[idx++] = get_next_seq();  // 包序号
    tx_buffer[idx++] = 0;   // CRC8占位
    
    // 命令码 0x0301
    tx_buffer[idx++] = 0x01;
    tx_buffer[idx++] = 0x03;
    
    // 数据段头 (子内容ID 0x0120)
    tx_buffer[idx++] = 0x20;
    tx_buffer[idx++] = 0x01;
    
    // 发送者ID (哨兵ID: 7或107)
    tx_buffer[idx++] = (ROBOT_ID == RED) ? 7 : 107;
    tx_buffer[idx++] = 0;
    
    // 接收者ID (服务器: 0x8080)
    tx_buffer[idx++] = 0x80;
    tx_buffer[idx++] = 0x80;
    
    // 哨兵指令数据 (4字节)
    memcpy(&tx_buffer[idx], &cmd_value, 4);
    idx += 4;
    
    // 计算CRC8
    tx_buffer[4] = calc_crc8(tx_buffer, 5);
    
    // 计算CRC16
    uint16_t crc16 = calc_crc16(tx_buffer, idx);
    tx_buffer[idx++] = crc16 & 0xFF;
    tx_buffer[idx++] = (crc16 >> 8) & 0xFF;
    
    // 发送
    uart_send(REFEREE_UART, tx_buffer, idx);
}

// 兑换发弹量（单调递增）
void sentry_exchange_ammo(uint16_t target_ammo) {
    static uint16_t last_ammo = 0;
    sentry_cmd_u cmd;
    
    cmd.value = 0;
    
    // 确保单调递增
    if (target_ammo > last_ammo) {
        cmd.bits.exchange_ammo = target_ammo;
        last_ammo = target_ammo;
        send_sentry_command(cmd.value);
    }
}

// 请求远程兑换发弹量（单调递增，每次+1）
void sentry_request_remote_ammo(void) {
    static uint8_t remote_ammo_cnt = 0;
    sentry_cmd_u cmd;
    
    cmd.value = 0;
    cmd.bits.remote_ammo_req = ++remote_ammo_cnt;
    send_sentry_command(cmd.value);
}

// 切换姿态
void sentry_set_posture(uint8_t posture) {
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.posture_cmd = posture;  // 1=进攻, 2=防御, 3=移动
    send_sentry_command(cmd.value);
}

// 确认复活
void sentry_confirm_respawn(void) {
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.confirm_respawn = 1;
    send_sentry_command(cmd.value);
}

// 确认付费复活
void sentry_confirm_pay_respawn(void) {
    sentry_cmd_u cmd;
    cmd.value = 0;
    cmd.bits.confirm_pay_respawn = 1;
    send_sentry_command(cmd.value);
}
```

### 4.4 哨兵路径数据发送

```c
/* ==================== 路径数据发送 ==================== */

// 路径点结构
typedef struct {
    float x;
    float y;
} path_point_t;

// 发送路径数据给选手端
void send_sentry_path(uint8_t intention, path_point_t *path, uint8_t point_num) {
    uint8_t tx_buffer[128];
    uint16_t idx = 0;
    
    // 帧头
    tx_buffer[idx++] = 0xA5;
    tx_buffer[idx++] = 103; // data_length = 103
    tx_buffer[idx++] = 0;
    tx_buffer[idx++] = get_next_seq();
    tx_buffer[idx++] = 0;   // CRC8占位
    
    // 命令码 0x0307
    tx_buffer[idx++] = 0x07;
    tx_buffer[idx++] = 0x03;
    
    // 意图 (1=攻击, 2=防守, 3=移动)
    tx_buffer[idx++] = intention;
    
    // 起点坐标 (单位: dm)
    uint16_t start_x = (uint16_t)(path[0].x * 10);
    uint16_t start_y = (uint16_t)(path[0].y * 10);
    tx_buffer[idx++] = start_x & 0xFF;
    tx_buffer[idx++] = (start_x >> 8) & 0xFF;
    tx_buffer[idx++] = start_y & 0xFF;
    tx_buffer[idx++] = (start_y >> 8) & 0xFF;
    
    // 路径点增量数组 (最多49个点)
    for (int i = 1; i < 50 && i < point_num; i++) {
        int16_t delta_x = (int16_t)((path[i].x - path[i-1].x) * 10);
        int16_t delta_y = (int16_t)((path[i].y - path[i-1].y) * 10);
        
        // 限制在-128~+127范围内
        delta_x = (delta_x > 127) ? 127 : (delta_x < -128) ? -128 : delta_x;
        delta_y = (delta_y > 127) ? 127 : (delta_y < -128) ? -128 : delta_y;
        
        tx_buffer[idx++] = (int8_t)delta_x;
    }
    // 填充剩余X增量
    for (int i = point_num; i < 50; i++) {
        tx_buffer[idx++] = 0;
    }
    
    // Y增量数组
    for (int i = 1; i < 50 && i < point_num; i++) {
        int16_t delta_y = (int16_t)((path[i].y - path[i-1].y) * 10);
        delta_y = (delta_y > 127) ? 127 : (delta_y < -128) ? -128 : delta_y;
        tx_buffer[idx++] = (int8_t)delta_y;
    }
    // 填充剩余Y增量
    for (int i = point_num; i < 50; i++) {
        tx_buffer[idx++] = 0;
    }
    
    // 发送者ID
    uint16_t sender_id = (ROBOT_ID == RED) ? 7 : 107;
    tx_buffer[idx++] = sender_id & 0xFF;
    tx_buffer[idx++] = (sender_id >> 8) & 0xFF;
    
    // 计算CRC
    tx_buffer[4] = calc_crc8(tx_buffer, 5);
    uint16_t crc16 = calc_crc16(tx_buffer, idx);
    tx_buffer[idx++] = crc16 & 0xFF;
    tx_buffer[idx++] = (crc16 >> 8) & 0xFF;
    
    uart_send(REFEREE_UART, tx_buffer, idx);
}
```

### 4.5 哨兵战术策略示例

```c
/* ==================== 战术策略 ==================== */

// 哨兵行为模式
typedef enum {
    SENTRY_MODE_IDLE,       // 待机
    SENTRY_MODE_ATTACK,     // 进攻
    SENTRY_MODE_DEFENSE,    // 防守
    SENTRY_MODE_PATROL,     // 巡逻
    SENTRY_MODE_FOLLOW,     // 跟随
    SENTRY_MODE_RETREAT,    // 撤退
} sentry_mode_t;

// 全局模式
sentry_mode_t g_current_mode = SENTRY_MODE_IDLE;

// 距离计算
float calc_distance(float x1, float y1, float x2, float y2) {
    return sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

// 进攻策略: 寻找敌方薄弱环节进攻
void strategy_attack(void) {
    // 获取敌方位置 (通过雷达或其他方式)
    // 计算最优进攻路径
    // 切换为进攻姿态
    sentry_set_posture(1);
    
    // 规划进攻路径
    path_point_t attack_path[10];
    // ... 路径规划算法 ...
    
    // 发送路径给操作手
    send_sentry_path(1, attack_path, 10);  // 1=攻击
}

// 防守策略: 保护己方基地/前哨站
void strategy_defense(void) {
    // 切换为防御姿态
    sentry_set_posture(2);
    
    // 在关键位置巡逻
    path_point_t patrol_path[20];
    // ... 巡逻路径规划 ...
    
    send_sentry_path(2, patrol_path, 20);  // 2=防守
}

// 跟随策略: 跟随己方英雄或步兵
void strategy_follow(uint8_t target_type) {
    float *target_pos;
    
    switch (target_type) {
        case 1: target_pos = g_sentry_state.hero_pos; break;
        case 2: target_pos = g_sentry_state.engineer_pos; break;
        case 3: target_pos = g_sentry_state.infantry3_pos; break;
        case 4: target_pos = g_sentry_state.infantry4_pos; break;
        default: return;
    }
    
    // 计算跟随路径 (保持一定距离)
    path_point_t follow_path[5];
    // ... 跟随路径计算 ...
    
    sentry_set_posture(3);  // 移动姿态
    send_sentry_path(3, follow_path, 5);  // 3=移动
}

// 复活管理
void handle_respawn(void) {
    // 检查是否可以免费复活
    if (g_sentry_state.can_free_respawn) {
        sentry_confirm_respawn();
        return;
    }
    
    // 检查是否可以付费复活
    if (g_sentry_state.can_pay_respawn) {
        // 根据金币情况决定是否付费复活
        if (g_sentry_state.gold_coin >= g_sentry_state.respawn_cost) {
            sentry_confirm_pay_respawn();
        }
    }
}

// 资源管理
void handle_resource(void) {
    // 如果发弹量不足，请求兑换
    if (g_sentry_state.ammo_17mm < 100) {
        // 请求远程兑换发弹量
        sentry_request_remote_ammo();
    }
}

// 主决策循环
void sentry_decision_loop(void) {
    // 检查血量状态
    if (g_sentry_state.current_hp == 0) {
        handle_respawn();
        return;
    }
    
    // 资源管理
    handle_resource();
    
    // 根据比赛阶段和战场态势选择策略
    switch (g_current_mode) {
        case SENTRY_MODE_ATTACK:
            strategy_attack();
            break;
        case SENTRY_MODE_DEFENSE:
            strategy_defense();
            break;
        case SENTRY_MODE_FOLLOW:
            strategy_follow(1);  // 跟随英雄
            break;
        case SENTRY_MODE_PATROL:
            // 巡逻逻辑
            break;
        case SENTRY_MODE_RETREAT:
            // 撤退逻辑
            break;
        default:
            break;
    }
}
```

---

## 五、关键注意事项

### 5.1 数据接收

1. **数据延迟**: 裁判系统数据延迟约130ms（正常）~200ms（恶劣环境）
2. **丢包率**: 正常<1%，恶劣环境约3%
3. **接收带宽**: 哨兵接收上限为5120字节/秒

### 5.2 指令发送

1. **单调递增规则**: 兑换发弹量值和远程兑换请求次数必须单调递增
2. **频率限制**: 0x0301命令码最高30Hz
3. **指令处理顺序**: 服务器从低位到高位依次处理

### 5.3 坐标系

- 原点: 红方补给站附近的场地围挡交点
- X轴正方向: 沿场地长边向蓝方
- Y轴正方向: 沿场地短边向红方停机坪

### 5.4 姿态切换

- 进攻姿态: 适合主动攻击
- 防御姿态: 适合保护要点
- 移动姿态: 适合快速转移

---

## 六、附录：机器人ID

| ID | 说明 |
|----|------|
| 7 | 红方哨兵机器人 |
| 107 | 蓝方哨兵机器人 |
| 0x8080 | 裁判系统服务器（哨兵自主决策指令接收者） |

---

*文档基于 RoboMaster 2026 机甲大师高校系列赛通信协议 V1.2.0 整理*
