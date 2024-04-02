#include "WiFiManager.h"

WiFiManager::WiFiManager(TimeManager* tm) : timeManager(tm) {}

void WiFiManager::connect(const char* ssid, const char* password) {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Establishing connection to WiFi...");
  }

  Serial.println("Connected to WiFi.");
}

bool WiFiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::attemptReconnect() {
  if (!isConnected()) {
    Serial.println("Reconnecting to WiFi...");
    WiFi.reconnect();
    delay(5000);

    if (isConnected()) {
      Serial.println("Reconnected to WiFi.");
    } else {
      Serial.println("Failed to reconnect to WiFi.");
    }
  }
}

void WiFiManager::saveConfiguration(const char* ssid, const char* password){
    Preferences prefs;
    prefs.begin("wifi", false); // Open with write access
    prefs.putString("ssid", ssid);
    prefs.putString("password", password);
    prefs.end();
}

void WiFiManager::forgetConfiguration() {
    preferences.begin("wifi", false); // "wifi" is the namespace for our WiFi settings
    preferences.clear(); // Clear all preferences within the "wifi" namespace
    preferences.end(); // Always call end to close the Preferences
    Serial.println("WiFi configuration forgotten.");
}

void WiFiManager::tryConnectStoredWifi() {
    preferences.begin("wifi", true); // Open the Preferences with read-only access
    String ssid = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");
    preferences.end();

    if (ssid != "" && password != "") {
        Serial.println("Trying to connect with stored credentials...");
        connect(ssid.c_str(), password.c_str());
    } else {
        Serial.println("No stored WiFi credentials found.");
    }
}

void WiFiManager::initiateProvisioning(const char* service_name, const char* pop) {
  Serial.println("Initiating WiFi provisioning...");

  WiFi.mode(WIFI_STA);

  WiFiProv.beginProvision(WIFI_PROV_SCHEME_BLE, WIFI_PROV_SCHEME_HANDLER_FREE_BTDM, WIFI_PROV_SECURITY_1, pop, service_name, NULL, NULL, true);
  while (!isConnected()) {
    delay(1000);
    Serial.println("Waiting for provisioning...");
  }

  if (timeManager && !timeManager->isTimeSynced()) {
      timeManager->begin();
      timeManager->update();
  }
  
  Serial.println("Provisioning done. WiFi connected.");
}

void WiFiManager::setupEventHandling() {
  WiFi.onEvent(SysProvEvent);
}

void WiFiManager::SysProvEvent(arduino_event_t *sys_event) {
    switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("\nConnected IP address : ");
        Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
        // Modify LED logic as needed or call another method within WiFiManager

        break;
    case ARDUINO_EVENT_PROV_CRED_RECV: {
        Serial.println("\nReceived Wi-Fi credentials");
        saveConfiguration((const char *) sys_event->event_info.prov_cred_recv.ssid, (char const *) sys_event->event_info.prov_cred_recv.password);
        Serial.print("\tSSID : ");
        Serial.println((const char *) sys_event->event_info.prov_cred_recv.ssid);
        Serial.print("\tPassword : ");
        Serial.println((char const *) sys_event->event_info.prov_cred_recv.password);
        break;
    }
    case ARDUINO_EVENT_PROV_START:
        Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app");
        // Handle provisioning start event
        break;
    // Handle other events similarly
    default:
        break;
    }
}