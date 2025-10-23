#include "alarm_manager.h"

#include <cJSON.h>
#include <esp_log.h>

#include <algorithm>
#include <cstring>

#define TAG "AlarmManager"

AlarmManager::AlarmManager() : settings_(nullptr), next_id_(1), last_check_time_(0) {}

AlarmManager::~AlarmManager() {
    if (settings_) {
        delete settings_;
    }
}

AlarmManager& AlarmManager::GetInstance() {
    static AlarmManager instance;
    return instance;
}

void AlarmManager::Initialize() {
    if (!settings_) {
        settings_ = new Settings("alarms", true);
    }
    LoadAlarms();
    ESP_LOGI(TAG, "AlarmManager initialized with %d alarms", GetAlarmCount());
}

void AlarmManager::LoadAlarms() {
    if (!settings_) return;

    alarms_.clear();
    next_id_ = settings_->GetInt("next_id", 1);

    // 加载所有闹钟
    for (uint32_t i = 1; i < next_id_; i++) {
        std::string alarm_key = "alarm_" + std::to_string(i);
        std::string version_key = "alarm_ver_" + std::to_string(i);

        // 先读取版本号
        uint16_t version = settings_->GetInt(version_key, 0);
        if (version == 0) {
            // 没有版本号，可能是旧数据，跳过
            continue;
        }

        if (version == ALARM_ENTRY_VERSION) {
            AlarmEntry alarm;
            if (settings_->GetBlob(alarm_key, &alarm, sizeof(AlarmEntry))) {
                alarms_.push_back(alarm);
            }
        } else {
            ESP_LOGW(TAG, "Unsupported alarm version %d for alarm %d", version, i);
        }
    }

    // 按ID排序
    std::sort(alarms_.begin(), alarms_.end(), [](const AlarmEntry& a, const AlarmEntry& b) { return a.id < b.id; });

    ESP_LOGI(TAG, "Loaded %d alarms from storage", static_cast<int>(alarms_.size()));
}

void AlarmManager::SaveAlarm(const AlarmEntry& alarm) {
    if (!settings_) return;

    std::string alarm_key = "alarm_" + std::to_string(alarm.id);
    std::string version_key = "alarm_ver_" + std::to_string(alarm.id);

    // 先保存版本号
    settings_->SetInt(version_key, ALARM_ENTRY_VERSION);
    // 再保存闹钟数据
    settings_->SetBlob(alarm_key, &alarm, sizeof(AlarmEntry));

    ESP_LOGI(TAG, "Saved alarm %u: %02d:%02d (version %d)", alarm.id, alarm.hour, alarm.minute, ALARM_ENTRY_VERSION);
}

void AlarmManager::DeleteAlarmFromStorage(uint32_t id) {
    if (!settings_) return;

    std::string alarm_key = "alarm_" + std::to_string(id);
    std::string version_key = "alarm_ver_" + std::to_string(id);

    settings_->EraseKey(alarm_key);
    settings_->EraseKey(version_key);

    ESP_LOGI(TAG, "Deleted alarm %u from storage", id);
}

uint32_t AlarmManager::GenerateNewId() {
    uint32_t id = next_id_++;
    if (settings_) {
        settings_->SetInt("next_id", next_id_);
    }
    return id;
}

uint32_t AlarmManager::AddAlarm(uint8_t hour, uint8_t minute, uint8_t repeat_days, const char* label) {
    if (hour > 23 || minute > 59) {
        ESP_LOGE(TAG, "Invalid alarm parameters: %02d:%02d", hour, minute);
        return 0;
    }

    if (static_cast<int>(alarms_.size()) >= MAX_ALARMS) {
        ESP_LOGE(TAG, "Cannot add alarm: maximum number of alarms (%d) reached", MAX_ALARMS);
        return 0;
    }

    uint32_t id = GenerateNewId();
    AlarmEntry alarm(id, hour, minute, repeat_days, label);

    alarms_.push_back(alarm);
    SaveAlarm(alarm);

    ESP_LOGI(TAG, "Added alarm %u: %02d:%02d, repeat=0x%02x, label='%s'", id, hour, minute, repeat_days,
             label ? label : "");

    return id;
}

bool AlarmManager::RemoveAlarm(uint32_t id) {
    auto it = std::find_if(alarms_.begin(), alarms_.end(), [id](const AlarmEntry& alarm) { return alarm.id == id; });

    if (it != alarms_.end()) {
        DeleteAlarmFromStorage(id);
        alarms_.erase(it);
        ESP_LOGI(TAG, "Removed alarm %u", id);
        return true;
    }

    ESP_LOGW(TAG, "Alarm %u not found for removal", id);
    return false;
}

