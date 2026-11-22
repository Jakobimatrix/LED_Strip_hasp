/**
 * @file Sensors.hpp
 * @brief Simple RTOS task for measuring with attached sensor hardware.
 *
 * This header declares a small helper task used to measure and report
 * various system metrics at a fixed interval. It provides a `TaskSensors` subclass of
 * `task::Task` which implements the `setup()`/`shutdown()` hooks and a
 * FreeRTOS-compatible `ledBlinkTask` entry function used as the task
 * entry point.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <array>

#include <Hardware/BoardHeader.hpp>
#include <Hardware/OneWireDevices.hpp>

#include <Task/Task.hpp>

namespace task {
/**
 * @brief FreeRTOS task entry function used by `TaskSensors`.
 *
 * @param pvParameters Pointer passed to the task on creation (may be `nullptr`).
 */
void sensorsTask(void* pvParameters);

enum class InstalledSensors : uint8_t {
  TEMPERATURE,
  DISTANCE,
};


constexpr std::string_view toString(InstalledSensors sensor) {
  switch (sensor) {
    case InstalledSensors::TEMPERATURE:
      return "Temperature";
    case InstalledSensors::DISTANCE:
      return "Distance";
    default:
      return "Unknown";
  }
}

struct SensorInfo {
  InstalledSensors sensor;
  TickType_t measurementInterval{pdMS_TO_TICKS(1000)};
  TickType_t lastMeasurementTime{0};

  TickType_t getTimeUntilNextMeasurement(TickType_t now) const;
};

class TaskSensors : public Task {
  friend void sensorsTask(void* pvParameters);

  /** @brief Stack depth (bytes) allocated for the sensors task. */
  constexpr static uint32_t STACK_DEPTH{2048};

  // even if a measurement is due, we dont want to sleep less than this time,
  // because otherwise the task might never give other tasks a chance to run.
  constexpr static TickType_t MINIMAL_TASK_SLEEP_TIME{pdMS_TO_TICKS(50)};

  /**
   * @brief Perform one-time initialization for the sensors task.
   *
   * Configure the sensor GPIOs and any state required before the task loop
   * begins.
   *
   * @return `true` on successful setup, `false` on failure.
   */
  [[nodiscard]] bool setup() override;


  /**
   * @brief Return the index of the enum corresponding to m_sensorInfoArray.
   * @return The index of the enum corresponding to m_sensorInfoArray.
   */
  constexpr static size_t IDX(InstalledSensors sensor) {
    switch (sensor) {
      case InstalledSensors::TEMPERATURE:
        return 0;
      case InstalledSensors::DISTANCE:
        return 1;
    }
    return NUM_SENSORS;
  }

  static constexpr size_t NUM_SENSORS{2};


  std::array<SensorInfo, NUM_SENSORS> m_sensorInfoArray{
    {SensorInfo{.sensor              = InstalledSensors::TEMPERATURE,
                .measurementInterval = pdMS_TO_TICKS(5000),
                .lastMeasurementTime = 0},
     SensorInfo{.sensor              = InstalledSensors::DISTANCE,
                .measurementInterval = pdMS_TO_TICKS(1000),
                .lastMeasurementTime = 0}}};

 public:
  static constexpr size_t NUM_TEMPERATURE_SENSORS{1};

 private:
  std::array<float, NUM_TEMPERATURE_SENSORS> m_last_temperatureMeasurements;
  float m_last_distanceMeasurement{NAN};

  static constexpr uint8_t ONE_WIRE_PIN = PINOUT_GPIO17;
  static constexpr int ONE_WIRE_BUS_ID  = 1;
  hw::OneWireDevices m_onwireDevices{ONE_WIRE_PIN, ONE_WIRE_BUS_ID};

  /**
   * @brief Return thesleep time until the next sensor measurement is due.
   * @return The sleep time in ticks until the next sensor measurement is due.
   */
  TickType_t getPeriodSleep();

  /**
   * @brief Return the next sensor that is due for measurement based on its configured interval.
   * If no sensor is due, returns the next sensor that will be due in the future.
   *
   * @return Pointer to the next `SensorInfo` that is due for measurement now or in the future. Never a nullptr.
   */
  SensorInfo* getNextSensorToMeasure();

  /**
   * @brief Measure a specific sensor.
   *
   * @param sensorInfo Pointer to the sensor info structure for the sensor to measure.
   * @return `true` if the measurement was successful, `false` otherwise.
   */
  bool measureSensor(SensorInfo* sensorInfo);

  /**
   * @brief Measure all temperature sensors.
   *
   * This function handles the measurement of all registered temperature
   * sensors. It is called when the temperature sensor is due for measurement.
   * @return `true` if the measurement was successful, `false` otherwise.
   */
  bool measureTemperatureSensors();

  /**
   * @brief Act on the result of a temperature measurement.
   *
   * Validate measurements
   * if measurement was successful
   *  - Update sensorInfo lastMeasurementTime
   *  - Store the measured temperatures in m_last_temperatureMeasurements
   *
   * @param sensorInfo Pointer to the sensor info structure for the temperature sensor.
   * @param temperatures Array containing the measured temperatures from the sensors.
   * @return `true` if the result was processed successfully, `false` otherwise.
   */
  bool actOnTemperatureMeasurementResult(SensorInfo* sensorInfo,
                                         std::array<float, NUM_TEMPERATURE_SENSORS> temperatures);

  /**
   * @brief Measure the distance sensor.
   *
   * This function handles the measurement of the distance sensor.
   * It is called when the distance sensor is due for measurement.
   * @return `true` if the measurement was successful, `false` otherwise.
   */
  bool measureDistanceSensor();

  /**
   * @brief Act on the result of a distance measurement.
   *
   * This function processes the result of a distance measurement and performs
   * any necessary actions based on the measured value.
   *
   * @param sensorInfo Pointer to the sensor info structure for the distance sensor.
   * @param distance The measured distance value.
   * @return `true` if the result was processed successfully, `false` otherwise.
   */
  bool actOnDistanceMeasurementResult(SensorInfo* sensorInfo, float distance);


 public:
  /**
   * @brief Release resources and perform clean shutdown for the task.
   */
  void shutdown() override;

  /**
   * @brief Get the last measured temperatures from the temperature sensors.
   * @return A vector containing the last measured temperatures.
   */
  std::array<float, NUM_TEMPERATURE_SENSORS> getLastTemperatureMeasurements() const;

  /**
   * @brief Get the last measured distance from the distance sensor.
   * @return The last measured distance value.
   */
  float getLastDistanceMeasurement() const;

  /**
   * @brief Get the last measurement time for the distance sensor.
   * @return The last measurement time in ticks for the distance sensor.
   */
  TickType_t getLastDistanceMeasurementTime() const;

  /**
   * @brief Get the last measurement time for the temperature sensors.
   * @return The last measurement time in ticks for the temperature sensors.
   */
  TickType_t getLastTemperatureMeasurementTime() const;

  /**
   * @brief Set the measurement interval for the temperature sensors.
   * @param interval The measurement interval in ticks for the temperature sensors.
   */
  void setTemperatureMeasurementInterval(TickType_t interval);

  /**
   * @brief Set the measurement interval for the distance sensor.
   * @param interval The measurement interval in ticks for the distance sensor.
   */
  void setDistanceMeasurementInterval(TickType_t interval);
};

}  // namespace task
