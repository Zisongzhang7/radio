# 功能迁移总结：闹钟管理 & Podcast 播放

## 📋 迁移完成概览

从 `xiaozhi-esp32-podcast` 项目成功迁移了两个核心功能模块到 `xiaozhi-esp32-main` 项目：

✅ **闹钟管理 (Alarm Manager)**
✅ **Podcast 播放 (Podcast Mode)**

---

## 🔧 修改的文件清单

### 1. 新增文件 (3 个)

| 文件路径 | 说明 |
|---------|------|
| `main/alarm_manager.h` | 闹钟管理器头文件 |
| `main/alarm_manager.cc` | 闹钟管理器实现文件 (12KB) |
| `main/assets/common/ringtone.ogg` | 闹钟铃声音频文件 (5.6KB) |

### 2. 修改的文件 (8 个)

| 文件路径 | 主要修改 |
|---------|---------|
| `main/application.h` | 添加闹钟和 Podcast 相关成员变量和方法 |
| `main/application.cc` | 实现闹钟和 Podcast 功能逻辑 |
| `main/device_state.h` | 添加 `kDeviceStateAlarm` 状态 |
| `main/CMakeLists.txt` | 添加 `alarm_manager.cc` 到编译列表 |
| `main/mcp_server.cc` | 添加闹钟管理 MCP 工具 |
| `main/assets/locales/zh-CN/language.json` | 添加 "ALARM" 字符串 |
| `main/assets/lang_config.h` | 自动生成（包含 OGG_RINGTONE 和 ALARM） |

---

## 📦 新增功能详情

### 🔔 闹钟管理功能

#### 核心特性
- ✅ 支持添加、删除、更新、列表查询闹钟
- ✅ 支持重复闹钟（每天、工作日、自定义）
- ✅ 持久化存储（保存到 NVS）
- ✅ 闹钟触发自动播放铃声
- ✅ 1分钟后自动停止
- ✅ 唤醒词可停止闹钟

#### AlarmManager 类方法
```cpp
// 核心方法
uint32_t AddAlarm(hour, minute, repeat_days, label);
bool RemoveAlarm(id);
bool UpdateAlarm(id, hour, minute, repeat_days, label);
bool EnableAlarm(id, enabled);
std::vector<AlarmEntry> GetAllAlarms();
std::string GetAlarmsJson();

// 检查和触发
void CheckAlarms();
uint32_t GetNextAlarmSeconds();
void TriggerAlarm(const AlarmEntry& alarm);
```

#### MCP 工具（通过 AI 控制）
- `self.alarm.add` - 添加闹钟
- `self.alarm.remove` - 删除闹钟
- `self.alarm.update` - 更新闹钟
- `self.alarm.list` - 查看所有闹钟

#### 使用示例
```cpp
// 添加闹钟
auto& alarm = AlarmManager::GetInstance();
uint32_t id = alarm.AddAlarm(7, 30, 0, "起床");  // 7:30 一次性闹钟

// 添加每天重复闹钟
id = alarm.AddAlarm(8, 0, 0x7F, "每天8点");  // 1234567 = 0x7F

// 查询闹钟
auto alarms = alarm.GetAllAlarms();
```

---

### 🎧 Podcast 播放功能

#### 核心特性
- ✅ 自动检测新音频内容
- ✅ 后台任务循环获取
- ✅ Podcast 模式切换
- ✅ 与闹钟协同工作

#### Application 类新增方法
```cpp
void FetchAudioLoop();          // Podcast 音频获取循环
void OnAlarmTriggered();        // 闹钟触发处理
void StartAlarmLoop();          // 开始闹钟循环播放
void StopAlarmLoop();           // 停止闹钟
void OnClockTimer();            // 时钟定时器（检查闹钟）
void ScheduleEvent();           // 事件调度
```

#### 新增成员变量
```cpp
bool podcast_mode_ = false;
TaskHandle_t fetch_audio_loop_task_handle_;
esp_timer_handle_t alarm_timer_handle_;
esp_timer_handle_t alarm_stop_timer_handle_;
```

---

## 🔄 代码改动详解

### 1. Application 类改动

#### 构造函数
```cpp
// 添加闹钟定时器
esp_timer_create_args_t alarm_timer_args = {...};
esp_timer_create(&alarm_timer_args, &alarm_timer_handle_);

esp_timer_create_args_t alarm_stop_timer_args = {...};
esp_timer_create(&alarm_stop_timer_args, &alarm_stop_timer_handle_);
```

