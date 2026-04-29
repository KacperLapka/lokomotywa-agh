#include "utils.hpp"
#include "Arduino.h"

void debugPrint(const char* message) {
#ifdef DEBUG
    Serial.println(message);
    Serial.print("\n");
#endif
}
