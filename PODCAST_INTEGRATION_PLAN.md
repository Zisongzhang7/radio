# Podcast 功能集成方案

## 当前状态

### ✅ 已实现的功能
- `FetchAudioLoop()` - 后台任务，每15秒检查一次 `incoming_audio`
- `podcast_mode_` - 播客模式标志
- `Ota::CheckIncomingAudio()` - 检查服务器上是否有新音频

### ❌ 缺失的功能
- 播客搜索和获取
- 播客列表管理
- **MCP 工具让 AI 控制播客**
- RSS feed 解析
- 播客元数据管理

---

## 问题分析

用户期望：
```
用户说："播放最近的看世界"
AI 应该：通过 MCP 工具获取《跟着课本看世界》最新一期并播放
```

但当前没有相应的 MCP 工具支持这个功能。

---

## 解决方案选项

### 方案 A：服务器端播客管理（推荐）

**架构**：
```
用户语音 → AI → MCP 工具 → 后端服务器 → 返回播客信息 → 设备播放
```

**需要的 MCP 工具**：
1. `self.podcast.search` - 搜索播客
2. `self.podcast.get_latest` - 获取最新节目
3. `self.podcast.play` - 请求服务器推送播客内容

**优点**：
- 设备端轻量化
- 服务器统一管理播客源
- 容易扩展和更新

**实现步骤**：
1. 添加 MCP 工具定义
2. 工具通过 HTTP API 与服务器通信
3. 服务器返回播客 URL 或直接推送音频流

---

### 方案 B：设备端 RSS 解析

**架构**：
```
用户语音 → AI → MCP 工具 → 设备解析 RSS → 下载并播放
```

**需要的组件**：
1. RSS/Atom feed 解析器
2. 播客订阅管理器
3. HTTP 下载器
4. MCP 工具集成

**优点**：
- 独立运行，不依赖服务器
- 可以订阅任何 RSS feed

**缺点**：
- 设备端内存占用大
- 实现复杂度高

---

### 方案 C：简化版 - 预设播客列表

**架构**：
```
用户语音 → AI → MCP 工具 → 从预设列表选择 → 通知服务器 → 播放
```

**实现**：
```cpp
// 预设播客列表
static const std::map<std::string, std::string> PODCAST_FEEDS = {
    {"看世界", "跟着课本看世界的 RSS URL 或 ID"},
    {"新闻", "某新闻播客 RSS URL 或 ID"},
    // ...
};
```

**MCP 工具**：
1. `self.podcast.list` - 列出可用播客
2. `self.podcast.play` - 通过名称或关键词播放

**优点**：
- 实现简单快速
- 资源占用小
- 易于维护

---

## 推荐实现方案

### 🎯 快速实现（方案 A + C 混合）

**步骤 1：添加基础 MCP 工具**

```cpp
// 在 mcp_server.cc 中添加

AddUserOnlyTool("self.podcast.play",
    "Request to play a podcast by name or keyword. "
    "The server will find and push the latest episode of the matching podcast.",
    PropertyList({
        Property("query", kPropertyTypeString, "Podcast name or keyword to search")
    }),
    [this](const PropertyList& properties) -> ReturnValue {
        auto query = properties["query"].value<std::string>();
        ESP_LOGI(TAG, "User requested podcast: %s", query.c_str());
        
        // 构造请求发送给服务器
        cJSON* request = cJSON_CreateObject();
        cJSON_AddStringToObject(request, "action", "play_podcast");
        cJSON_AddStringToObject(request, "query", query.c_str());
        
        char* json_str = cJSON_PrintUnformatted(request);
        std::string payload = json_str;
        free(json_str);
        cJSON_Delete(request);
        
        // 通过 Protocol 发送给服务器
        auto& app = Application::GetInstance();
        app.SendMcpMessage(payload);
        
        return "Podcast request sent to server";
    });

AddUserOnlyTool("self.podcast.list",
    "List all available podcasts",
    PropertyList(),
    [this](const PropertyList& properties) -> ReturnValue {
        // 返回预设的播客列表或从服务器获取
        cJSON* podcasts = cJSON_CreateArray();
        
        // 示例数据
        cJSON* item1 = cJSON_CreateObject();
        cJSON_AddStringToObject(item1, "name", "跟着课本看世界");
        cJSON_AddStringToObject(item1, "description", "历史文化播客");
        cJSON_AddItemToArray(podcasts, item1);
        
        char* json_str = cJSON_Print(podcasts);
        std::string result = json_str;
        free(json_str);
        cJSON_Delete(podcasts);
        
        return result;
    });
```

**步骤 2：服务器端支持**

服务器需要：
1. 接收 `play_podcast` MCP 消息
2. 根据 query 搜索播客
3. 获取最新一期的音频 URL
4. 通过 `incoming_audio` 标志或直接推送音频流

---

## 下一步行动

请告知：

1. **是否有后端服务器可以修改？**
   - 如果有，采用方案 A（服务器端管理）
   - 如果没有，采用方案 B（设备端 RSS）

2. **《跟着课本看世界》的播客源是什么？**
   - RSS feed URL？
   - API 接口？
   - 其他格式？

3. **期望的播放方式？**
   - 下载完整音频文件后播放？
   - 流式播放？
   - TTS 朗读内容？

回答这些问题后，我可以为您实现完整的播客 MCP 工具集成。

---

## 临时解决方案

如果需要快速测试，我可以先添加一个简化版工具：

```cpp
AddUserOnlyTool("self.podcast.toggle_mode",
    "Toggle podcast mode on/off",
    PropertyList(),
    [](const PropertyList& properties) -> ReturnValue {
        auto& app = Application::GetInstance();
        // 切换播客模式（需要添加公共接口）
        return "Podcast mode toggled";
    });
```

这样 AI 至少可以响应播客相关的请求，即使功能有限。