bool AlarmManager::EnableAlarm(uint32_t id, bool enabled) {
    auto it = std::find_if(alarms_.begin(), alarms_.end(), [id](const AlarmEntry& alarm) { return alarm.id == id; });

    if (it != alarms_.end()) {
        it->enabled = enabled;
        SaveAlarm(*it);
        ESP_LOGI(TAG, "Alarm %u %s", id, enabled ? "enabled" : "disabled");
        return true;
    }

    ESP_LOGW(TAG, "Alarm %u not found for enable/disable", id);
    return false;
}

bool AlarmManager::UpdateAlarm(uint32_t id, uint8_t hour, uint8_t minute, uint8_t repeat_days, const char* label) {
    if (hour > 23 || minute > 59) {
        ESP_LOGE(TAG, "Invalid alarm parameters for update");
        return false;
    }

    auto it = std::find_if(alarms_.begin(), alarms_.end(), [id](const AlarmEntry& alarm) { return alarm.id == id; });

    if (it != alarms_.end()) {
        it->hour = hour;
        it->minute = minute;
        it->repeat_days = repeat_days;

        memset(it->label, 0, sizeof(it->label));
        if (label) {
            strncpy(it->label, label, sizeof(it->label) - 1);
        }

        SaveAlarm(*it);
        ESP_LOGI(TAG, "Updated alarm %u: %02d:%02d", id, hour, minute);
        return true;
    }

    ESP_LOGW(TAG, "Alarm %u not found for update", id);
    return false;
}

std::vector<AlarmEntry> AlarmManager::GetAllAlarms() { return alarms_; }

AlarmEntry* AlarmManager::GetAlarm(uint32_t id) {
    auto it = std::find_if(alarms_.begin(), alarms_.end(), [id](const AlarmEntry& alarm) { return alarm.id == id; });

    return (it != alarms_.end()) ? &(*it) : nullptr;
}

int AlarmManager::GetAlarmCount() { return static_cast<int>(alarms_.size()); }

int AlarmManager::GetActiveAlarmCount() {
    return static_cast<int>(
        std::count_if(alarms_.begin(), alarms_.end(), [](const AlarmEntry& alarm) { return alarm.enabled; }));
}

std::string AlarmManager::GetAlarmsJson() {
    cJSON* root = cJSON_CreateObject();
    cJSON* alarms_array = cJSON_CreateArray();

    for (const auto& alarm : alarms_) {
        cJSON* alarm_obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(alarm_obj, "id", alarm.id);
        cJSON_AddNumberToObject(alarm_obj, "hour", alarm.hour);
        cJSON_AddNumberToObject(alarm_obj, "minute", alarm.minute);
        cJSON_AddStringToObject(alarm_obj, "time", FormatTime(alarm.hour, alarm.minute).c_str());
        cJSON_AddStringToObject(alarm_obj, "repeat_days", RepeatDaysToString(alarm.repeat_days).c_str());
        cJSON_AddBoolToObject(alarm_obj, "enabled", alarm.enabled);
        cJSON_AddStringToObject(alarm_obj, "label", alarm.label);
        cJSON_AddNumberToObject(alarm_obj, "created_time", alarm.created_time);

        cJSON_AddItemToArray(alarms_array, alarm_obj);
    }

    cJSON_AddItemToObject(root, "alarms", alarms_array);
    cJSON_AddNumberToObject(root, "total", GetAlarmCount());
    cJSON_AddNumberToObject(root, "active", GetActiveAlarmCount());
    cJSON_AddNumberToObject(root, "next_in_seconds", GetNextAlarmSeconds());

    char* json_str = cJSON_PrintUnformatted(root);
    std::string result(json_str);
    cJSON_free(json_str);
    cJSON_Delete(root);

    return result;
}

void AlarmManager::CheckAlarms() {
    if (alarms_.empty()) {
        return;
    }

    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);

    if (!timeinfo) {
        ESP_LOGW(TAG, "Failed to get time info");
        return;
    }

    uint8_t current_hour = timeinfo->tm_hour;
    uint8_t current_minute = timeinfo->tm_min;
    uint8_t current_second = timeinfo->tm_sec;
    uint8_t current_wday = timeinfo->tm_wday;  // 0 = Sunday

    // 防重复触发：检查是否在同一分钟内已经检查过
    uint32_t current_minute_timestamp = current_hour * 60 + current_minute;
    if (last_check_time_ == current_minute_timestamp) return;

    last_check_time_ = current_minute_timestamp;

    ESP_LOGI(TAG, "Checking alarms at %02d:%02d:%02d (wday=%d)", current_hour, current_minute, current_second,
             current_wday);

    // 遍历闹钟，使用索引避免const问题
    for (size_t i = 0; i < alarms_.size(); i++) {
        AlarmEntry& alarm = alarms_[i];
        if (!alarm.enabled) continue;

        // 快速时间匹配检查
        if (alarm.hour != current_hour || alarm.minute != current_minute) {
            continue;
        }

        // 检查重复日期
        if (alarm.repeat_days == 0) {
            // 一次性闹钟，触发后直接删除
            TriggerAlarm(alarm);
            RemoveAlarm(alarm.id);
        } else {
            // 重复闹钟，检查是否匹配今天
            uint8_t today_bit = 1 << current_wday;
            if (alarm.repeat_days & today_bit) {
                TriggerAlarm(alarm);
            }
        }
    }
}

