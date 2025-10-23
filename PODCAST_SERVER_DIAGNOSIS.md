# 播客功能诊断指南

## 🔍 问题诊断

用户说："播放最近的看世界"，设备不能正常工作。

根据代码对比，**设备端的代码是完全正确的**，问题很可能在服务器端。

---

## 📊 完整流程分析

### 正常工作流程应该是：

```
1. 用户说话："播放最近的看世界"
   ↓
2. 设备唤醒，发送语音到服务器
   ↓
3. 服务器 ASR (语音识别)："播放最近的看世界"
   ↓
4. 服务器 LLM (大模型理解意图)
   ↓
5. LLM 决定调用 MCP 工具：self.podcast.request
   ↓
6. 设备收到 MCP 调用请求
   ↓
7. 设备执行 MCP 工具，发送消息到服务器：
   {
     "action": "request_podcast",
     "query": "看世界"
   }
   ↓
8. **服务器接收并处理播客请求** ← 关键环节！
   ↓
9. 服务器搜索"看世界"相关播客
   ↓
10. 服务器推送音频到设备
    ↓
11. 设备播放音频 ✅
```

---

## 🚨 可能的问题点

### 问题 1：服务器不支持 `self.podcast.request` 工具

**症状**：
- 用户说话后，AI 不知道如何响应
- 或者 AI 只是用 TTS 回答，不调用工具

**原因**：
- 服务器端没有注册 `self.podcast.request` 工具
- LLM 不知道有这个工具可用

**解决方案**：
需要确认服务器端是否正确返回了工具列表，包含 `self.podcast.request`

### 问题 2：服务器不处理 `request_podcast` MCP 消息

**症状**：
- 设备日志显示发送了 MCP 消息
- 但没有任何响应

**原因**：
- 服务器接收到 MCP 消息但不处理
- 或者服务器不知道如何处理 `request_podcast` action

**解决方案**：
服务器端需要实现播客请求处理逻辑

### 问题 3：服务器没有播客数据源

**症状**：
- 服务器处理了请求
- 但找不到"看世界"相关的播客

**原因**：
- 服务器没有配置播客数据库或 RSS feeds
- 没有《跟着课本看世界》的数据

**解决方案**：
需要在服务器端配置播客源

---

## 🔧 诊断步骤

### 步骤 1：检查设备日志

```bash
idf.py monitor | tee podcast_test.log
```

对设备说："播放最近的看世界"

查看日志中是否有：

**期望的日志 ✅**：
```
I (xxx) MCP: User requested podcast: 看世界
I (xxx) Application: Sending MCP message: {"action":"request_podcast","query":"看世界"}
```

如果**没有这些日志 ❌**：
- 说明 AI 没有调用 MCP 工具
- 问题在服务器端不知道有 `self.podcast.request` 工具

如果**有这些日志 ✅**：
- 说明设备端工作正常
- 问题在服务器端没有处理请求

### 步骤 2：检查 MCP 工具是否注册

设备启动时应该看到：

```
I (xxx) MCP: Add tool: self.podcast.request
I (xxx) MCP: Add tool: self.podcast.mode.toggle
```

如果**没有看到 ❌**：
- 重新编译烧录固件
- 确认使用的是最新代码

### 步骤 3：检查服务器类型

您使用的是哪个服务器？

#### A. 官方服务器 (xiaozhi.me)

如果使用官方服务器，需要确认：
1. 官方服务器是否支持播客功能
2. 是否需要特殊配置

#### B. 自建服务器

如果使用自建服务器（Python/Java/Go），需要：
1. 确认服务器版本支持 MCP 工具
2. 实现播客请求处理逻辑

---

## 💻 服务器端实现要求

### 1. 接收 MCP 消息

服务器需要监听设备发送的 MCP 消息：

```python
# Python 示例
def handle_mcp_message(device_id, message_json):
    message = json.loads(message_json)
    
    if message.get("action") == "request_podcast":
        query = message.get("query")
        handle_podcast_request(device_id, query)
```

### 2. 实现播客搜索

```python
def handle_podcast_request(device_id, query):
    # 搜索播客数据库
    podcast = search_podcast(query)
    
    if not podcast:
        # 没找到匹配的播客
        send_tts_response(device_id, f"抱歉，没有找到与'{query}'相关的播客")
        return
    
    # 获取最新一期
    latest_episode = podcast.get_latest_episode()
    
    # 推送音频
    push_audio(device_id, {
        "title": podcast.title,
        "episode": latest_episode.title,
        "url": latest_episode.audio_url
    })
```

### 3. 配置播客数据源

```python
# 示例：简单的播客数据库
PODCASTS = {
    "看世界": {
        "full_name": "跟着课本看世界",
        "rss_url": "https://example.com/feed/看世界",
        "description": "历史文化播客"
    },
    "新闻": {
        "full_name": "每日新闻播客",
        "rss_url": "https://example.com/feed/news",
        "description": "时事新闻"
    }
}

def search_podcast(query):
    # 关键词搜索
    for keyword, info in PODCASTS.items():
        if keyword in query or query in info["full_name"]:
            return info
    return None
```

