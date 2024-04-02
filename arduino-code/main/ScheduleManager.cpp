#include "ScheduleManager.h"

ScheduleManager::ScheduleManager() {
}

void ScheduleManager::loadSchedule() {
    preferences.begin("pill-schedule", false);
    String scheduleJson = preferences.getString("schedule", "{}");
    Serial.println("Loaded schedule: " + scheduleJson);
    preferences.end();
    // Parse the schedule JSON string into the StaticJsonDocument
    DeserializationError error = deserializeJson(scheduleDoc, scheduleJson);
    if (error) {
        Serial.print(F("Failed to parse schedule JSON: "));
        Serial.println(error.c_str());
    }
}

void ScheduleManager::saveSchedule(const String& schedule) {
    preferences.begin("pill-schedule", false);
    preferences.putString("schedule", schedule);
    Serial.println("Schedule saved: " + schedule);
    preferences.end();

    loadSchedule();
}


bool ScheduleManager::isScheduledDay(int currentDayOfWeek) {
    // Assuming scheduleDoc is already populated

    JsonArray daysOfWeek = scheduleDoc["daysOfWeek"];
    Serial.println(daysOfWeek);
    Serial.println(currentDayOfWeek);
    
    for(JsonVariant v : daysOfWeek) {

        if(v.as<int>() == currentDayOfWeek) {
            return true;
        }
    }
    return false;
}

bool ScheduleManager::isScheduledTime(String currentTime, int currentDayOfWeek) {
    if(!isScheduledDay(currentDayOfWeek)) {
        Serial.println(F("Today is not a scheduled day."));
        return false;
    }

    JsonArray times = scheduleDoc["times"];
    for(JsonVariant time : times) {
        if(time.as<String>() == currentTime) {
            return true;  // Current time matches a scheduled time
        }
    }

    return false; 
}
