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

  handleButton(currentMillis);

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

void handleButton(unsigned long currentMillis) {
  static unsigned long buttonPressStartTime = 0; // When the current button press started
  bool currentButtonState = digitalRead(BUTTON_PIN);

  // Check for button press (rising edge)
  if (currentButtonState && !buttonPreviouslyPressed) {
    buttonPreviouslyPressed = true;
    buttonPressStartTime = currentMillis;
  }
  
  // Check for button release (falling edge)
  if (!currentButtonState && buttonPreviouslyPressed) {
    buttonPreviouslyPressed = false;
    unsigned long pressDuration = currentMillis - buttonPressStartTime;

    if (pressDuration >= longPressTime) {
      Serial.println("Long press detected, reinitiating provisioning");
      initiateBLEProvisioning();
    } else {
      // Short press logic
      if (ledState) {
        digitalWrite(LED_PIN, LOW);
        ledState = false;
        Serial.println("Pill dispensed.");
      } else {
        // Optionally, handle a short press when the LED is off, if needed
      }
    }
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