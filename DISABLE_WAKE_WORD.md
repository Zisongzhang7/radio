# 禁用语音唤醒词配置

## 目的
确保只通过按钮触发manual模式，而不是通过语音唤醒词触发auto模式。

## 配置步骤

### 方法1：通过menuconfig禁用（推荐）

```bash
cd /Users/zhangzisong/Desktop/test1.1/xiaozhi-esp32-main
idf.py menuconfig
```

在菜单中：
1. 进入 `Audio Configuration`
2. 找到 `Enable Wake Word Detection` 或类似选项
3. 取消勾选
4. 保存并退出

### 方法2：直接修改sdkconfig

在 `sdkconfig` 文件中添加或修改：
```
CONFIG_ENABLE_WAKE_WORD=n
# 或者
CONFIG_USE_WAKE_WORD=n
```

### 方法3：代码层面禁用

修改 `main/boards/bread-compact-wifi/compact_wifi_board.cc`，注释掉语音唤醒的初始化代码。

## 验证
重新编译烧录后，应该只能通过按住按钮来触发对话，日志会显示：
```
mode: manual
```


