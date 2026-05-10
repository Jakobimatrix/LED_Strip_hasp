#include <Task/Task.hpp>

#include <Debugger/Logger.hpp>

namespace task {


Task::Task() = default;

Task::~Task() {
  if (isRunning()) {
    stop();
  }
}

bool Task::isRunning() const { return eTaskGetState(handle) == eRunning; }

bool Task::start() {
  if (isRunning()) {
    glob::dbgTaskLogger.log(LEVEL::WARN,
                            TOPIC::TASK,
                            "Attempted to start task that is already running");
    return true;
  }
  return setup();
}

bool Task::stop() {
  if (!isRunning()) {
    glob::dbgTaskLogger.log(
      LEVEL::WARN, TOPIC::TASK, "Attempted to stop task that is not running");
    return true;
  }
  shutdown();
  vTaskDelete(handle);

  const bool is_running = isRunning();
  if (is_running) {
    glob::dbgTaskLogger.log(
      LEVEL::ERROR, TOPIC::TASK, "Failed to delete task: still running");
  } else {
    glob::dbgTaskLogger.log(
      LEVEL::INFO, TOPIC::TASK, "Task deleted successfully");
  }
  return !is_running;
}

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
    glob::dbgTaskLogger.log(
      LEVEL::ERROR, TOPIC::TASK, "Failed to create task %s: %d", pcName, ret);
  } else {
    glob::dbgTaskLogger.log(
      LEVEL::INFO, TOPIC::TASK, "Created task %s with priority %u on core %d", pcName, uxPriority, xCoreID);
  }
  return ret;
}

}  // namespace task