---

## 🧪 快速测试方法

### 测试 1：简化测试

先测试一个简单的播客请求：

```bash
# 对设备说：
"播放播客"
```

查看是否触发 MCP 工具调用。

### 测试 2：直接推送测试

不通过语音请求，直接让服务器推送测试音频：

在服务器端设置 `incoming_audio = true`，看设备是否能播放。

如果能播放，说明：
- 设备端音频播放功能正常 ✅
- 问题在于 MCP 工具链路

### 测试 3：MCP 工具调用测试

使用简单的 MCP 工具测试：

```bash
# 对设备说：
"音量设置为50"
```

查看 `self.audio_speaker.set_volume` 工具是否正常工作。

如果工作正常，说明：
- MCP 工具机制正常 ✅
- 问题在于播客工具的特殊处理

---

## 📋 检查清单

请依次检查以下项目：

### 设备端 ✅

- [ ] `self.podcast.request` 工具已注册（查看启动日志）
- [ ] 用户说话后，设备发送 MCP 消息（查看运行日志）
- [ ] WiFi 连接正常
- [ ] MQTT/WebSocket 连接正常

### 服务器端 ⚠️

- [ ] 服务器版本支持 MCP 协议
- [ ] 服务器知道 `self.podcast.request` 工具存在
- [ ] 服务器能接收设备发送的 MCP 消息
- [ ] 服务器实现了 `request_podcast` 处理逻辑
- [ ] 服务器配置了播客数据源
- [ ] 服务器能推送音频到设备

---

## 🔍 详细日志分析

### 正常工作的完整日志应该是：

```
# 1. MCP 工具注册
I (5000) MCP: Add tool: self.podcast.request
I (5001) MCP: Add tool: self.podcast.mode.toggle

# 2. 用户唤醒设备
I (15000) WakeWord: Wake word detected

# 3. 用户说话被识别
I (20000) Protocol: << "播放最近的看世界"

# 4. AI 调用 MCP 工具
I (21000) MCP: tools/call: self.podcast.request
I (21001) MCP: User requested podcast: 看世界

# 5. 设备发送请求到服务器
I (21002) Application: Sending MCP message: {"action":"request_podcast","query":"看世界"}

# 6. 服务器响应（通过 OTA 检查或直接推送）
I (25000) Ota: Incoming audio detected
I (25001) Application: Fetching new podcast audio...

# 7. 开始播放
I (26000) AudioService: Playing incoming audio
```

### 异常情况分析

**情况 A：没有工具调用日志**
```
I (15000) WakeWord: Wake word detected
I (20000) Protocol: << "播放最近的看世界"
# ❌ 没有 "tools/call" 日志
```
**问题**：服务器端不知道有 `self.podcast.request` 工具

**情况 B：有工具调用但没有响应**
```
I (21000) MCP: tools/call: self.podcast.request
I (21002) Application: Sending MCP message: ...
# ❌ 之后没有 "Incoming audio" 日志
```
**问题**：服务器接收到请求但没有处理或推送音频

---

## 🆘 下一步行动

请提供以下信息以便继续诊断：

1. **使用的服务器类型**：
   - [ ] xiaozhi.me 官方服务器
   - [ ] 自建 Python 服务器
   - [ ] 自建 Java 服务器
   - [ ] 自建 Go 服务器
   - [ ] 其他：______

2. **设备日志**：
   运行 `idf.py monitor`，对设备说"播放最近的看世界"，复制完整日志

3. **服务器日志**（如果有访问权限）：
   查看服务器端是否收到 MCP 消息

4. **简化测试结果**：
   - 对设备说"音量设置为50"是否工作？
   - 对设备说"播放播客"有什么反应？

---

## 💡 临时解决方案

如果需要立即测试播客功能，可以：

### 方案 A：手动触发播放

在服务器端直接设置 `incoming_audio = true`，不需要通过 MCP 请求。

### 方案 B：使用简化版服务器

使用一个最简单的测试服务器，只需要：
1. 接收到 `request_podcast` 消息
2. 返回一个固定的测试音频 URL
3. 推送到设备

这样可以验证整个流程是否通畅。

---

## 📞 获取帮助

如果仍然无法解决，请：

1. **收集日志**
   ```bash
   idf.py monitor > podcast_debug.log 2>&1
   ```

2. **提供信息**
   - 服务器类型
   - 完整的设备日志
   - 服务器日志（如果可以访问）

3. **参考文档**
   - [PODCAST_GUIDE.md](PODCAST_GUIDE.md)
   - [docs/mcp-usage.md](docs/mcp-usage.md)
   - [docs/mcp-protocol.md](docs/mcp-protocol.md)

---

**设备端代码已完成，问题很可能在服务器端！** 🎯

