# MCP 服务器配置说明

## ✅ 配置已完成

### 当前 MCP 服务器地址

```
https://xbh.xiguacity.cn/api/ota/
```

**这是西瓜硬件（西瓜音箱）的专用服务器，支持完整的播客功能。**

---

## 🔍 配置对比结果

### xiaozhi-esp32-podcast 项目
- 服务器：`https://xbh.xiguacity.cn/api/ota/`（西瓜服务器）
- 播客功能：✅ **可用**

### xiaozhi-esp32-main 项目（当前项目）
- **之前**：`https://api.tenclass.net/xiaozhi/ota/`（官方服务器）
- **现在**：`https://xbh.xiguacity.cn/api/ota/`（西瓜服务器）✅
- 播客功能：✅ **已配置**

---

## 📁 已迁移的配置文件

### 1. sdkconfig.defaults.prod
```bash
CONFIG_OTA_URL="https://xbh.xiguacity.cn/api/ota/"
```
- 用途：生产环境配置
- 来源：从 xiaozhi-esp32-podcast 项目复制

### 2. sdkconfig
```bash
CONFIG_OTA_URL="https://xbh.xiguacity.cn/api/ota/"
```
- 用途：当前项目配置
- 状态：✅ 已更新为西瓜服务器

---

## 🛠️ 可用工具

### switch_server.sh - 服务器切换工具
**功能：**
- 交互式切换服务器配置
- 支持西瓜服务器、官方服务器、自定义服务器
- 自动备份配置
- 可选自动重新配置项目

**使用方法：**
```bash
./switch_server.sh
```

### verify_server_config.sh - 配置验证工具
**功能：**
- 验证服务器配置
- 检查迁移的文件
- 检查 MCP 工具集成
- 显示当前配置状态

**使用方法：**
```bash
./verify_server_config.sh
```

---

## 🚀 编译和烧录

### 完整流程

```bash
# 1. 重新配置项目（应用新的服务器地址）
idf.py reconfigure

# 2. 编译固件
idf.py build

# 3. 烧录固件
idf.py flash

# 4. 监视日志
idf.py monitor

# 或者一条命令完成 2-4
idf.py build flash monitor
```

### 快速命令（推荐）

```bash
# 重新配置、编译、烧录、监视 - 一步完成
idf.py reconfigure && idf.py build flash monitor
```

---

## 🎤 测试播客功能

烧录新固件后，对设备说以下命令进行测试：

### 1. 播放播客
```
"播放最近的看世界"
```

**预期行为：**
- 设备通过 MCP 向西瓜服务器发送播客请求
- 服务器返回《跟着课本看世界》最新播客的音频 URL
- 设备自动开始播放

### 2. 测试闹钟
```
"设置明天早上7点的闹钟"
```

**预期行为：**
- 通过 MCP `self.alarm.add` 工具创建闹钟
- 设备确认闹钟已设置

### 3. 列出闹钟
```
"有哪些闹钟"
```

**预期行为：**
- 通过 MCP `self.alarm.list` 工具列出所有闹钟

---

## 📊 监视器日志示例

### 成功连接西瓜服务器
```
I (1234) Ota: Check version URL: https://xbh.xiguacity.cn/api/ota/
I (1235) Ota: Setup HTTP, User-Agent: ...
I (1500) Ota: Checking version...
I (2000) Ota: Server time: 2025-10-10 12:00:00
```

### MCP 播客请求
```
I (3000) MCP: Tool call: self.podcast.request
I (3001) MCP: User requested podcast: 看世界
I (3002) Application: Sending MCP message: {"action":"request_podcast","query":"看世界"}
```

### 播客播放
```
I (4000) Application: Incoming audio detected
I (4001) Application: Starting chat (toggle from idle)
I (4500) AudioService: Playing audio from URL: https://...
```

---

## 🔧 服务器架构说明

### OTA 服务器 (xbh.xiguacity.cn)

