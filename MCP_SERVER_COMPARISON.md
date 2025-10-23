# MCP 服务器配置对比分析

## 🔍 关键发现

### xiaozhi-esp32-podcast 项目的服务器配置

**两套配置：**

1. **生产环境（--prod）** - 使用西瓜服务器
   - 配置文件：`sdkconfig.defaults.prod`
   - 服务器地址：`https://xbh.xiguacity.cn/api/ota/`
   - **这是西瓜硬件（西瓜音箱）专用服务器，支持播客功能！**

2. **开发环境（--dev）** - 使用本地服务器
   - 配置文件：动态生成 `sdkconfig.defaults.dev`
   - 服务器地址：`http://本机IP:8003/api/ota/`

### xiaozhi-esp32-main 项目的当前配置

**只有一套配置：**
- 默认服务器：`https://api.tenclass.net/xiaozhi/ota/`
- **这是官方服务器，可能不支持播客功能！**

---

## 🎯 问题根源

**您在 xiaozhi-esp32-podcast 项目中可以正常使用播客功能，是因为：**
- 该项目使用的是**西瓜服务器**（xbh.xiguacity.cn）
- 西瓜服务器支持播客内容的搜索和推送

**当前 xiaozhi-esp32-main 项目无法使用播客功能，是因为：**
- 使用的是**官方服务器**（api.tenclass.net）
- 官方服务器可能不支持播客功能

---

## ✅ 解决方案

### 方案 1：使用西瓜服务器（推荐）

将 xiaozhi-esp32-podcast 的生产配置迁移到当前项目：

1. 创建 `sdkconfig.defaults.prod` 文件
2. 配置使用西瓜服务器
3. 重新编译和烧录固件

### 方案 2：部署自建服务器

如果要使用自己的服务器，需要：
1. 部署 xiaozhi-esp32-server 后端
2. 实现播客搜索和推送功能
3. 配置设备连接到自建服务器

---

## 📋 迁移步骤

### 1. 创建生产配置文件

创建 `sdkconfig.defaults.prod` 并配置西瓜服务器：
```
CONFIG_OTA_URL="https://xbh.xiguacity.cn/api/ota/"
```

### 2. 修改 sdkconfig

在 `sdkconfig` 文件中更新：
```
CONFIG_OTA_URL="https://xbh.xiguacity.cn/api/ota/"
```

### 3. 重新配置和编译

```bash
# 方法 1：使用 idf.py menuconfig
idf.py menuconfig
# 进入 Xiaozhi Assistant → Default OTA URL
# 修改为：https://xbh.xiguacity.cn/api/ota/

# 方法 2：直接修改配置文件后重新配置
idf.py reconfigure

# 编译和烧录
idf.py build flash monitor
```

---

## 🔄 两种服务器对比

| 特性 | 官方服务器 | 西瓜服务器 |
|------|-----------|-----------|
| 地址 | api.tenclass.net | xbh.xiguacity.cn |
| OTA 更新 | ✅ 支持 | ✅ 支持 |
| 基本对话 | ✅ 支持 | ✅ 支持 |
| 闹钟管理 | ✅ 支持 | ✅ 支持 |
| 播客功能 | ❓ 未知 | ✅ **支持** |
| 用途 | 官方设备 | 西瓜音箱 |

---

## 🎯 推荐配置

**如果您需要播客功能，强烈建议使用西瓜服务器！**

设备端代码已经完全准备好，只需要连接到支持播客的服务器即可。

---

## 📝 后续步骤

1. ✅ 确认使用西瓜服务器
2. ⏳ 迁移配置文件
3. ⏳ 重新编译和烧录
4. ⏳ 测试播客功能

