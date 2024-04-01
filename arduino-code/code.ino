#include <WiFi.h>
#include <WiFiProv.h>
#include <Preferences.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>

// Assuming placeholder implementations for these
#include "Button.h"
#include "LED.h"
#include <ArduinoJson.h> 

// WiFi Provisioning
const char* pop = "yfth65cjh";
const char* service_name = "MINI-PILL";

bool ble_provisioning = false;
// NTP for time synchronization
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0); // Offset is set later based on your timezone

// Button and LED
#define BUTTON_PIN 4 
#define LED_PIN 2   

// Preferences for storing schedules
Preferences preferences;
String schedule;

// Function declarations for later
void initiateBLEProvisioning();
void checkAndHandleDispensing();
void pullScheduleFromServer();
void storeScheduleLocally(const String& newSchedule);
void attemptReconnect();
void longPressCheckToReinitiateProvisioning();
void SysProvEvent(arduino_event_t *sys_event);


void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  preferences.begin("pill-dispenser", false);
  schedule = preferences.getString("schedule", ""); 

  WiFi.onEvent(SysProvEvent);
  if (!WiFi.isConnected()) {
    initiateBLEProvisioning();
  }
  
  timeClient.begin();
  timeClient.setTimeOffset(-1 * 3600);
}

unsigned long lastCheck = 0;
const long checkInterval = 61000;

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastCheck >= checkInterval) {
    lastCheck = currentMillis; 

    // Existing logic for WiFi connectivity and button press handling remains unchanged
    if (!WiFi.isConnected()) {
        Serial.println("Attempting to reconnect to WiFi...");
        WiFi.reconnect();
    }else{
      Serial.println("Attempting to pull schedule...");
      pullScheduleFromServer();
    }

    Serial.println("Attempting to checkAndHandleDispensing...");
    checkAndHandleDispensing();
  }

  longPressCheckToReinitiateProvisioning();
}

void initiateBLEProvisioning() {
  Serial.println("Starting BLE Provisioning");
  ble_provisioning = true;
  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name, NULL, NULL, false);
}

void SysProvEvent(arduino_event_t *sys_event)
{
    switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("\nConnected IP address : ");
        Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("\nDisconnected. Connecting to the AP again... ");
        break;
    case ARDUINO_EVENT_PROV_START:
        Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app");
        digitalWrite(LED_PIN, HIGH);
        break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
    }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        Serial.println("\nProvisioning Successful");
        break;
    case ARDUINO_EVENT_PROV_END:
        Serial.println("\nProvisioning Ends");
        digitalWrite(LED_PIN, LOW);
        ble_provisioning = false;
        break;
    default:
        break;
    }
}

void checkAndHandleDispensing() {
    Serial.println(F("Entering checkAndHandleDispensing"));

    if (!timeClient.update()) {
        Serial.println(F("Failed to update time from NTP server."));
        return;
    } else {
        Serial.println(F("Time updated successfully from NTP server."));
    }

    // Getting the current time and day of the week
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    int currentDayOfWeek = timeClient.getDay(); // Ensure this matches your server's day numbering

    // Combine current hour and minute to a string
    String currentTime = String(currentHour) + ":" + (currentMinute < 10 ? "0" : "") + String(currentMinute);
    
    Serial.print(F("Current Time: "));
    Serial.println(currentTime);

    // Load the schedule from preferences
    String scheduleJson = preferences.getString("schedule", "");
    if (scheduleJson == "") {
        Serial.println(F("No schedule found in preferences."));
        return;
    } else {
        Serial.println(F("Schedule loaded from preferences."));
    }

    Serial.println(scheduleJson);
    
    // Parse the JSON schedule
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, scheduleJson);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    } else {
        Serial.println(F("JSON schedule parsed successfully."));
    }

    // Check if today is one of the scheduled days
    JsonArray daysOfWeek = doc["daysOfWeek"];
    bool isScheduledDay = false;
    for (JsonVariant v : daysOfWeek) {
        if (v.as<int>() == currentDayOfWeek) {
            isScheduledDay = true;
            break;
        }
    }

    Serial.println(isScheduledDay ? F("Today is a scheduled day.") : F("Today is not a scheduled day."));
    if (!isScheduledDay) {
        digitalWrite(LED_PIN, LOW);
        return;
    }

    // Check if the current time is one of the scheduled times
    JsonArray times = doc["times"];
    bool isScheduledTime = false;
    for (JsonVariant v : times) {
        if (v.as<String>() == currentTime) {
            isScheduledTime = true;
            break;
        }
    }

    Serial.println(isScheduledTime ? F("Current time is a scheduled time.") : F("Current time is not a scheduled time."));
    // Turn on or off the LED based on the schedule
    if (isScheduledTime) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println(F("LED turned ON based on schedule."));
    } else {
        digitalWrite(LED_PIN, LOW);
        Serial.println(F("LED turned OFF as it's not a scheduled time."));
    }
}


void pullScheduleFromServer() {
    HTTPClient http;
    http.begin("http://192.168.1.137:3000/schedule?apikey=yourValidApiKey123"); // Replace <server-ip> with your server's IP address
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        deserializeAndStoreSchedule(payload);
    } else {
        Serial.println("Failed to retrieve the schedule");
    }

    http.end();
}

void deserializeAndStoreSchedule(const String& jsonSchedule) {
  StaticJsonDocument<512> doc; // Adjust size based on your JSON data
  DeserializationError error = deserializeJson(doc, jsonSchedule);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // Example of extracting data: Assume the JSON structure is known
  const char* validFrom = doc["validFrom"];
  const char* validUntil = doc["validUntil"];
  // Further processing based on your schedule's structure

  // Assuming you just store the raw JSON string for now
  storeScheduleLocally(jsonSchedule);
}

void storeScheduleLocally(const String& newSchedule) {
  preferences.putString("schedule", newSchedule);
  schedule = newSchedule; // Update the current schedule variable
}

void longPressCheckToReinitiateProvisioning() {
    while (digitalRead(BUTTON_PIN) == HIGH) {
        long pressStartTime = millis();
        if ((millis() - pressStartTime) > 3000) { // 5 seconds long press
            Serial.println("Long press detected, reinitiating provisioning");
            initiateBLEProvisioning();
            break;
        }else {
          if (digitalRead(LED_PIN) == HIGH) {
            digitalWrite(LED_PIN, LOW);
            Serial.println("Pill dispensed.");
          }
        }
    }
}