#ifndef ApiManager_h
#define ApiManager_h

#include <Arduino.h>
#include <HTTPClient.h>

class ApiManager {
public:
  ApiManager();
  void pullScheduleFromServer(std::function<void(String)> callback);  
};

#endif
