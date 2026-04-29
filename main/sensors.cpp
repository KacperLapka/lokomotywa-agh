#include "sensors.hpp"
#include "constants.hpp"
#include "Arduino.h"
#include "utils.hpp"
#include <Adafruit_VL53L0X.h>
#include <Wire.h>
#include <iostream>

Adafruit_VL53L0X DistanceSensor[NUM_STATIONS];
void sensorsReboot() {
  for (int i = 0; i < NUM_STATIONS; i++) {
    pinMode(DISTANCE_SENSOR_SHUT[i], OUTPUT);
    digitalWrite(DISTANCE_SENSOR_SHUT[i], LOW);
  }

  delay(50); // give them some time to go down
    
  debugPrint("All the distance Sensors are disabled now.");

  for (int i = 0; i < NUM_STATIONS; i++) {
    char buffer[32];
    std::sprintf(buffer, "Booting sensor %d...", i);
    debugPrint(buffer);

    digitalWrite(DISTANCE_SENSOR_SHUT[i], HIGH);
    delay(100);
    debugPrint("The Sensor should be up now.");

    if (!DistanceSensor[i].begin(DISTANCE_SENSOR_ADDRESS[i],true , &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY)) {
      debugPrint("Failed to boot VL53L0X");
      i--;
    }
    delay(100);
  }
}

void sensorReboot(int sensor_num) {
  pinMode(DISTANCE_SENSOR_SHUT[sensor_num], OUTPUT);
  digitalWrite(DISTANCE_SENSOR_SHUT[sensor_num], LOW);

  delay(10); // give them some time to go down

  char buffer[32];
  std::sprintf(buffer, "Booting sensor %d...", sensor_num);
  debugPrint(buffer);

  digitalWrite(DISTANCE_SENSOR_SHUT[sensor_num], HIGH);
  delay(10);
  debugPrint("The Sensor should be up now.");

  if (!DistanceSensor[sensor_num].begin(DISTANCE_SENSOR_ADDRESS[sensor_num],true , &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY)) {
    debugPrint("Failed to boot VL53L0X");
  }
  delay(10);
}

void setupInput() {
    debugPrint("Initializing I2C distance sensors...");
    Wire.setPins(DISTANCE_SENSOR_SDA, DISTANCE_SENSOR_SCL);
    if (!Wire.begin()) {
        debugPrint("Failed initialize I2C on specified ports (ESP will be trapped here)!");
        while (1);
    }

    Wire.setClock(400);

    debugPrint("I2C initialized.");

    /*
    for (int i = 0; i < NUM_STATIONS; i++) {
        DistanceSensor[i] = Adafruit_VL53L0X();
    }
    */

    sensorsReboot();

    debugPrint("Both VL53L0X Ready now.");
}


// IsTrainDetected returns true whenever a train is detected by our sensor.
// This also saves last result and returns true ONLY, when the state changed.
bool isTrainDetected() {
    int distance[NUM_STATIONS];
    VL53L0X_RangingMeasurementData_t measure;
    for (int i = 0; i < NUM_STATIONS; i++) {
        char buffer[256];
        char err_code =  DistanceSensor[i].rangingTest(&measure, false);
        if(err_code != 0){
          char err[128];  
          VL53L0X_get_pal_error_string(err_code, err);
          std::sprintf(buffer, "Error getting sensor%d state error_code: %d (restarting sensors): %s", i, err_code, err);
          debugPrint(buffer);
          sensorReboot(i);
          continue;
        }
        distance[i] = measure.RangeMilliMeter;

        std::sprintf(buffer, "Distance%d: %d mm", i, distance[i]);
        debugPrint(buffer);
        bool detection = (distance[i] <= TRAIN_DETECTION_THRESHOLD) && distance[i];
        if (detection) {
            return true;
        }
    }

    return false;
}

void setupDistanceSensors() {
  pinMode(DISTANCE_SENSOR_SDA, INPUT_PULLUP);
  pinMode(DISTANCE_SENSOR_SCL, INPUT_PULLUP);
}