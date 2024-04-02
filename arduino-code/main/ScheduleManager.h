#ifndef ScheduleManager_h
#define ScheduleManager_h

#include <Preferences.h>
#include <ArduinoJson.h>

class ScheduleManager {
private:
    Preferences preferences;
    StaticJsonDocument<512> scheduleDoc;

public:
    ScheduleManager();
    void loadSchedule();
    bool isScheduledDay(int currentDayOfWeek);
    bool isScheduledTime(String currentTime, int currentDayOfWeek);
    void saveSchedule(const String& schedule);
};

#endif
