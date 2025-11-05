# AFE 音频处理任务丢失问题诊断

## 问题现象

第一次按键可以正常录音，第二次及以后按键无法录音。

## 根本原因

### AudioProcessor 的初始化机制

```cpp
// audio_service.cc:477-494
void AudioService::EnableVoiceProcessing(bool enable) {
    if (enable) {
        if (!audio_processor_initialized_) {
            audio_processor_->Initialize(codec_, ...);  // ← 只在第一次调用
            audio_processor_initialized_ = true;
        }
        audio_processor_->Start();  // ← 只设置 event bit
        xEventGroupSetBits(event_group_, AS_EVENT_AUDIO_PROCESSOR_RUNNING);
    } else {
        audio_processor_->Stop();
        xEventGroupClearBits(event_group_, AS_EVENT_AUDIO_PROCESSOR_RUNNING);
    }
}
```

### AFE 任务的创建

```cpp
// afe_audio_processor.cc:70-74
void AfeAudioProcessor::Initialize(...) {
    // ... 初始化 AFE 配置 ...
    
    xTaskCreate([](void* arg) {
        auto this_ = (AfeAudioProcessor*)arg;
        this_->AudioProcessorTask();  // ← 任务主循环
        vTaskDelete(NULL);  // ← 任务退出后删除
    }, "audio_communication", 4096, this, 3, NULL);
}
```

### AFE 任务主循环

```cpp
// afe_audio_processor.cc:121-150
void AfeAudioProcessor::AudioProcessorTask() {
    ESP_LOGI(TAG, "Audio communication task started, ...");  // ← 只打印一次
    
    while (true) {
        xEventGroupWaitBits(event_group_, PROCESSOR_RUNNING, ...);  // ← 等待运行标志
        
        auto res = afe_iface_->fetch_with_delay(afe_data_, portMAX_DELAY);
        
        if (res == nullptr || res->ret_value == ESP_FAIL) {
            // 可能在这里出错，导致任务退出？
            continue;
        }
        
        // 处理音频数据...
    }
}
```

## 问题分析

### 第一次（成功）

```
1. EnableVoiceProcessing(true)
2. audio_processor_initialized_ = false
3. 调用 Initialize()
4. 创建 "audio_communication" 任务 ✅
5. 打印 "Audio communication task started" ✅
6. 任务进入循环，等待 PROCESSOR_RUNNING bit
7. Start() 设置 PROCESSOR_RUNNING bit
8. 任务开始处理音频 ✅
```

### 第二次（失败）

```
1. EnableVoiceProcessing(false) - 在 Connecting 状态
2. Stop() 清除 PROCESSOR_RUNNING bit
3. 任务回到等待状态
   → 但任务可能在这里崩溃或退出了！

4. [200ms 延迟]

5. EnableVoiceProcessing(true)
6. audio_processor_initialized_ = true
7. 跳过 Initialize()，不创建新任务 ❌
8. Start() 设置 PROCESSOR_RUNNING bit
9. 但任务已经不存在了！❌
10. 没有音频处理 ❌
```

## 可能导致任务退出的原因

1. **栈溢出**：任务栈只有 4096 字节
2. **AFE 内部错误**：`fetch_with_delay` 返回 ESP_FAIL 太多次
3. **断言失败**：AFE 内部可能有断言
4. **内存不足**：PSRAM 不足导致 AFE 失败
5. **硬件资源冲突**：音频输入没有正确重置

## 日志证据

### 第一次（17773ms-18063ms）

```
I (18033) Application: 调用 EnableVoiceProcessing(true)
I (18033) AFE: AFE Version: (1MIC_V250121)  ← Initialize 被调用
I (18053) AfeAudioProcessor: Audio communication task started  ← 任务创建
I (18063) Application: 检查音频处理器是否成功启动: 是
```

### 第二次（22963ms-23233ms）

```
I (23033) Application: 强制关闭音频处理器（如果在运行）
I (23033) Application: 等待音频处理器完全停止...
I (23233) Application: 停止完成，准备启动
I (23233) Application: 调用 EnableVoiceProcessing(true)
[没有 AFE 日志 - Initialize 没有被调用]
I (23233) Application: EnableVoiceProcessing(true) 调用完成
I (23233) Application: 检查音频处理器是否成功启动: 是  ← 假阳性！
```

**关键差异**：第二次没有 AFE 和 AfeAudioProcessor 的日志！

## 解决方案

### 方案1：检查任务是否真的存在（推荐）

在启动前检查 "audio_communication" 任务是否真的在运行：

```cpp
// 检查 AFE 任务是否真的存在
TaskHandle_t afe_task = xTaskGetHandle("audio_communication");
if (afe_task == NULL) {
    ESP_LOGW(TAG, "AFE 任务不存在，需要重新初始化");
    // 强制重新初始化
}
```

### 方案2：增加 AFE 任务栈大小

```cpp
// 从 4096 增加到 8192
xTaskCreate(..., "audio_communication", 8192, this, 3, NULL);
```

### 方案3：捕获任务崩溃

在 `AudioProcessorTask` 中添加异常处理：

```cpp
void AudioProcessorTask() {
    ESP_LOGI(TAG, "Audio communication task started");
    
    while (true) {
        try {
            xEventGroupWaitBits(...);
            auto res = afe_iface_->fetch_with_delay(...);
            // ...
        } catch (...) {
            ESP_LOGE(TAG, "AFE 任务异常，继续运行");
        }
    }
}
```

### 方案4：添加任务健康检查

定期检查任务状态，如果发现任务不存在，自动重启：

```cpp
if (audio_service_.IsAudioProcessorRunning()) {
    TaskHandle_t afe_task = xTaskGetHandle("audio_communication");
    if (afe_task == NULL) {
        ESP_LOGE(TAG, "AFE 任务已退出，强制重新初始化");
        audio_service_.ResetAudioProcessor();
    }
}
```

## 下一步行动

1. **添加任务存在性检查**
2. **增加 AFE 任务栈大小**
3. **添加错误日志捕获**
4. **监控系统内存状态**

---

**关键结论**：`IsAudioProcessorRunning()` 只检查 event bit，不检查任务是否真的存在。需要添加实际的任务存在性检查。









