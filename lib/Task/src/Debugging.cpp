#include <Task/Debugging.hpp>
#include <Globals.hpp>
#include <Task/TaskPriority.hpp>

#include <Debugger/Serializer.hpp>

namespace task {


void debuggerTask(void* pvParameters) {
  auto* self = static_cast<TaskDebugging*>(pvParameters);

  auto serializer = dbg::Serializer(glob::sharedQueue,
                                    glob::debugWiFiProvQueue,
                                    glob::debugWiFiQueue,
                                    glob::debugMqttQueue,
                                    glob::debugLedQueue);

  while (!self->isStopRequested()) {
    self->logStackHighWaterMark(dbg::TOPIC::PERF);
    self->sleepFixedDelay(pdMS_TO_TICKS(20));
    while (serializer.printNext()) {
      self->sleepFixedDelay(pdMS_TO_TICKS(5));
    }
  }

  self->shutdown();
}

bool TaskDebugging::setup() {
  return pdPASS == createTask(debuggerTask,
                              "Printer",
                              TaskDebugging::STACK_DEPTH,
                              this,
                              task::DEBUG_TASK_PRIORITY,
                              Task::NonRealTimeCore);
}

void TaskDebugging::shutdown() { deleteHandle(); }

}  // namespace task
