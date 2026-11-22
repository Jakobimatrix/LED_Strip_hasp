#include <Task/Sensors.hpp>

#include <Task/TaskPriority.hpp>
#include <Globals.hpp>
#include <Hardware/TemperatureSensor_DS18B20.hpp>

#include <Arduino.h>



namespace task {

TickType_t SensorInfo::getTimeUntilNextMeasurement(TickType_t now) const {
  const TickType_t elapsed = now - lastMeasurementTime;
  return elapsed >= measurementInterval ? 0 : measurementInterval - elapsed;
}

void sensorsTask(void* pvParameters) {
  auto* self = static_cast<TaskSensors*>(pvParameters);

  self->startCycleTime();

  while (!self->isStopRequested()) {
    SensorInfo* nextSensor = self->getNextSensorToMeasure();
    const TickType_t now{xTaskGetTickCount()};
    if (nextSensor->getTimeUntilNextMeasurement(now) <= TaskSensors::MINIMAL_TASK_SLEEP_TIME) {
      if (self->measureSensor(nextSensor)) {
        nextSensor->lastMeasurementTime = now;
      } else {
        glob::dbgMeasurementsLogger.log(dbg::LEVEL::ERROR,
                                        dbg::TOPIC::MEASURE,
                                        "Sensor measurement failed of sensor: ",
                                        toString(nextSensor->sensor));
      }
    }
    self->logStackHighWaterMark(dbg::TOPIC::MEASURE);
    self->sleepFixedRate(self->getPeriodSleep());
  }

  self->shutdown();
}


TickType_t TaskSensors::getPeriodSleep() {
  SensorInfo* nextSensor = getNextSensorToMeasure();
  const TickType_t now{xTaskGetTickCount()};
  TickType_t timeUntilNextMeasurement = nextSensor->getTimeUntilNextMeasurement(now);
  if (timeUntilNextMeasurement < MINIMAL_TASK_SLEEP_TIME) {
    return MINIMAL_TASK_SLEEP_TIME;
  }
  return timeUntilNextMeasurement;
}

bool TaskSensors::setup() {
  name = "Sensors";

  m_onwireDevices.add<hw::TemperatureSensor_DS18B20>(0);
  if (!m_onwireDevices.discover()) {
    Serial.println("Not all OneWire devices found!");
  }

  return pdPASS == createTask(sensorsTask,
                              "Sensors Task",
                              TaskSensors::STACK_DEPTH,
                              this,
                              task::LED_TASK_PRIORITY,
                              Task::RealTimeCore);
}

SensorInfo* TaskSensors::getNextSensorToMeasure() {
  const TickType_t now{xTaskGetTickCount()};
  SensorInfo* nextSensor = &m_sensorInfoArray[0];
  TickType_t minTimeUntilNextMeasurement = nextSensor->getTimeUntilNextMeasurement(now);

  for (SensorInfo& sensorInfo : m_sensorInfoArray) {
    TickType_t timeUntilNextMeasurement = sensorInfo.getTimeUntilNextMeasurement(now);
    if (timeUntilNextMeasurement < minTimeUntilNextMeasurement) {
      minTimeUntilNextMeasurement = timeUntilNextMeasurement;
      nextSensor                  = &sensorInfo;
    }
  }

  return nextSensor;
}

bool TaskSensors::measureSensor(SensorInfo* sensorInfo) {
  switch (sensorInfo->sensor) {
    case InstalledSensors::TEMPERATURE:
      return measureTemperatureSensors();
    case InstalledSensors::DISTANCE:
      return measureDistanceSensor();
    default:
      glob::dbgMeasurementsLogger.log(
        dbg::LEVEL::ERROR, dbg::TOPIC::MEASURE, "Unknown sensor type");
      return false;
  }
}

bool TaskSensors::measureTemperatureSensors() {
  // Placeholder for actual temperature measurement logic
  std::array<float, NUM_TEMPERATURE_SENSORS> measuredTemperatures{NAN};
  m_onwireDevices.startMeasurement();
  // Wait for DS18B20 conversion
  sleepFixedRate(pdMS_TO_TICKS(750));
  const auto temp0{m_onwireDevices.read<hw::TemperatureSensor_DS18B20>(0)};
  if (temp0.valid()) {
    measuredTemperatures[0] = temp0.temperature;
    return actOnTemperatureMeasurementResult(
      &m_sensorInfoArray[IDX(InstalledSensors::TEMPERATURE)], measuredTemperatures);
  }
  glob::dbgMeasurementsLogger.log(
    dbg::LEVEL::ERROR,
    dbg::TOPIC::MEASURE,
    "Temperature measurement failed for sensor 0: ",
    hw::TemperatureSensor_DS18B20::toString(temp0.error));
  return false;
}

bool TaskSensors::measureDistanceSensor() {
  // Placeholder for actual distance measurement logic
  float measuredDistance = 10.0f;  // Example measurement
  return actOnDistanceMeasurementResult(
    &m_sensorInfoArray[IDX(InstalledSensors::DISTANCE)], measuredDistance);
}

bool TaskSensors::actOnTemperatureMeasurementResult(
  SensorInfo* sensorInfo, std::array<float, NUM_TEMPERATURE_SENSORS> temperatures) {
  for (const float temp : temperatures) {
    if (std::isnan(temp)) {
      glob::dbgMeasurementsLogger.log(dbg::LEVEL::ERROR,
                                      dbg::TOPIC::MEASURE,
                                      "Temperature measurement has nan value");
      return false;
    }
  }

  m_last_temperatureMeasurements = temperatures;
  return true;
}

bool TaskSensors::actOnDistanceMeasurementResult(SensorInfo* sensorInfo, float distance) {
  if (std::isnan(distance)) {
    glob::dbgMeasurementsLogger.log(dbg::LEVEL::ERROR,
                                    dbg::TOPIC::MEASURE,
                                    "Distance measurement has nan value");
    return false;
  }
  m_last_distanceMeasurement = distance;
  return true;
}


void TaskSensors::shutdown() { deleteHandle(); }


std::array<float, TaskSensors::NUM_TEMPERATURE_SENSORS> TaskSensors::getLastTemperatureMeasurements() const {
  return m_last_temperatureMeasurements;
}


float TaskSensors::getLastDistanceMeasurement() const {
  return m_last_distanceMeasurement;
}


TickType_t TaskSensors::getLastDistanceMeasurementTime() const {
  return m_sensorInfoArray[IDX(InstalledSensors::DISTANCE)].lastMeasurementTime;
}


TickType_t TaskSensors::getLastTemperatureMeasurementTime() const {
  return m_sensorInfoArray[IDX(InstalledSensors::TEMPERATURE)].lastMeasurementTime;
}


void TaskSensors::setTemperatureMeasurementInterval(TickType_t interval) {
  m_sensorInfoArray[IDX(InstalledSensors::TEMPERATURE)].measurementInterval = interval;
}


void TaskSensors::setDistanceMeasurementInterval(TickType_t interval) {
  m_sensorInfoArray[IDX(InstalledSensors::DISTANCE)].measurementInterval = interval;
}

}  // namespace task
