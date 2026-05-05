#include "engine.hpp"
#include "constants.hpp"
#include "Arduino.h"

void setSpeed(int speed) {
  if (speed < 0) speed = 0;
  if (speed > SPEED_CAP) speed = SPEED_CAP;
  analogWrite(ENGINE_A_IN1, speed);
}

void engineStop() {
  if (GRACEFUL_STOP) {
    int currentSpeed = SPEED_CAP;
    while (currentSpeed > 0) {
      currentSpeed -= SPEED_CHANGE_FACTOR;
      setSpeed(currentSpeed - SPEED_CHANGE_FACTOR);
      delay(100);
    }
  }else {
    setSpeed(0);
  }
}

void engineStart() {
  if (GRACEFUL_START) {
    setSpeed(INITAL_SPEED);
    int currentSpeed = INITAL_SPEED;
    while (currentSpeed < SPEED_CAP) {
      currentSpeed += SPEED_CHANGE_FACTOR;
      setSpeed(currentSpeed);
      delay(100);
    }
  }else {
    setSpeed(SPEED_CAP);
  }
}

void setupEngine() {
  // Stop first engine
  digitalWrite(ENGINE_A_IN1, HIGH);
  digitalWrite(ENGINE_A_IN2, HIGH);

  // Unused engine always disabled
  digitalWrite(ENGINE_B_IN1, HIGH);
  digitalWrite(ENGINE_B_IN2, HIGH);
}