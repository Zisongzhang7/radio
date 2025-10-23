# 🔔 闹钟和 Podcast 功能快速开始指南

## ✅ 迁移完成

已成功从 `xiaozhi-esp32-podcast` 项目迁移：
- ✅ 闹钟管理功能
- ✅ Podcast 播放功能

## 🚀 立即测试

### 1. 编译项目

```bash
# 如果还没有配置 ESP-IDF 环境
source ~/esp/esp-idf/export.sh

# 进入项目目录
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main

# 清理并重新编译
idf.py fullclean
idf.py build
```

### 2. 烧录固件

```bash
# 查找串口
ls /dev/cu.* | grep usb

# 烧录（替换 PORT 为实际串口）
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

### 3. 观察启动日志

成功启动后应该看到：
```
I (xxx) AlarmManager: Initialized successfully
I (xxx) Application: AlarmManager callback registered
I (xxx) MCP: Add tool: self.alarm.add
I (xxx) MCP: Add tool: self.alarm.remove
I (xxx) MCP: Add tool: self.alarm.list
I (xxx) MCP: Add tool: self.alarm.update
```

## 🎯 功能测试

### 测试 1: 添加闹钟（通过 AI）

**方法 1: 语音**
1. 说唤醒词："你好小智"
2. 说："帮我设置一个明天早上7点的闹钟"
3. AI 会通过 MCP 调用 `self.alarm.add`

**方法 2: 查看日志**
```
I (xxx) AlarmManager: Adding alarm at 07:00
I (xxx) AlarmManager: Alarm 1 added successfully
I (xxx) MCP: Alarm 1 added successfully at 07:00
```

### 测试 2: 查看闹钟列表

说："查看我的所有闹钟"

预期响应：
```json
{
  "alarms": [
    {
      "id": 1,
      "time": "07:00",
      "repeat": "once",
      "label": "明天早起",
      "enabled": true
    }
  ]
}
```

### 测试 3: 闹钟触发

等待闹钟时间到达，观察：

**日志输出：**
```
I (xxx) Application: Alarm triggered: 07:00 '明天早起'
I (xxx) Application: SetDeviceState: alarm
I (xxx) Application: Playing ringtone...
```

**预期行为：**
1. 开始播放 `ringtone.ogg`
2. 每3秒重复一次
3. 持续1分钟后自动停止

### 测试 4: 停止闹钟

**方法 1: 说唤醒词**
- 说："你好小智"
- 闹钟立即停止

**方法 2: 等待自动停止**
- 1分钟后自动停止

**日志输出：**
```
I (xxx) Application: Wake word detected during alarm
I (xxx) Application: StopAlarmLoop called
I (xxx) Application: SetDeviceState: idle
```

### 测试 5: 删除闹钟

说："删除第一个闹钟"

**日志输出：**
```
I (xxx) AlarmManager: Removing alarm 1
I (xxx) MCP: Alarm 1 removed successfully
```

## 🎧 Podcast 功能测试

Podcast 功能会自动在后台运行：

**日志输出：**
```
I (xxx) Application: Starting fetch_audio_loop task
I (xxx) OTA: Checking for incoming audio...
I (xxx) Application: Podcast mode: false
```

当有新音频时：
```
I (xxx) Application: Fetching new podcast audio...
I (xxx) Application: Podcast mode: true
I (xxx) Application: ToggleChatState for podcast
```

## 📊 验证清单

完成以下检查确认功能正常：

### 编译阶段
- [ ] 编译无错误
- [ ] 看到 `alarm_manager.cc` 被编译
- [ ] 看到 `ringtone.ogg` 被嵌入固件

### 运行阶段
- [ ] AlarmManager 初始化成功
- [ ] 4个 MCP 工具被注册
- [ ] fetch_audio_loop 任务启动
- [ ] 时钟定时器运行（每秒检查闹钟）

### 功能阶段
- [ ] 可以添加闹钟
- [ ] 可以查看闹钟列表
- [ ] 闹钟可以触发
- [ ] 铃声正常播放
- [ ] 唤醒词可停止闹钟
- [ ] 可以删除闹钟
- [ ] 闹钟持久化保存（重启后仍在）

## ⚙️ 高级配置

### 修改闹钟铃声

替换 `main/assets/common/ringtone.ogg` 为你自己的铃声文件，然后重新编译。

### 修改闹钟数量限制

在 `main/alarm_manager.h` 中：
```cpp
static constexpr int MAX_ALARMS = 10;  // 改为你需要的数量
```

### 修改闹钟播放时长

在 `main/application.cc` 的 `StartAlarmLoop()` 函数中：
```cpp
esp_timer_start_once(alarm_stop_timer_handle_, 60000000);  // 60秒，改为你需要的时长
```

### 修改闹钟播放间隔

在 `main/application.cc` 的 `StartAlarmLoop()` 函数中：
```cpp
esp_timer_start_periodic(alarm_timer_handle_, 3000000);  // 3秒，改为你需要的间隔
```

## 🐛 故障排除

### 问题 1: 编译错误 `alarm_manager.h not found`

**原因**: CMakeLists.txt 配置错误

**解决**:
```bash
grep "alarm_manager.cc" main/CMakeLists.txt
# 应该看到 alarm_manager.cc 在 SOURCES 列表中
```

### 问题 2: 闹钟不触发

**可能原因**:
1. 时间未同步 - 检查 `has_server_time_` 标志
2. 闹钟未启用 - 检查 `alarm.enabled` 字段
3. 时间不匹配 - 检查系统时间

**调试**:
```bash
# 监视日志，每10秒会输出系统状态
I (xxx) SystemInfo: Heap: xxx KB free
```

### 问题 3: 铃声不播放

**可能原因**:
1. `ringtone.ogg` 未嵌入 - 检查编译输出
2. 音量为0 - 调整音量

**验证**:
```bash
# 查找 ringtone.ogg 引用
grep -r "ringtone.ogg" build/
```

### 问题 4: MCP 工具未注册

**检查日志**:
```bash
idf.py monitor | grep "Add tool"
# 应该看到 4 个 self.alarm.* 工具
```

## 📞 获取帮助

如遇到问题：

1. **查看完整文档**
   - [MIGRATION_SUMMARY.md](MIGRATION_SUMMARY.md) - 迁移详情
   - [PROJECT_COMPARISON.md](PROJECT_COMPARISON.md) - 项目对比

2. **检查日志**
   ```bash
   idf.py monitor > debug.log 2>&1
   ```

3. **提交 Issue**
   - GitHub: https://github.com/78/xiaozhi-esp32/issues
   - 提供：日志、配置、错误信息

4. **社区支持**
   - QQ 群: 1011329060

---

## 🎉 成功标志

如果你看到：
- ✅ 编译成功
- ✅ AlarmManager 初始化日志
- ✅ MCP 工具注册日志
- ✅ 可以通过 AI 添加闹钟
- ✅ 闹钟可以正常触发和播放

**恭喜！功能迁移成功！**

享受你的新功能吧！ 🎊

---

**文档版本**: 1.0  
**更新时间**: 2025-10-10