uint32_t AlarmManager::GetNextAlarmSeconds() {
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);

    if (!timeinfo) return 0;

    uint32_t min_seconds = UINT32_MAX;

    for (const auto& alarm : alarms_) {
        if (!alarm.enabled) continue;

        // 计算今天剩余的闹钟时间
        uint32_t alarm_seconds_today = alarm.hour * 3600 + alarm.minute * 60;
        uint32_t current_seconds = timeinfo->tm_hour * 3600 + timeinfo->tm_min * 60 + timeinfo->tm_sec;

        if (alarm.repeat_days == 0) {
            // 一次性闹钟
            if (alarm_seconds_today > current_seconds) {
                min_seconds = std::min(min_seconds, alarm_seconds_today - current_seconds);
            }
        } else {
            // 重复闹钟，查找最近的匹配日期
            for (int days_ahead = 0; days_ahead < 7; days_ahead++) {
                int check_wday = (timeinfo->tm_wday + days_ahead) % 7;
                uint8_t day_bit = 1 << check_wday;

                if (alarm.repeat_days & day_bit) {
                    uint32_t seconds_to_alarm;
                    if (days_ahead == 0 && alarm_seconds_today > current_seconds) {
                        // 今天的闹钟
                        seconds_to_alarm = alarm_seconds_today - current_seconds;
                    } else if (days_ahead > 0) {
                        // 未来几天的闹钟
                        seconds_to_alarm = days_ahead * 86400 + alarm_seconds_today - current_seconds;
                    } else {
                        continue;  // 今天已过的闹钟
                    }

                    min_seconds = std::min(min_seconds, seconds_to_alarm);
                    break;  // 找到最近的就停止
                }
            }
        }
    }

    return (min_seconds == UINT32_MAX) ? 0 : min_seconds;
}

bool AlarmManager::ShouldTriggerAlarm(const AlarmEntry& alarm) {
    if (!alarm.enabled) return false;

    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);

    if (!timeinfo) return false;

    uint8_t current_hour = timeinfo->tm_hour;
    uint8_t current_minute = timeinfo->tm_min;
    uint8_t current_wday = timeinfo->tm_wday;

    if (alarm.hour != current_hour || alarm.minute != current_minute) {
        return false;
    }

    if (alarm.repeat_days == 0) {
        return true;  // 一次性闹钟
    }

    uint8_t today_bit = 1 << current_wday;
    return (alarm.repeat_days & today_bit) != 0;
}

void AlarmManager::SetTriggerCallback(AlarmTriggerCallback callback) { trigger_callback_ = callback; }

void AlarmManager::TriggerAlarm(const AlarmEntry& alarm) {
    ESP_LOGI(TAG, "Triggering alarm %u: %02d:%02d '%s'", alarm.id, alarm.hour, alarm.minute, alarm.label);

    // 调用回调函数处理闹钟触发
    if (trigger_callback_) {
        trigger_callback_(alarm);
    } else {
        ESP_LOGW(TAG, "No trigger callback set for alarm %u", alarm.id);
    }
}

std::string AlarmManager::RepeatDaysToString(uint8_t repeat_days) {
    if (repeat_days == 0) return "Once";
    if (repeat_days == 0x7F) return "Everyday";

    std::string result;
    const char* day_names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    for (int i = 0; i < 7; i++) {
        if (repeat_days & (1 << i)) {
            if (!result.empty()) result += ",";
            result += day_names[i];
        }
    }

    return result;
}

uint8_t AlarmManager::StringToRepeatDays(const std::string& days_str) {
    if (days_str.empty() || days_str == "0") return 0;
    if (days_str == "1234567" || days_str == "everyday") return 0x7F;

    uint8_t repeat_days = 0;
    for (char c : days_str) {
        if (c >= '0' && c <= '6') {
            int day = c - '0';
            repeat_days |= (1 << day);
        }
    }

    return repeat_days;
}

std::string AlarmManager::FormatTime(uint8_t hour, uint8_t minute) {
    char buffer[8];
    snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
    return std::string(buffer);
}