#ifndef WiFiManager_h
#define WiFiManager_h

#include <WiFi.h>
#include <WiFiProv.h>
#include <Preferences.h>
#include "TimeManager.h" 

class WiFiManager {
private:
  Preferences preferences;
  TimeManager* timeManager; // Pointer to TimeManager
public:
  WiFiManager(TimeManager* tm = nullptr);
  void tryConnectStoredWifi();
  void connect(const char* ssid, const char* password);
  bool isConnected();
  void attemptReconnect();
  static void saveConfiguration(const char* ssid, const char* password);
  void forgetConfiguration(); 
  void initiateProvisioning(const char* service_name, const char* pop);
  static void SysProvEvent(arduino_event_t *sys_event); // Event handler function
  void setupEventHandling(); // Set up WiFi event handling
};

#endif
