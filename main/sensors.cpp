#include "sensors.hpp"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
extern "C" {
    #include "vl53l0x.h" // Replace with the actual name of your library header
}
#include "Arduino.h"
#include "constants.hpp"

vl53l0x_t *sensors[NUM_STATIONS];
i2c_master_bus_handle_t bus_handle;

// int8_t DISTANCE_SENSOR_SHUT[NUM_SENSORS] = {SENSOR1_XSHUT, SENSOR2_XSHUT, SENSOR3_XSHUT, SENSOR4_XSHUT};
// uint8_t DISTANCE_SENSOR_ADDRESS[NUM_SENSORS] = {SENSOR1_ADDR, SENSOR2_ADDR, SENSOR3_ADDR, SENSOR4_ADDR};

void setupSensors()
{
  for (int i = 0; i < NUM_STATIONS; i++)
  {
    gpio_reset_pin((gpio_num_t) DISTANCE_SENSOR_SHUT[i]);
    gpio_set_direction((gpio_num_t) DISTANCE_SENSOR_SHUT[i], GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t) DISTANCE_SENSOR_SHUT[i], 0); // Keep in reset
    delay(10);
  }

  delay(100);

  // Initialize each sensor one by one
  for (int i = 0; i < NUM_STATIONS; i++)
  {
    // Create sensor with shared bus and default address (0x29)
    sensors[i] = vl53l0x_config_with_bus(
        bus_handle,
        DISTANCE_SENSOR_SHUT[i],
        0x29, // All start with default address
        1     // Use 2.8V I/O mode
    );

    if (!sensors[i])
    {
      continue;
    }

    // Release this sensor from reset
    gpio_set_level((gpio_num_t) DISTANCE_SENSOR_SHUT[i], 1);
    delay(10); // Wait for sensor to boot

    // Initialize the sensor
    const char *err = vl53l0x_init(sensors[i]);
    if (err)
    {
      vl53l0x_end(sensors[i]);
      sensors[i] = NULL;
      continue;
    }

    // Change to unique I2C address
    vl53l0x_setAddress(sensors[i], DISTANCE_SENSOR_ADDRESS[i]);
    // Optional: Configure timing budget
    vl53l0x_setMeasurementTimingBudget(sensors[i], 40000); // 40ms
  }

  for (int i = 0; i < NUM_STATIONS; i++)
  {
    if (sensors[i])
    {
      vl53l0x_startContinuous(sensors[i], 100); // 100ms interval
    }
  }
}

void sensorsReboot()
{
  for (int i = 0; i < NUM_STATIONS; i++)
  {
    if (sensors[i])
    {
      vl53l0x_stopContinuous(sensors[i]);
      vl53l0x_end(sensors[i]); // Does NOT delete the shared bus
    }
  }
  i2c_del_master_bus(bus_handle);

  i2c_master_bus_config_t bus_config = {
      .i2c_port = 0,
      .sda_io_num = (gpio_num_t) DISTANCE_SENSOR_SDA,
      .scl_io_num = (gpio_num_t) DISTANCE_SENSOR_SCL,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,           
      .trans_queue_depth = 0,
      .flags = {
          .enable_internal_pullup = true
      },
  };

  i2c_new_master_bus(&bus_config, &bus_handle);

  setupSensors();
}

void sensorReboot(int sensor_num)
{
  gpio_reset_pin((gpio_num_t) DISTANCE_SENSOR_SHUT[sensor_num]);
  gpio_set_direction((gpio_num_t) DISTANCE_SENSOR_SHUT[sensor_num], GPIO_MODE_OUTPUT);
  gpio_set_level((gpio_num_t) DISTANCE_SENSOR_SHUT[sensor_num], 0); // Keep in reset

  sensors[sensor_num] = vl53l0x_config_with_bus(
      bus_handle,
      DISTANCE_SENSOR_SHUT[sensor_num],
      0x29, // All start with default address
      1     // Use 2.8V I/O mode
  );
  gpio_set_level((gpio_num_t) DISTANCE_SENSOR_SHUT[sensor_num], 1);

  const char *err = vl53l0x_init(sensors[sensor_num]);
  if (err)
  {
    vl53l0x_end(sensors[sensor_num]);
    sensors[sensor_num] = NULL;
  }

  vl53l0x_setAddress(sensors[sensor_num], DISTANCE_SENSOR_ADDRESS[sensor_num]);
  vl53l0x_setMeasurementTimingBudget(sensors[sensor_num], 40000); // 40ms

  vl53l0x_startContinuous(sensors[sensor_num], 100); // 100ms interval
}

void setupInput()
{
  i2c_master_bus_config_t bus_config = {
      .i2c_port = 0,
      .sda_io_num = (gpio_num_t) DISTANCE_SENSOR_SDA,
      .scl_io_num = (gpio_num_t) DISTANCE_SENSOR_SCL,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,           
      .trans_queue_depth = 0,
      .flags = {
          .enable_internal_pullup = true
      },
  };

  esp_err_t ret = i2c_new_master_bus(&bus_config, &bus_handle);
  if (ret != ESP_OK)
  {
    return;
  }

  sensorsReboot();
}

// IsTrainDetected returns true whenever a train is detected by our sensor.
// This also saves last result and returns true ONLY, when the state changed.
bool isTrainDetected()
{
  int min_mesure = 0xffff;
  for (int i = 0; i < NUM_STATIONS; i++)
  {
    if (!sensors[i])
    {
      // debugPrint("Sensor " + (i + 1) + " unaviable");
      continue;
    }

    uint16_t range_mm = vl53l0x_readRangeContinuousMillimeters(sensors[i]);
    if (range_mm < min_mesure)
    {
      min_mesure = range_mm;
    }
    if (vl53l0x_timeoutOccurred(sensors[i]))
    {
      // debugPrint("Sensor " + (i + 1) + " Timeout");
    }
    else if (vl53l0x_i2cFail(sensors[i]))
    {
      // debugPrint("Sensor: " + (i + 1) + " I2C error");
    }
    else
    {
      // debugPrint("Sensor: " + range_mm + " mm");
    }
    if (min_mesure < TRAIN_DETECTION_THRESHOLD)
    {
      return true;
    }
  }
  return false;
}