#ifndef TimeManager_h
#define TimeManager_h

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Arduino.h>

class TimeManager {
private:
    WiFiUDP ntpUDP;
    NTPClient timeClient;
    bool timeSynced = false; // Add this line
public:
    TimeManager(long offset = 0);
    void begin();
    bool update(); // Ensure this returns a bool indicating success
    String getCurrentTime();
    int getCurrentDayOfWeek();
    bool isTimeSynced() const;
};

#endif
