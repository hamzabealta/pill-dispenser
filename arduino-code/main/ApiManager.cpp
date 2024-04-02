#include "ApiManager.h"

ApiManager::ApiManager() {}

void ApiManager::pullScheduleFromServer(std::function<void(String)> callback) {
    HTTPClient http;
    http.begin("http://192.168.1.137:3000/schedule?apikey=yourValidApiKey123");
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        callback(payload); // Invoke the callback with the payload
    } else {
        Serial.println("Failed to retrieve the schedule");
    }

    http.end();
}
