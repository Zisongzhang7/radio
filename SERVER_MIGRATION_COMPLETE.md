# ✅ 服务器配置迁移完成

## 📋 已完成的操作

### 1. ✅ 创建生产环境配置文件

**文件：** `sdkconfig.defaults.prod`
```
CONFIG_OTA_URL="https://xbh.xiguacity.cn/api/ota/"
```

### 2. ✅ 更新 sdkconfig 配置

已将服务器地址从：
- ❌ `https://api.tenclass.net/xiaozhi/ota/` （官方服务器）

更改为：
- ✅ `https://xbh.xiguacity.cn/api/ota/` （西瓜服务器 - 支持播客功能）

### 3. ✅ 创建服务器切换工具

**文件：** `switch_server.sh`

这是一个交互式脚本，可以轻松在不同服务器之间切换：
- 西瓜服务器（推荐，支持播客）
- 官方服务器
- 自定义服务器

---

## 🎯 当前配置状态

### 服务器配置
```
OTA URL: https://xbh.xiguacity.cn/api/ota/
```

### 支持的功能
- ✅ 基本语音对话
- ✅ OTA 固件更新
- ✅ 闹钟管理
- ✅ **播客播放**（"播放最近的看世界"）
- ✅ MCP 服务器交互

---

## 🚀 下一步操作

### 方法 1：立即编译和烧录（推荐）

```bash
# 重新配置项目（应用新的服务器地址）
idf.py reconfigure

# 编译和烧录
idf.py build flash monitor
```

### 方法 2：使用切换工具

如果将来需要切换服务器：
```bash
./switch_server.sh
```

---

## 📝 服务器对比

| 服务器 | 地址 | 播客功能 | 闹钟功能 | 当前使用 |
|--------|------|---------|---------|---------|
| 西瓜服务器 | xbh.xiguacity.cn | ✅ 支持 | ✅ 支持 | **✅ 是** |
| 官方服务器 | api.tenclass.net | ❓ 未知 | ✅ 支持 | ❌ 否 |

---

## 🎤 测试播客功能

烧录新固件后，对设备说：

1. **"播放最近的看世界"**
   - 设备会通过 MCP 向服务器请求《跟着课本看世界》的最新播客
   - 服务器返回音频 URL
   - 设备自动播放

2. **"设置明天早上7点的闹钟"**
   - 测试闹钟管理功能

3. **查看监视器日志**
   ```bash
   idf.py monitor
   ```
   
   应该看到类似：
   ```
   I (xxx) Ota: Check version URL: https://xbh.xiguacity.cn/api/ota/
   I (xxx) MCP: User requested podcast: 看世界
   I (xxx) Application: Sending MCP message: {"action":"request_podcast","query":"看世界"}
   ```

---

## 🔧 配置文件说明

### sdkconfig.defaults.prod
- 用于生产环境
- 配置西瓜服务器
- 可以手动合并到 `sdkconfig.defaults` 以设为默认

### sdkconfig
- 当前项目配置
- 已更新为西瓜服务器
- 编译时使用此配置

### switch_server.sh
- 交互式服务器切换工具
- 自动备份配置
- 提供友好的用户界面

---

## ⚠️ 重要提醒

### 1. 必须重新编译和烧录
更改服务器地址后，**必须**重新编译和烧录固件，配置才会生效。

### 2. 首次连接西瓜服务器
设备首次连接到西瓜服务器时，可能需要：
- 设备激活（如果需要）
- 获取新的 MQTT/WebSocket 配置

### 3. 网络连接
确保设备能够访问 `xbh.xiguacity.cn`（西瓜服务器）

---

## 📚 相关文档

- `MCP_SERVER_COMPARISON.md` - 服务器配置对比详细分析
- `PODCAST_GUIDE.md` - 播客功能使用指南
- `SERVER_CONFIG.md` - 服务器配置完整说明

---

## ✅ 迁移检查清单

- [x] 创建 `sdkconfig.defaults.prod` 文件
- [x] 更新 `sdkconfig` 配置
- [x] 创建服务器切换工具 `switch_server.sh`
- [x] 添加执行权限
- [ ] 重新配置项目 (`idf.py reconfigure`)
- [ ] 编译固件 (`idf.py build`)
- [ ] 烧录固件 (`idf.py flash`)
- [ ] 测试播客功能

---

## 🎉 总结

**服务器配置已成功从 xiaozhi-esp32-podcast 迁移到当前项目！**

- ✅ 设备端代码完整（闹钟 + 播客）
- ✅ MCP 工具完整（request_podcast, mode.toggle）
- ✅ 服务器配置正确（西瓜服务器）

**下一步：重新编译和烧录固件，然后测试"播放最近的看世界"功能！**