#### Start() 函数
```cpp
// 初始化 AlarmManager
AlarmManager::GetInstance().Initialize();
AlarmManager::GetInstance().SetTriggerCallback([this](const AlarmEntry& alarm) {
    OnAlarmTriggered(alarm);
});

// 启动 Podcast 音频获取任务
xTaskCreate([](void* arg) {
    Application* app = (Application*)arg;
    app->FetchAudioLoop();
    vTaskDelete(NULL);
}, "fetch_audio_loop", 2048 * 2, this, 1, &fetch_audio_loop_task_handle_);
```

#### OnWakeWordDetected() 函数
```cpp
// 唤醒词可停止闹钟
if (device_state_ == kDeviceStateAlarm) {
    StopAlarmLoop();
    SetDeviceState(kDeviceStateIdle);
    return;
}
```

#### 时钟定时器改动
```diff
- xEventGroupSetBits(app->event_group_, MAIN_EVENT_CLOCK_TICK);
+ app->OnClockTimer();  // 直接调用，不再使用事件位
```

---

### 2. 设备状态改动

#### device_state.h
```diff
+ kDeviceStateAlarm,  // 新增闹钟状态
  kDeviceStateFatalError
```

#### STATE_STRINGS 数组
```diff
+ "alarm",
  "fatal_error",
```

---

### 3. MCP Server 改动

#### 新增 MCP 工具
```cpp
AddUserOnlyTool("self.alarm.add", ...);      // 添加闹钟
AddUserOnlyTool("self.alarm.remove", ...);   // 删除闹钟
AddUserOnlyTool("self.alarm.list", ...);     // 列表闹钟
AddUserOnlyTool("self.alarm.update", ...);   // 更新闹钟
```

---

### 4. 语言配置改动

#### language.json
```json
{
  "ALARM": "闹钟"
}
```

#### 音频文件
- `main/assets/common/ringtone.ogg` - 闹钟铃声（从 podcast 项目复制）

---

## 🎯 功能工作流程

### 闹钟工作流程

```
1. 用户通过 AI/MCP 添加闹钟
   ↓
2. AlarmManager 保存到 NVS
   ↓
3. OnClockTimer() 每秒检查一次
   ↓
4. 时间匹配 → OnAlarmTriggered()
   ↓
5. 设置状态为 kDeviceStateAlarm
   ↓
6. StartAlarmLoop() 开始播放铃声
   ↓
7. 每3秒播放一次，持续1分钟
   ↓
8. 用户说唤醒词 OR 1分钟到
   ↓
9. StopAlarmLoop() 停止闹钟
   ↓
10. 设置状态回 kDeviceStateIdle
```

### Podcast 工作流程

```
1. FetchAudioLoop 任务启动
   ↓
2. 每15秒检查一次新音频
   ↓
3. 发现新音频 → ToggleChatState()
   ↓
4. podcast_mode_ = true
   ↓
5. 播放完成 → podcast_mode_ = false
   ↓
6. 循环继续
```

---

## ⚙️ 编译配置

### CMakeLists.txt
```cmake
set(SOURCES 
    ...
    "mcp_server.cc"
    "alarm_manager.cc"  # ← 新增
    "system_info.cc"
    ...
)
```

### 音频资源嵌入
```cmake
file(GLOB COMMON_SOUNDS ${CMAKE_CURRENT_SOURCE_DIR}/assets/common/*.ogg)
# ringtone.ogg 会被自动嵌入到固件中
```

---

## 🔍 兼容性说明

### 与 v2 项目的区别

| 特性 | v2 main (原) | v2 main (迁移后) |
|------|-------------|------------------|
| 闹钟功能 | ❌ 无 | ✅ 有 |
| Podcast | ❌ 无 | ✅ 有 |
| Clock Timer | Event 方式 | 直接调用 |
| CLOCK_TICK Event | ✅ 有 | ❌ 删除 |
| main_event_loop_task_handle_ | ✅ 有 | ✅ 保留（用于 MCP） |
| UpgradeFirmware | ✅ 有 | ✅ 保留 |
| CheckAssetsVersion | ✅ 有 | ✅ 保留 |

### 保持原有功能
- ✅ Assets 版本检查（v2 独有）
- ✅ 固件升级功能（v2 独有）
- ✅ Emote 动画显示（v2 独有）
- ✅ LVGL UI 系统（v2 独有）
- ✅ v2 分区表支持

