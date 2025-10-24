# Gravity 消息调试指南

## 🔍 问题

服务端确认接收正常（能收到 MCP 消息），但收不到 gravity 消息。

---

## 📊 已添加详细调试日志

### 1. SensorManager 层（sensor_manager.cc）

**位置**: 面2触发时

```
I (xxx) SensorManager: 🟢 姿态触发：发送重力模型2
I (xxx) SensorManager: 调用 PlaySound...
I (xxx) SensorManager: PlaySound 调用完成
I (xxx) SensorManager: 调用 SendGravityMessage...
I (xxx) SensorManager: SendGravityMessage 调用完成
```

### 2. Application 层（application.cc）

```
I (xxx) Application: Application::SendGravityMessage called with model: gravityModel_2
I (xxx) Application: Protocol available, checking thread...
I (xxx) Application: Not in main thread, scheduling to main thread
I (xxx) Application: Scheduled task executing, calling protocol_->SendGravityMessage
```

或

```
I (xxx) Application: Already in main thread, sending directly
```

### 3. Protocol 层（protocol.cc）

```
I (xxx) Protocol: Protocol::SendGravityMessage called with model: gravityModel_2
I (xxx) Protocol: Session ID: (empty) 或 abc123
I (xxx) Protocol: Constructed message: {"session_id":"xxx","type":"gravity","model":"gravityModel_2"}
I (xxx) Protocol: Calling SendText...
I (xxx) Protocol: SendText completed for gravity message: gravityModel_2
```

---

## 🔬 诊断步骤

### 步骤 1: 重新编译和烧录

```bash
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
. ~/esp/esp-idf/export.sh
idf.py build
idf.py flash
idf.py monitor
```

### 步骤 2: 翻转设备到面2

观察日志输出的完整流程。

### 步骤 3: 根据日志定位问题

#### 情况 A: 没有看到 "🟢 姿态触发"

**问题**: 传感器面检测没有工作

**检查**:
- 传感器是否正常初始化
- GPIO 配置是否正确
- 传感器数据是否正常读取

#### 情况 B: 看到 "姿态触发" 但没有 "Application::SendGravityMessage"

**问题**: `app.SendGravityMessage()` 调用没有执行

**可能原因**:
- 代码路径有问题
- `app` 引用无效
- 崩溃在 PlaySound

**检查**:
- 查看是否有崩溃重启
- 检查 PlaySound 是否导致问题

#### 情况 C: 看到 "Application::SendGravityMessage" 但显示 "protocol_ is nullptr"

**问题**: Protocol 未初始化或已释放

**解决**:
- 检查设备是否连接到服务器
- 查看 Application 初始化日志
- 确认 WiFi 连接状态

#### 情况 D: 看到 "Scheduled task executing" 但没有 "Protocol::SendGravityMessage"

**问题**: Schedule 任务执行有问题

**可能原因**:
- 主事件循环阻塞
- Schedule 队列满
- lambda 捕获有问题

#### 情况 E: 看到 "Calling SendText" 但服务器没收到

**问题**: SendText 没有真正发送，或网络层有问题

**检查**:
- 查看 WebSocket/MQTT 连接状态
- 检查 SendText 的实现
- 使用 Wireshark 抓包验证

---

## 🎯 对比 MCP 消息流程

### 测试 MCP 消息是否正常

先确认 MCP 消息能正常发送和接收：

```cpp
// 在 sensor_manager.cc 测试位置添加
ESP_LOGI(TAG, "测试发送 MCP 消息");
app.SendMcpMessage("{\"test\":\"hello\"}");
```

如果 MCP 消息能收到但 gravity 收不到，说明问题在 gravity 特定的代码路径中。

### 对比日志

**MCP 消息日志**（应该能看到）:
```
I (xxx) Application: (MCP 相关日志)
I (xxx) Protocol: (SendMcpMessage 日志)
```

**Gravity 消息日志**（看不到说明有问题）:
```
I (xxx) Application: Application::SendGravityMessage called...
I (xxx) Protocol: Protocol::SendGravityMessage called...
```

---

## 🐛 常见问题和解决方案

### 问题 1: PlaySound 导致崩溃

