# 服务器配置说明

## 📡 默认服务器配置

### OTA 服务器地址

**默认 URL**: `https://api.tenclass.net/xiaozhi/ota/`

这是**官方服务器**（xiaozhi.me），用于：
1. 检查固件更新
2. 获取服务器地址（MQTT/WebSocket）
3. 设备激活和配置
4. **检查播客音频**（incoming_audio）

### 配置位置

**文件**: `main/Kconfig.projbuild`
```kconfig
config OTA_URL
    string "Default OTA URL"
    default "https://api.tenclass.net/xiaozhi/ota/"
```

**当前值**: `sdkconfig`
```
CONFIG_OTA_URL="https://api.tenclass.net/xiaozhi/ota/"
```

---

## 🔄 通信协议

设备支持两种通信协议：

### 1. MQTT + UDP

**默认协议**，由服务器通过 OTA 接口配置：

```json
{
  "mqtt": {
    "host": "mqtt服务器地址",
    "port": 1883,
    "client_id": "设备ID",
    "username": "用户名",
    "password": "密码"
  }
}
```

### 2. WebSocket

**备用协议**，由服务器通过 OTA 接口配置：

```json
{
  "websocket": {
    "url": "wss://服务器地址/ws",
    "token": "认证token"
  }
}
```

---

## 🎯 播客功能与服务器的关系

### 播客功能流程

```
设备 → OTA URL (api.tenclass.net) → 获取配置
  ↓
获取 MQTT/WebSocket 服务器地址
  ↓
连接到通信服务器
  ↓
用户说话 → 语音 → 服务器 ASR → LLM
  ↓
LLM 决定使用 MCP 工具
  ↓
服务器 → 设备：调用 self.podcast.request
  ↓
设备 → 服务器：发送 request_podcast MCP 消息
  ↓
服务器处理播客请求 ← 这里是关键！
  ↓
服务器推送音频到设备
```

### 关键点：官方服务器是否支持播客？

**重要问题**：`api.tenclass.net` **是否支持播客功能**？

您需要确认：
1. 官方服务器是否实现了播客搜索
2. 官方服务器是否有"看世界"等播客数据
3. 官方服务器是否会处理 `request_podcast` MCP 消息

---

## 🔍 如何检查服务器配置

### 方法 1：查看设备日志

设备启动并连接后：

```bash
idf.py monitor | grep -E "mqtt|websocket|Protocol"
```

应该看到：
```
I (xxx) Protocol: Using MQTT protocol
I (xxx) MQTT: Connecting to mqtt.server.com:1883
```
或
```
I (xxx) Protocol: Using WebSocket protocol
I (xxx) WebSocket: Connecting to wss://server.com/ws
```

### 方法 2：查看 CheckVersion 响应

```bash
idf.py monitor | grep -E "CheckVersion|mqtt|websocket"
```

设备会定期调用 OTA URL 检查版本，查看响应中的配置。

---

## 💻 自建服务器选项

如果官方服务器不支持播客功能，您需要**自建服务器**。

### 可用的开源服务器

1. **Python 服务器**
   - https://github.com/xinnan-tech/xiaozhi-esp32-server
   - 最流行，功能完整

2. **Java 服务器**
   - https://github.com/joey-zhou/xiaozhi-esp32-server-java
   - 适合企业环境

3. **Go 服务器**
   - https://github.com/AnimeAIChat/xiaozhi-server-go
   - 高性能

### 自建服务器需要实现

1. **OTA 接口**
   ```
   POST/GET https://your-server.com/ota/
   返回: {
     "firmware": {...},
     "mqtt": {...} 或 "websocket": {...},
     "incoming_audio": true/false
   }
   ```

2. **MQTT/WebSocket 通信**
   - 接收语音数据
   - ASR 识别
   - LLM 处理
   - TTS 生成
   - 返回音频

3. **MCP 协议支持**
   - 获取设备 MCP 工具列表
   - 让 LLM 知道有哪些工具可用
   - 调用设备端 MCP 工具
   - **处理设备发送的 MCP 消息**（如 request_podcast）

4. **播客功能**
   - 接收 `request_podcast` MCP 消息
   - 搜索播客数据库/RSS feeds
   - 获取音频 URL
   - 推送到设备

---

## ⚙️ 修改服务器配置

### 修改 OTA URL

有两种方法：

#### 方法 1：通过 menuconfig（推荐）

```bash
idf.py menuconfig
```

导航到：
```
Xiaozhi Assistant
  → Default OTA URL
```

修改为您的服务器地址，例如：
```
http://192.168.1.100:8000/ota/
```

#### 方法 2：直接修改 sdkconfig

编辑 `sdkconfig` 文件：
```
CONFIG_OTA_URL="http://192.168.1.100:8000/ota/"
```

然后重新编译：
```bash
idf.py build
```

### 修改通信协议配置

服务器地址**不是硬编码**的，而是通过 OTA 接口动态获取的。

服务器在响应 OTA 请求时返回：
```json
{
  "firmware": {
    "version": "1.0.0",
    "url": "https://..."
  },
  "mqtt": {
    "host": "mqtt.your-server.com",
    "port": 1883,
    "username": "device123",
    "password": "pass123"
  }
}
```

或

```json
{
  "firmware": {...},
  "websocket": {
    "url": "wss://your-server.com/ws",
    "token": "auth-token"
  }
}
```

