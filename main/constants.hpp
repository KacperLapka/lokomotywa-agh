#pragma once
const int TRAIN_DETECTION_THRESHOLD = 70; // mm
const int SPEED_CHANGE_FACTOR = 40;
const int SPEED_CAP = 200; // [0-255]
const int STOP_TIME = 5000; // ms
const bool GRACEFUL_STOP = true; // whether to stop immediately or gradually
const bool GRACEFUL_START = true; // whether to start immediately or gradually

const int NUM_STATIONS = 4; // Number of Distance sensors to be proceeded.
const int DISTANCE_SENSOR_ADDRESS[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35};

// GPIO setup
const int ENGINE_A_IN1 = 13; //D13
const int ENGINE_A_IN2 = 12; //D12

const int ENGINE_B_IN1 = 14; //D14
const int ENGINE_B_IN2 = 27; //D27 niespodzianka, są w tej samej lini

const int DISTANCE_SENSOR_SDA = 21; // D21
const int DISTANCE_SENSOR_SCL = 22; // D22
const int DISTANCE_SENSOR_SHUT[] = {15,2,4,16,5,17}; // TODO