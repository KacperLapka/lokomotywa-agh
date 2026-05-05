#include "engine.hpp"
#include "sensors.hpp"
#include "utils.hpp"
#include "constants.hpp"
#include <iostream>

#if defined(ESP_PLATFORM)
#include "Arduino.h"
#endif

void setupSerial() {
  // set up serial.
  Serial.begin(115200);
  Serial.setDebugOutput(0);
  esp_log_level_set("*", ESP_LOG_NONE);
  Serial.print("Lokomotywa AGH - Setup\n");
}

// Przygotowanie do pracy
void setup() {
  setupSerial();

  debugPrint("Hello from Lokomotywa AGH script!");
  debugPrint("Preparing stuff...");
  setupInput();
  setupEngine();

  debugPrint("Preparation done. Proceed to loop...n");
  engineStart();
}

// The main loop.
void loop() {
  if (isTrainDetected()) {
    debugPrint("Train detected! Stopping train");
    engineStop();
    delay(5000);
    engineStart();
    delay(2000);
  }
  delay(10);
}