设备会自动保存并使用这些配置。

---

## 🧪 测试服务器连接

### 1. 查看设备是否连接到服务器

```bash
idf.py monitor
```

查找：
```
I (xxx) wifi: connected to SSID
I (xxx) Ota: Checking version...
I (xxx) Protocol: Using MQTT protocol
I (xxx) MQTT: Connected to server
```

### 2. 手动测试 OTA 接口

```bash
curl -X POST https://api.tenclass.net/xiaozhi/ota/ \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "test-device",
    "version": "1.0.0"
  }'
```

查看返回的 JSON，确认：
- 是否有 `mqtt` 或 `websocket` 配置
- 是否有 `incoming_audio` 字段

---

## 📋 官方服务器 vs 自建服务器

| 特性 | 官方服务器 (xiaozhi.me) | 自建服务器 |
|------|------------------------|------------|
| 配置难度 | ✅ 简单（直接使用） | ⚠️ 需要部署 |
| 费用 | ✅ 免费（有限额） | 💰 服务器成本 |
| 功能支持 | ⚠️ 标准功能 | ✅ 可自定义 |
| 播客功能 | ❓ 不确定 | ✅ 可自己实现 |
| 隐私 | ⚠️ 数据上传到官方 | ✅ 数据在本地 |
| 稳定性 | ✅ 官方维护 | ⚠️ 自己维护 |
| 大模型选择 | ⚠️ 官方提供 | ✅ 自己选择 |

---

## 🎯 针对播客功能的建议

### 如果使用官方服务器

1. **联系官方支持**
   - 访问 xiaozhi.me
   - 询问是否支持播客功能
   - 询问如何配置播客数据源

2. **检查控制台**
   - 登录 xiaozhi.me 控制台
   - 查找播客相关设置
   - 查看 MCP 工具配置

### 如果需要播客功能但官方不支持

**建议：部署自建服务器**

1. **选择服务器**
   - 推荐 Python 服务器（最成熟）
   - https://github.com/xinnan-tech/xiaozhi-esp32-server

2. **配置播客**
   - 添加播客数据源（RSS feeds）
   - 实现播客搜索逻辑
   - 处理 `request_podcast` MCP 消息

3. **修改设备配置**
   ```bash
   idf.py menuconfig
   # 修改 OTA URL 为自建服务器
   ```

---

## 🔧 快速切换到自建服务器

### 步骤 1：部署 Python 服务器

```bash
git clone https://github.com/xinnan-tech/xiaozhi-esp32-server
cd xiaozhi-esp32-server
pip install -r requirements.txt
python server.py
```

### 步骤 2：修改设备配置

```bash
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
idf.py menuconfig
```

修改：
```
Xiaozhi Assistant → Default OTA URL
改为: http://192.168.1.xxx:8000/ota/
```

（将 xxx 替换为您的服务器 IP）

### 步骤 3：重新编译烧录

```bash
idf.py build flash monitor
```

### 步骤 4：在服务器端实现播客功能

参考 `PODCAST_GUIDE.md` 和 `PODCAST_ISSUE_ANALYSIS.md` 中的服务器端实现代码。

---

## 📞 进一步诊断

### 确认当前使用的服务器

1. **查看启动日志**
   ```bash
   idf.py monitor | grep "OTA"
   ```
   
   应该看到：
   ```
   I (xxx) Ota: Check version URL: https://api.tenclass.net/xiaozhi/ota/
   ```

2. **查看连接日志**
   ```bash
   idf.py monitor | grep -E "MQTT|WebSocket"
   ```
   
   查看连接到的服务器地址

### 确认播客功能支持

1. **测试语音命令**
   对设备说："播放最近的看世界"

2. **查看日志**
   ```bash
   idf.py monitor | grep -E "podcast|MCP"
   ```

3. **分析结果**
   - 如果有 `User requested podcast: 看世界` → 设备端正常
   - 如果有 `Sending MCP message` → 设备发送了请求
   - 如果之后没有音频推送 → **服务器端问题**

---

## 📚 相关文档

- [PODCAST_ISSUE_ANALYSIS.md](PODCAST_ISSUE_ANALYSIS.md) - 播客问题详细分析
- [PODCAST_GUIDE.md](PODCAST_GUIDE.md) - 播客功能完整指南
- [PODCAST_SERVER_DIAGNOSIS.md](PODCAST_SERVER_DIAGNOSIS.md) - 服务器端诊断
- [docs/mcp-protocol.md](docs/mcp-protocol.md) - MCP 协议说明
- [docs/websocket.md](docs/websocket.md) - WebSocket 协议文档
- [docs/mqtt-udp.md](docs/mqtt-udp.md) - MQTT+UDP 协议文档

---

## 🎉 总结

**当前服务器**：
- OTA: `https://api.tenclass.net/xiaozhi/ota/` （官方服务器）
- MQTT/WebSocket: 由 OTA 接口返回

**播客功能需要**：
- ✅ 设备端已完成（MCP 工具已集成）
- ⚠️ 服务器端需要支持（需确认官方是否支持）

**建议**：
1. 先联系官方确认是否支持播客
2. 如果不支持，考虑部署自建服务器
3. 参考开源项目实现播客功能

---

**当前默认服务器**: https://api.tenclass.net/xiaozhi/ota/ (官方)

