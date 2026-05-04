idf_component_register(SRCS "main.cpp" "engine.cpp" "sensors.cpp" "utils.cpp" "vl53l0x.c"
                       INCLUDE_DIRS "."
                       REQUIRES "driver" "esp_timer")
