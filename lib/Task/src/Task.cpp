#include <Task/Task.hpp>
#include <Globals.hpp>

#include <Arduino.h>

namespace task {


Task::Task() = default;

Task::~Task() {
  int tries{5};
  while (isRunning() && tries-- > 0) {
    stop();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

bool Task::isRunning() const {
  if (handle == nullptr) {
    return false;
  }
  return eTaskGetState(handle) == eRunning;
}

bool Task::start() {
  if (isRunning()) {
    glob::dbgTaskLogger.log(dbg::LEVEL::WARN,
                            dbg::TOPIC::TASK,
                            "Attempted to start task ",
                            name.c_str(),
                            " that is already running");
    return true;
  }
  return setup();
}

void Task::stop() {
  stopRequested.store(true);
  if (!isRunning()) {
    glob::dbgTaskLogger.log(dbg::LEVEL::WARN,
                            dbg::TOPIC::TASK,
                            "Attempted to stop task ",
                            name.c_str(),
                            " that is not running");
    return;
  }
  glob::dbgTaskLogger.log(dbg::LEVEL::INFO,
                          dbg::TOPIC::TASK,
                          "Stop requested for task ",
                          name.c_str(),
                          " send");
}

void Task::deleteHandle() {
  if (handle != nullptr) {
    vTaskDelete(handle);
    handle = nullptr;
    glob::dbgTaskLogger.log(
      dbg::LEVEL::INFO, dbg::TOPIC::TASK, "Deleted task handle for task ", name.c_str());
  } else {
    glob::dbgTaskLogger.log(dbg::LEVEL::WARN,
                            dbg::TOPIC::TASK,
                            "Attempted to delete task handle for task ",
                            name.c_str(),
                            " but handle was already null");
  }
}

bool Task::isStopRequested() const { return stopRequested.load(); }

BaseType_t Task::createTask(TaskFunction_t pxTaskCode,
                            const char* const pcName,
                            const uint32_t ulStackDepth,
                            void* const pvParameters,
                            UBaseType_t uxPriority,
                            const BaseType_t xCoreID) {
  stackDepth = ulStackDepth;

  const BaseType_t ret = xTaskCreatePinnedToCore(
    pxTaskCode, pcName, ulStackDepth, pvParameters, uxPriority, &handle, xCoreID);

  if (ret != pdFREERTOS_ERRNO_NONE) {
    glob::dbgTaskLogger.log(dbg::LEVEL::ERROR,
                            dbg::TOPIC::TASK,
                            "Failed to create task ",
                            pcName,
                            ": ",
                            ret);
  } else {
    glob::dbgTaskLogger.log(dbg::LEVEL::INFO,
                            dbg::TOPIC::TASK,
                            "Created task ",
                            pcName,
                            " with priority ",
                            uxPriority,
                            " on core ",
                            xCoreID);
    const std::string_view name_view{pcName};
    name.assign(name_view.substr(0, name.capacity()));
  }
  cycle_start_tick = xTaskGetTickCount();
  return ret;
}

void Task::sleepFixedRate(TickType_t period) {
  if (stopRequested.load()) {
    glob::dbgTaskLogger.log(dbg::LEVEL::INFO,
                            dbg::TOPIC::TASK,
                            "Stop requested for task ",
                            name.c_str(),
                            ". Exiting sleep loop.");
    return;
  }
  const TickType_t current_time{xTaskGetTickCount()};
  const TickType_t elapsedTime{current_time - cycle_start_tick};


  if (elapsedTime >= period) {
    glob::dbgTaskLogger.log(dbg::LEVEL::WARN,
                            dbg::TOPIC::TASK,
                            "Task ",
                            name.c_str(),
                            " loop time exceeded target cycle time! Elapsed: ",
                            pdTICKS_TO_MS(elapsedTime),
                            " ms, Target: ",
                            pdTICKS_TO_MS(period),
                            " ms");
  }
  vTaskDelayUntil(&cycle_start_tick, period);
}

void Task::sleepFixedDelay(TickType_t delay) {
  if (stopRequested.load()) {
    glob::dbgTaskLogger.log(dbg::LEVEL::INFO,
                            dbg::TOPIC::TASK,
                            "Stop requested for task ",
                            name.c_str(),
                            ". Exiting sleep loop.");
    return;
  }
  vTaskDelay(delay);
}

void Task::logStackHighWaterMark(dbg::TOPIC topic) {
  glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                         topic,
                         "Stack size: ",
                         getStackDepth(),
                         " bytes, High Water Mark: ",
                         getStackHighWaterMark(),
                         " bytes");
}

}  // namespace task