**作用：**
1. **设备激活和配置**
   - 提供设备激活 challenge
   - 返回设备配置

2. **固件更新**
   - 检查固件版本
   - 提供固件下载 URL

3. **通信服务器配置**
   - 返回 MQTT 或 WebSocket 服务器地址
   - 提供认证信息

4. **播客检查**
   - 响应 `CheckIncomingAudio` 请求
   - 返回 `incoming_audio: true` 标志

### 通信服务器 (MQTT/WebSocket)

**作用：**
1. **语音交互**
   - 接收音频流
   - 返回 AI 响应

2. **MCP 消息处理**
   - 处理 `request_podcast` 消息
   - 搜索播客内容
   - 返回音频 URL

3. **实时通信**
   - 接收设备状态
   - 推送指令

---

## 🌐 服务器对比

| 特性 | 官方服务器 | 西瓜服务器 | 自建服务器 |
|------|-----------|-----------|-----------|
| **地址** | api.tenclass.net | xbh.xiguacity.cn | 自定义 |
| **OTA 更新** | ✅ | ✅ | ✅ (需实现) |
| **基本对话** | ✅ | ✅ | ✅ (需实现) |
| **闹钟管理** | ✅ | ✅ | ✅ (设备端) |
| **播客功能** | ❓ 未知 | ✅ **支持** | ⚠️ 需实现 |
| **推荐使用** | 官方设备 | **播客功能** | 开发测试 |

---

## 📝 常见问题

### Q1: 为什么需要切换到西瓜服务器？
**A:** 西瓜服务器支持播客内容的搜索和推送，官方服务器目前不确定是否支持此功能。

### Q2: 切换服务器后需要重新激活设备吗？
**A:** 可能需要。不同服务器可能有不同的设备认证系统。

### Q3: 可以切换回官方服务器吗？
**A:** 可以，使用 `./switch_server.sh` 工具即可轻松切换。

### Q4: 自建服务器需要实现哪些功能？
**A:** 参考 `PODCAST_GUIDE.md` 和 `PODCAST_SERVER_DIAGNOSIS.md` 文档。

### Q5: 如何验证当前使用的服务器？
**A:** 运行 `./verify_server_config.sh` 或查看 `idf.py monitor` 日志。

---

## 📚 相关文档

- `SERVER_MIGRATION_COMPLETE.md` - 迁移完成总结
- `MCP_SERVER_COMPARISON.md` - 服务器详细对比
- `PODCAST_GUIDE.md` - 播客功能使用指南
- `PODCAST_SERVER_DIAGNOSIS.md` - 服务器端诊断
- `SERVER_CONFIG.md` - 完整服务器配置说明

---

## 🎉 总结

### ✅ 已完成
- [x] 服务器配置从 xiaozhi-esp32-podcast 迁移到当前项目
- [x] sdkconfig 已更新为西瓜服务器
- [x] 创建生产配置文件 `sdkconfig.defaults.prod`
- [x] 创建服务器切换工具 `switch_server.sh`
- [x] 创建配置验证工具 `verify_server_config.sh`

### 🔄 待操作
- [ ] 重新配置项目 (`idf.py reconfigure`)
- [ ] 编译固件 (`idf.py build`)
- [ ] 烧录固件 (`idf.py flash`)
- [ ] 测试播客功能（"播放最近的看世界"）

### 📌 关键信息
**当前 MCP 服务器：** `https://xbh.xiguacity.cn/api/ota/`

**这是支持播客功能的西瓜服务器，与 xiaozhi-esp32-podcast 项目使用的服务器完全一致！**

---

## 💬 需要帮助？

如果遇到问题：
1. 运行 `./verify_server_config.sh` 检查配置
2. 查看 `idf.py monitor` 日志
3. 参考相关文档
4. 检查网络连接到 `xbh.xiguacity.cn`

**祝您使用愉快！🎉**

