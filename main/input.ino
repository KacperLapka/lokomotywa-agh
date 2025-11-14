#ifndef INPUTINO
#define INPUTINO
#include "Arduino.h"
#include "utils.ino"
#include <Adafruit_VL53L0X.h>
#include <Wire.h>
#include <iostream>

Adafruit_VL53L0X DistanceSensor[NUM_STATIONS];
bool previous[NUM_STATIONS];
void SensorReboot() {
  for (int i = 0; i < NUM_STATIONS; i++) {
    pinMode(DistanceSensorShut[i], OUTPUT);
    digitalWrite(DistanceSensorShut[i], LOW);
  }

  delay(50); // give them some time to go down
    
  DebugPrint("All the distance Sensors are disabled now.");

  for (int i = 0; i < NUM_STATIONS; i++) {
    char buffer[32];
    std::sprintf(buffer, "Booting sensor %d...", i);
    DebugPrint(buffer);

    digitalWrite(DistanceSensorShut[i], HIGH);
    delay(100);
    DebugPrint("The Sensor should be up now.");

    if (!DistanceSensor[i].begin(DISTANCE_SENSOR_ADDRESS[i],true , &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY)) {
      DebugPrint("Failed to boot VL53L0X");
      while (1);
    }
    delay(100);
  }
}

void SensorReboot(int sensor_num) {
  pinMode(DistanceSensorShut[sensor_num], OUTPUT);
  digitalWrite(DistanceSensorShut[sensor_num], LOW);

  delay(10); // give them some time to go down

  char buffer[32];
  std::sprintf(buffer, "Booting sensor %d...", sensor_num);
  DebugPrint(buffer);

  digitalWrite(DistanceSensorShut[sensor_num], HIGH);
  delay(10);
  DebugPrint("The Sensor should be up now.");

  if (!DistanceSensor[sensor_num].begin(DISTANCE_SENSOR_ADDRESS[sensor_num],true , &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY)) {
    DebugPrint("Failed to boot VL53L0X");
    while (1);
  }
  delay(10);
}

void SetupInput() {
    for (int i = 0; i < NUM_STATIONS; i++) {
        previous[i] = false;
    }

    DebugPrint("Initializing I2C distance sensors...");
    Wire.setPins(DistanceSensorSDA, DistanceSensorSCL);
    if (!Wire.begin()) {
        DebugPrint("Failed initialize I2C on specified ports (ESP will be trapped here)!");
        while (1);
    }

    Wire.setClock(400);

    DebugPrint("I2C initialized.");

    /*
    for (int i = 0; i < NUM_STATIONS; i++) {
        DistanceSensor[i] = Adafruit_VL53L0X();
    }
    */

    SensorReboot();

    DebugPrint("Both VL53L0X Ready now.");
}


// IsTrainDetected returns true whenever a train is detected by our sensor.
// This also saves last result and returns true ONLY, when the state changed.
bool IsTrainDetected() {
    int distance[NUM_STATIONS];
    bool result = false;
    VL53L0X_RangingMeasurementData_t measure;
    for (int i = 0; i < NUM_STATIONS; i++) {
        char buffer[256];
        char state;
        state = DistanceSensor[i].rangingTest(&measure, false);
        if(state != 0){
          printf("Error getting sensor state (restarting sensors): %d", state);
          char err[128];  
          VL53L0X_get_pal_error_string(state, err);
          std::sprintf(buffer, "Error getting sensor%d state (restarting sensors): %s", i, err);
          DebugPrint(buffer);
          SensorReboot(i);
        }
        distance[i] = measure.RangeMilliMeter;

        std::sprintf(buffer, "Distance%d: %d mm", i, distance[i]);
        DebugPrint(buffer);
        bool detection = (distance[i] <= TrainDetectionThreshold) && distance[i];
        result |= detection && !previous[i];
        previous[i] = detection;
    }

    return result;
}

// ShouldResumeTrain returns true when a reobot finishes its job.
// It is designed to be able to occupy the main thread till true. If false returned this will be recalled till true.
bool ShouldResumeTrain() {
    delay(5000);
    return true;
}

#endif