---

## 🧪 测试检查清单

### 功能测试

- [ ] **闹钟添加**: 通过 AI 或 MCP 添加闹钟
- [ ] **闹钟触发**: 等待闹钟时间到达，验证铃声播放
- [ ] **闹钟停止**: 说唤醒词停止闹钟
- [ ] **闹钟重复**: 测试每天/工作日重复
- [ ] **闹钟列表**: 查看所有闹钟
- [ ] **闹钟删除**: 删除指定闹钟
- [ ] **闹钟更新**: 更新闹钟时间
- [ ] **Podcast**: 测试音频自动获取

### 编译测试

```bash
# 1. 清理构建
idf.py fullclean

# 2. 重新配置
idf.py set-target esp32s3
./setup_custom_board.sh

# 3. 编译
idf.py build

# 4. 检查编译输出
# 应该看到 alarm_manager.cc 被编译
# 应该看到 ringtone.ogg 被嵌入
```

### 运行时测试

```bash
# 烧录并监视
idf.py -p PORT flash monitor

# 观察日志
# 应该看到:
# I (xxx) Application: Initializing ESP32-S3...
# I (xxx) AlarmManager: Initialized
# I (xxx) MCP: Add tool: self.alarm.add
# I (xxx) MCP: Add tool: self.alarm.remove
# I (xxx) MCP: Add tool: self.alarm.list
# I (xxx) MCP: Add tool: self.alarm.update
```

---

## 📝 API 使用示例

### C++ 代码使用

```cpp
// 添加闹钟
auto& alarm_mgr = AlarmManager::GetInstance();
uint32_t id = alarm_mgr.AddAlarm(7, 30, 0, "起床闹钟");

// 查询闹钟
auto alarms = alarm_mgr.GetAllAlarms();
for (const auto& alarm : alarms) {
    ESP_LOGI(TAG, "Alarm %d: %02d:%02d - %s", 
             alarm.id, alarm.hour, alarm.minute, alarm.label);
}

// 删除闹钟
alarm_mgr.RemoveAlarm(id);
```

### 通过 AI/MCP 使用

用户: "帮我设置一个明天早上7点的闹钟"
AI: 调用 `self.alarm.add` → `{hour: 7, minute: 0, repeat_days: "0", label: "早起"}`

用户: "查看我的所有闹钟"
AI: 调用 `self.alarm.list` → 返回 JSON 格式的闹钟列表

用户: "删除第一个闹钟"
AI: 调用 `self.alarm.remove` → `{id: 1}`

---

## ⚠️ 注意事项

1. **设备必须保持开机**
   - 闹钟需要设备持续运行才能触发
   - 断电或重启不会影响已保存的闹钟
   - 重新上电后闹钟继续有效

2. **时间同步要求**
   - 需要通过服务器同步时间
   - `has_server_time_` 标志位控制闹钟检查
   - 未同步时间前闹钟不会触发

3. **闹钟存储限制**
   - 最多支持 10 个闹钟（MAX_ALARMS = 10）
   - 存储在 NVS 分区，持久化保存

4. **Podcast 模式**
   - 自动检测间隔：15 秒
   - 播放中暂停检测：60 秒
   - 需要服务器支持音频推送

---

## 🎉 迁移成功标志

如果看到以下日志输出，说明迁移成功：

```
I (xxx) Application: Initializing ESP32-S3-DevKitC-1 Custom Board
I (xxx) AlarmManager: Loading alarms from NVS
I (xxx) AlarmManager: Loaded 0 alarms
I (xxx) AlarmManager: Initialized successfully
I (xxx) Application: AlarmManager callback registered
I (xxx) MCP: Add tool: self.alarm.add
I (xxx) MCP: Add tool: self.alarm.remove
I (xxx) MCP: Add tool: self.alarm.list
I (xxx) MCP: Add tool: self.alarm.update
```

---

## 📚 相关文档

- [闹钟管理器头文件](main/alarm_manager.h)
- [MCP 协议说明](docs/mcp-protocol.md)
- [MCP 使用指南](docs/mcp-usage.md)
- [项目对比分析](PROJECT_COMPARISON.md)

---

**迁移完成时间**: 2025-10-10  
**迁移版本**: v2 (xiaozhi-esp32-main)  
**源项目版本**: v1 (xiaozhi-esp32-podcast)  
**迁移状态**: ✅ 完成，待编译测试

