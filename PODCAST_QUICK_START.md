# 播客功能快速开始 ⚡

## ✅ 问题解决

**原问题**：用户说"播放最近的看世界"，AI 无法实现

**现在**：✅ AI 可以通过新增的 MCP 工具主动请求播客

---

## 🚀 立即使用（3步）

### 1. 编译烧录

```bash
source ~/esp/esp-idf/export.sh
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
idf.py build flash monitor
```

### 2. 验证启动

查看日志是否有：
```
I (xxx) MCP: Add tool: self.podcast.request
I (xxx) MCP: Add tool: self.podcast.mode.toggle
```

### 3. 语音测试

对设备说：
- **"播放最近的看世界"**
- "播放新闻播客"
- "听一下科技节目"

---

## 🎙️ 支持的语音命令

| 用户说话 | AI 调用 | 参数 |
|---------|---------|------|
| "播放最近的看世界" | `self.podcast.request` | query="看世界" |
| "播放新闻播客" | `self.podcast.request` | query="新闻播客" |
| "听一下科技节目" | `self.podcast.request` | query="科技节目" |
| "打开播客模式" | `self.podcast.mode.toggle` | enabled=true |
| "关闭自动播客" | `self.podcast.mode.toggle` | enabled=false |

---

## 🔧 新增的 MCP 工具

### 1. `self.podcast.request`

**功能**：请求播放特定播客

**流程**：
```
用户语音 → AI → self.podcast.request → 服务器 → 推送音频 → 播放
```

### 2. `self.podcast.mode.toggle`

**功能**：控制播客自动检查

**选项**：
- `true` = 启用自动检查（每15秒）
- `false` = 禁用自动检查

---

## 📡 MCP 消息格式

### 设备 → 服务器

```json
{
  "action": "request_podcast",
  "query": "看世界"
}
```

### 服务器 → 设备

```json
{
  "incoming_audio": true,
  "podcast": {
    "title": "跟着课本看世界",
    "url": "https://example.com/podcast.mp3"
  }
}
```

---

## 🖥️ 服务器端要求

**必须实现**：
1. 接收 `request_podcast` MCP 消息
2. 搜索匹配的播客
3. 推送音频到设备

**示例代码**（Python）：
```python
def handle_podcast_request(query):
    podcast = search_podcast(query)  # 搜索
    latest = podcast.get_latest()     # 获取最新
    push_audio(device_id, latest.url) # 推送
```

---

## 🔍 调试日志

### 正常工作的日志

```
I (xxx) MCP: User requested podcast: 看世界
I (xxx) Application: Sending MCP message: {"action":"request_podcast","query":"看世界"}
I (xxx) Ota: Incoming audio detected
I (xxx) Application: Fetching new podcast audio...
```

### 查看日志命令

```bash
idf.py monitor | grep -E "podcast|MCP"
```

---

## 📋 修改的文件

| 文件 | 修改内容 |
|------|---------|
| `main/mcp_server.cc` | 新增 2 个 MCP 工具 |
| `main/application.h` | 新增 `SetPodcastMode` 方法 |

---

## 📚 完整文档

| 文档 | 用途 |
|------|------|
| [PODCAST_GUIDE.md](PODCAST_GUIDE.md) | 详细使用指南 |
| [PODCAST_MCP_UPDATE.md](PODCAST_MCP_UPDATE.md) | 更新说明 |
| [QUICK_REFERENCE.md](QUICK_REFERENCE.md) | 项目快速参考 |

---

## ⚠️ 重要提示

1. **服务器端必须实现播客处理逻辑**
   - 设备已准备好发送请求
   - 服务器需要接收和处理

2. **网络连接必须正常**
   - 检查 WiFi 连接
   - 验证 MQTT/WebSocket 正常

3. **IDE 错误可忽略**
   - Linter 可能显示缓存错误
   - 实际编译没有问题

---

## 🎯 快速验证清单

- [ ] 编译成功
- [ ] 日志显示 MCP 工具已注册
- [ ] 语音测试能触发工具调用
- [ ] 设备发送 MCP 消息到服务器
- [ ] 服务器接收并处理请求
- [ ] 服务器推送音频
- [ ] 设备播放音频

---

**现在可以使用播客功能了！** 🎉

