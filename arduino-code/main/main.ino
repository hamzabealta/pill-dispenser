#include <Preferences.h>
#include "TimeManager.h"
#include "WiFiManager.h"
#include "ApiManager.h"
#include "ScheduleManager.h"
#include "ButtonHandler.h"
#include "LEDController.h"

#define LED_PIN 2
#define BUTTON_PIN 4

// WiFi Provisioning
const char* pop = "yfth65cjh";
const char* service_name = "MINI-PILL";


TimeManager timeManager(7200);
WiFiManager wifiManager(&timeManager);
ApiManager apiManager;
ScheduleManager scheduleManager;
LEDController ledController(LED_PIN);

// Callbacks for ButtonHandler
void shortPressAction() {
  // Action for short button press, turn off the led (pill dispensed)
  Serial.println("Short press");
  ledController.turnOff(); 
}

void longPressAction() {
  wifiManager.forgetConfiguration();
  Serial.println("Long press detected. Resetting the chip...");
  delay(1000); // Short delay to allow serial message to be sent before reset
  ESP.restart();
}

ButtonHandler buttonHandler(BUTTON_PIN, shortPressAction, longPressAction);

void setup() {
  Serial.begin(115200);
  wifiManager.setupEventHandling();
  wifiManager.tryConnectStoredWifi();
    
  if (!wifiManager.isConnected()) {
    wifiManager.initiateProvisioning(service_name, pop);
  }

  timeManager.update();
  // Attempt to pull the schedule from the server periodically
  apiManager.pullScheduleFromServer([&](String scheduleData) {
    scheduleManager.saveSchedule(scheduleData);
  });
}

void loop() {
  buttonHandler.update(); 

  static unsigned long lastCheck = 0;
  const long checkInterval = 60000; // 60 seconds
  unsigned long currentMillis = millis();


  if (currentMillis - lastCheck >= checkInterval) {
    lastCheck = currentMillis;

    if (!wifiManager.isConnected()) {
      Serial.println("Attempting to reconnect to WiFi...");
      wifiManager.attemptReconnect();
    } else {
      timeManager.update();
      // Attempt to pull the schedule from the server periodically
      apiManager.pullScheduleFromServer([&](String scheduleData) {
        scheduleManager.saveSchedule(scheduleData);
      });
    }

    // check schedule and dispense
    checkAndHandleDispensing();
  }
}

void checkAndHandleDispensing() {
    Serial.println(F("Entering checkAndHandleDispensing"));
    if (!timeManager.isTimeSynced()) {
      return;
    }
    // Example of fetching both current time and day. Adjust based on your TimeManager implementation.
    String currentTime = timeManager.getCurrentTime(); // Assuming format "HH:MM"
    int currentDayOfWeek = timeManager.getCurrentDayOfWeek(); // Assuming this returns an integer representing the day of the week.
    
    if (currentTime.isEmpty()) {
        Serial.println(F("Failed to get current time."));
        return;
    }

    Serial.print(F("Current Time: "));
    Serial.println(currentTime);

    // Assuming isScheduledTime now also internally checks if it's a scheduled day for simplicity.
    if (scheduleManager.isScheduledTime(currentTime, currentDayOfWeek)) {
        ledController.turnOn();
        Serial.println(F("LED turned ON based on schedule."));
    } else {
        // ledController.turnOff();
        // Serial.println(F("LED turned OFF as it's not a scheduled time."));
    }
}
