#include <Task/Task.hpp>
#include <Globals.hpp>

namespace task {


Task::Task() = default;

Task::~Task() {
  if (isRunning()) {
    if (!stop()) {
      glob::dbgTaskLogger.log(dbg::LEVEL::ERROR,
                              dbg::TOPIC::TASK,
                              "Attempted to stop task ",
                              name.c_str(),
                              " failed.");
    }
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

bool Task::stop() {
  if (!isRunning()) {
    glob::dbgTaskLogger.log(dbg::LEVEL::WARN,
                            dbg::TOPIC::TASK,
                            "Attempted to stop task ",
                            name.c_str(),
                            " that is not running");
    return true;
  }
  shutdown();
  vTaskDelete(handle);

  const bool is_running = isRunning();
  if (is_running) {
    glob::dbgTaskLogger.log(dbg::LEVEL::ERROR,
                            dbg::TOPIC::TASK,
                            "Failed to delete task ",
                            name.c_str(),
                            ": still running");
  } else {
    glob::dbgTaskLogger.log(dbg::LEVEL::INFO,
                            dbg::TOPIC::TASK,
                            "Task ",
                            name.c_str(),
                            ": deleted successfully");
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
  return ret;
}

}  // namespace task
