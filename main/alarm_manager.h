#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <time.h>

#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

#include "settings.h"

struct AlarmEntry {
    uint32_t id;            // 闹钟唯一ID
    uint8_t hour;           // 小时 (0-23)
    uint8_t minute;         // 分钟 (0-59)
    uint8_t repeat_days;    // 重复日期位掩码 (bit0=周日, bit1=周一, etc.)
    bool enabled;           // 是否启用
    char label[32];         // 闹钟标签
    uint32_t created_time;  // 创建时间戳

    AlarmEntry() : id(0), hour(0), minute(0), repeat_days(0), enabled(true), created_time(0) {
        memset(label, 0, sizeof(label));
    }

    AlarmEntry(uint32_t _id, uint8_t _hour, uint8_t _minute, uint8_t _repeat_days, const char* _label,
               bool _enabled = true)
        : id(_id),
          hour(_hour),
          minute(_minute),
          repeat_days(_repeat_days),
          enabled(_enabled),
          created_time(time(nullptr)) {
        memset(label, 0, sizeof(label));
        if (_label) {
            strncpy(label, _label, sizeof(label) - 1);
        }
    }
};

class AlarmManager {
   public:
    // 版本管理
    static constexpr uint16_t ALARM_ENTRY_VERSION = 1;
    // 最大闹钟个数
    static constexpr int MAX_ALARMS = 10;
    
    // 闹钟触发回调函数类型
    using AlarmTriggerCallback = std::function<void(const AlarmEntry& alarm)>;
    
    static AlarmManager& GetInstance();

    // 核心功能
    uint32_t AddAlarm(uint8_t hour, uint8_t minute, uint8_t repeat_days, const char* label = "");
    bool RemoveAlarm(uint32_t id);
    bool EnableAlarm(uint32_t id, bool enabled);
    bool UpdateAlarm(uint32_t id, uint8_t hour, uint8_t minute, uint8_t repeat_days, const char* label);
    std::vector<AlarmEntry> GetAllAlarms();
    AlarmEntry* GetAlarm(uint32_t id);

    // 状态查询
    int GetAlarmCount();
    int GetActiveAlarmCount();
    std::string GetAlarmsJson();

    // 低功耗相关
    void CheckAlarms();              // 检查是否有闹钟需要触发
    uint32_t GetNextAlarmSeconds();  // 获取下次闹钟秒数(用于深度睡眠计算)
    bool ShouldTriggerAlarm(const AlarmEntry& alarm);

    // 系统集成
    void Initialize();
    void SetTriggerCallback(AlarmTriggerCallback callback);
    void TriggerAlarm(const AlarmEntry& alarm);

    // 工具函数
    static std::string RepeatDaysToString(uint8_t repeat_days);
    static uint8_t StringToRepeatDays(const std::string& days_str);
    static std::string FormatTime(uint8_t hour, uint8_t minute);

   private:
    AlarmManager();
    ~AlarmManager();

    void LoadAlarms();
    void SaveAlarm(const AlarmEntry& alarm);
    void DeleteAlarmFromStorage(uint32_t id);
    uint32_t GenerateNewId();

    std::vector<AlarmEntry> alarms_;
    Settings* settings_;
    uint32_t next_id_;
    uint32_t last_check_time_;  // 防重复触发
    AlarmTriggerCallback trigger_callback_;  // 闹钟触发回调

    // 禁止拷贝和赋值
    AlarmManager(const AlarmManager&) = delete;
    AlarmManager& operator=(const AlarmManager&) = delete;
};

#endif  // ALARM_MANAGER_H