**症状**: 看到 "调用 PlaySound" 后设备重启

**解决**: 将 PlaySound 也放到 Schedule 中

```cpp
app.Schedule([&app]() {
    app.PlaySound(Lang::Sounds::OGG_POPUP);
    app.SendGravityMessage("gravityModel_2");
});
```

### 问题 2: Session ID 为空

**症状**: 日志显示 "Session ID: (empty)"

**说明**: 音频通道未打开，但消息应该仍能发送

**检查**: 服务器端是否要求 session_id 不为空

### 问题 3: SendText 没有实际发送

**症状**: 看到 "Calling SendText" 但服务器没收到

**可能原因**:
- WebSocket/MQTT 连接已断开
- SendText 内部有错误但没有日志
- 消息被过滤或缓存

**调试**:
```bash
# 抓包验证
tcpdump -i any -A host 172.25.75.94 and port 8003
```

### 问题 4: 线程调度问题

**症状**: "Scheduled task executing" 很久才出现或不出现

**原因**: 主事件循环繁忙或阻塞

**检查**:
- Application 主循环是否正常
- 是否有其他任务占用主线程

---

## 📝 完整日志示例

### 正常情况（完整流程）

```
I (12345) SensorManager: 🎲 当前朝上: 面 2 (背面)
I (12345) SensorManager: 🟢 姿态触发：发送重力模型2
I (12345) SensorManager: 调用 PlaySound...
I (12345) SensorManager: PlaySound 调用完成
I (12345) SensorManager: 调用 SendGravityMessage...
I (12346) Application: Application::SendGravityMessage called with model: gravityModel_2
I (12346) Application: Protocol available, checking thread...
I (12346) Application: Not in main thread, scheduling to main thread
I (12346) SensorManager: SendGravityMessage 调用完成
I (12347) Application: Scheduled task executing, calling protocol_->SendGravityMessage
I (12347) Protocol: Protocol::SendGravityMessage called with model: gravityModel_2
I (12347) Protocol: Session ID: abc123-xyz789
I (12347) Protocol: Constructed message: {"session_id":"abc123-xyz789","type":"gravity","model":"gravityModel_2"}
I (12347) Protocol: Calling SendText...
I (12347) Protocol: SendText completed for gravity message: gravityModel_2
```

### 异常情况 1: Protocol 为空

```
I (12345) SensorManager: 🟢 姿态触发：发送重力模型2
I (12345) SensorManager: 调用 SendGravityMessage...
I (12346) Application: Application::SendGravityMessage called with model: gravityModel_2
E (12346) Application: Application::SendGravityMessage - protocol_ is nullptr!
I (12346) SensorManager: SendGravityMessage 调用完成
```

### 异常情况 2: SendText 失败（需要检查 SendText 实现）

```
I (12347) Protocol: Calling SendText...
E (12347) WebsocketProtocol: SendText failed - not connected
```

---

## 🔧 临时测试代码

### 在 sensor_manager.cc 中添加 MCP 测试

```cpp
case 2:
    ESP_LOGI(TAG, "🟢 姿态触发：发送重力模型2");
    
    // 先测试 MCP 是否正常
    ESP_LOGI(TAG, "测试发送 MCP 消息...");
    app.SendMcpMessage("{\"test\":\"mcp_works\"}");
    
    // 再发送 gravity
    ESP_LOGI(TAG, "发送 Gravity 消息...");
    app.SendGravityMessage("gravityModel_2");
    
    last_triggered_face = 2;
    break;
```

如果 MCP 消息能到服务器但 gravity 不能，说明问题在 gravity 的处理逻辑中。

---

## 🚀 下一步

根据日志输出判断：

1. **如果完整流程都有日志**:
   - 设备端发送正常
   - 问题在服务器端接收/解析
   - 检查服务器处理 `type: "gravity"` 的代码

2. **如果中途中断**:
   - 记录中断位置
   - 检查该位置的代码
   - 可能有崩溃、异常或死锁

3. **如果根本没有日志**:
   - 传感器触发没有工作
   - 检查面检测逻辑

---

**创建时间**: 2025年10月24日  
**目的**: 通过详细日志定位 gravity 消息发送问题

