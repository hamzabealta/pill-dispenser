#include "TimeManager.h"

TimeManager::TimeManager(long offset) : timeClient(ntpUDP, "pool.ntp.org", offset * 3600, 60000) {}

bool TimeManager::isTimeSynced() const {
    return timeSynced;
}

void TimeManager::begin() {
    timeClient.begin();
}

bool TimeManager::update() {
    if (timeClient.update()) {
        timeSynced = true; // Set the flag to true on successful update
        return true;
    }
    return false;
}

String TimeManager::getCurrentTime() {
    unsigned long epochTime = timeClient.getEpochTime(); // Get the current time as epoch time

    // Convert epoch time to hours, minutes, and seconds
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    // int currentSecond = timeClient.getSeconds();

    // Format the time string as HH:MM:SS
    String formattedTime = "";
    if(currentHour < 10) formattedTime += "0";
    formattedTime += String(currentHour) + ":";
    if(currentMinute < 10) formattedTime += "0";
    formattedTime += String(currentMinute);
    // if(currentSecond < 10) formattedTime += "0";
    // formattedTime += String(currentSecond);

    return formattedTime; // Return the formatted string
}

int TimeManager::getCurrentDayOfWeek() {
    unsigned long epochTime = timeClient.getEpochTime();
    int dayOfWeek = ((epochTime / 86400L) + 4 ) % 7; 
    return dayOfWeek; // Return the day of the week as an integer (0 = Sunday, 1 = Monday, ..., 6 = Saturday)
}
