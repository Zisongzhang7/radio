# 播客功能使用指南

## ✅ 已完成的集成

### 1. 后台自动检查（已迁移）
- ✅ `FetchAudioLoop()` - 每15秒检查服务器上的新音频
- ✅ `Ota::CheckIncomingAudio()` - 检查 `incoming_audio` 标志
- ✅ 自动触发播放当服务器推送新内容

### 2. MCP 播客控制工具（新增）✨
- ✅ `self.podcast.request` - AI 可以主动请求播客
- ✅ `self.podcast.mode.toggle` - 控制播客模式开关

---

## 🎯 使用方法

### 方式 1：语音请求播客（推荐）

用户说话后，AI 会调用 MCP 工具：

**示例对话**：
```
用户: "播放最近的看世界"
AI: 调用 self.podcast.request(query="看世界")
设备: 发送请求到服务器
服务器: 找到《跟着课本看世界》最新一期
服务器: 推送音频到设备
设备: 自动开始播放
```

**更多示例**：
- "播放新闻播客"
- "听一下最新的科技节目"
- "播放历史故事"
- "我想听音乐电台"

### 方式 2：服务器自动推送（现有功能）

服务器可以主动推送音频：

```json
{
  "incoming_audio": true
}
```

设备会自动检测并播放。

---

## 🔧 MCP 工具详解

### 1. `self.podcast.request`

**用途**：请求播放特定播客或音频内容

**参数**：
- `query` (string): 播客名称、关键词或描述

**工作流程**：
```
1. AI 理解用户意图
2. 调用 self.podcast.request 工具
3. 设备构造 MCP 消息：
   {
     "action": "request_podcast",
     "query": "看世界"
   }
4. 通过 Protocol 发送给服务器
5. 服务器处理请求
6. 服务器推送对应音频
7. 设备播放
```

**示例 MCP 调用**：
```json
{
  "jsonrpc": "2.0",
  "method": "tools/call",
  "params": {
    "name": "self.podcast.request",
    "arguments": {
      "query": "跟着课本看世界"
    }
  },
  "id": 1
}
```

### 2. `self.podcast.mode.toggle`

**用途**：开关自动播客检查模式

**参数**：
- `enabled` (boolean): true=启用, false=禁用

**说明**：
- 启用时：每15秒检查一次服务器
- 禁用时：不自动检查（但仍可通过 `request` 主动请求）

**示例**：
```
用户: "关闭自动播客"
AI: 调用 self.podcast.mode.toggle(enabled=false)

用户: "打开播客模式"
AI: 调用 self.podcast.mode.toggle(enabled=true)
```

---

## 🖥️ 服务器端实现要求

为了支持播客功能，服务器需要：

### 1. 接收播客请求

监听设备发送的 MCP 消息：

```json
{
  "action": "request_podcast",
  "query": "看世界"
}
```

### 2. 处理请求逻辑

```python
def handle_podcast_request(query):
    # 1. 搜索播客
    podcast = search_podcast(query)  # 在数据库或 RSS feeds 中搜索
    
    # 2. 获取最新一期
    latest_episode = podcast.get_latest_episode()
    
    # 3. 准备音频 URL
    audio_url = latest_episode.audio_url
    
    # 4. 推送给设备
    push_audio_to_device(device_id, audio_url)
```

### 3. 推送方式

**方式 A：通过 `incoming_audio` 标志**

在设备的 `CheckVersion` 响应中：

```json
{
  "incoming_audio": true,
  "audio_url": "https://example.com/podcast.mp3",
  "title": "跟着课本看世界 - 第XX期",
  "description": "..."
}
```

**方式 B：直接推送音频流**

通过 MQTT/WebSocket 主动推送音频数据。

---

## 📡 通信协议

### 设备 → 服务器

```json
{
  "type": "mcp",
  "payload": {
    "action": "request_podcast",
    "query": "看世界",
    "device_id": "xxx",
    "timestamp": 1234567890
  }
}
```

### 服务器 → 设备

**选项 1：通过 CheckVersion 响应**

```json
{
  "firmware": {...},
  "incoming_audio": true,
  "podcast": {
    "title": "跟着课本看世界",
    "episode": "第10期",
    "url": "https://example.com/podcast.mp3"
  }
}
```

**选项 2：直接 TTS/音频推送**

服务器可以直接通过对话通道推送 TTS 内容或音频流。

---

## 🔍 调试和日志

### 设备端日志

启用后，您会看到：

```
I (xxx) MCP: User requested podcast: 看世界
I (xxx) Application: Sending MCP message: {"action":"request_podcast","query":"看世界"}
I (xxx) Ota: Incoming audio detected
I (xxx) Application: Fetching new podcast audio...
```

### 检查 MCP 工具注册

设备启动时应该看到：

```
I (xxx) MCP: Add tool: self.podcast.request
I (xxx) MCP: Add tool: self.podcast.mode.toggle
```

---

## 💡 常见问题

### Q1: 用户说"播放播客"，但没有反应？

**可能原因**：
1. 服务器端未实现播客请求处理
2. MCP 消息未正确发送
3. 服务器未返回音频

**调试步骤**：
```bash
# 查看设备日志
idf.py monitor

# 检查 MCP 消息是否发送
grep "User requested podcast" /dev/ttyUSB0

# 检查服务器日志
tail -f /path/to/server.log
```

### Q2: 如何自定义播客源？

**设备端**：
- 不需要修改设备代码
- 所有播客逻辑在服务器端实现

**服务器端**：
- 维护播客列表（数据库或配置文件）
- 实现搜索和匹配逻辑
- 返回对应的音频 URL

### Q3: 支持哪些音频格式？

根据设备的解码器配置：
- ✅ PCM（原始音频）
- ✅ Opus（推荐，低带宽）
- ✅ MP3（常见格式）
- ✅ AAC
- ✅ OGG/Vorbis

### Q4: 可以播放多长的音频？

取决于：
- 设备内存（流式播放可以播放任意长度）
- 服务器推送方式（建议流式推送）

---

## 🚀 快速测试

### 1. 编译并烧录

```bash
source ~/esp/esp-idf/export.sh
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
idf.py build flash monitor
```

### 2. 验证 MCP 工具已注册

设备启动后，查看日志：

```
I (xxx) MCP: Add tool: self.podcast.request
I (xxx) MCP: Add tool: self.podcast.mode.toggle
```

### 3. 测试语音请求

对设备说：
- "播放最近的看世界"
- "播放新闻"
- "听一下科技播客"

查看日志：
```
I (xxx) MCP: User requested podcast: 看世界
I (xxx) Application: Sending MCP message: ...
```

### 4. 服务器端处理（需要实现）

服务器收到 MCP 消息后：
1. 解析 `query` 参数
2. 搜索匹配的播客
3. 推送音频到设备

---

## 📋 下一步

### 设备端（已完成）✅
- ✅ MCP 工具集成
- ✅ 后台检查循环
- ✅ 自动播放逻辑

### 服务器端（需要实现）⚠️
- [ ] 接收 `request_podcast` MCP 消息
- [ ] 实现播客搜索逻辑
- [ ] 维护播客数据源（RSS/API/数据库）
- [ ] 推送音频到设备

### 可选增强
- [ ] 添加播客历史记录
- [ ] 支持播客订阅管理
- [ ] 支持播放进度控制
- [ ] 添加播客收藏功能

---

## 📞 技术支持

如果遇到问题：

1. **检查设备日志**
   ```bash
   idf.py monitor | grep -E "MCP|podcast|Ota"
   ```

2. **检查 MCP 消息格式**
   确保服务器正确解析 JSON 消息

3. **验证服务器响应**
   检查 `incoming_audio` 标志或音频推送是否正常

---

## 🎉 总结

现在您的设备支持：

✅ **主动请求播客** - 通过语音控制
✅ **被动接收播客** - 服务器自动推送
✅ **播客模式控制** - 开关自动检查

**下一步**：在服务器端实现播客请求处理逻辑，即可完整使用播客功能！

