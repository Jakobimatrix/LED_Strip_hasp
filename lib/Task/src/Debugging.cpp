#include <Task/Debugging.hpp>
#include <Globals.hpp>

#include <Debugger/Serializer.hpp>

namespace task {


void debuggerTask(void* pvParameters) {
  auto* self = static_cast<TaskDebugging*>(pvParameters);

  auto serializer = dbg::Serializer(
    glob::debugTaskQueue, glob::debugWiFiQueue, glob::debugMqttQueue, glob::debugLedQueue);

  while (!self->isStopRequested()) {
    vTaskDelay(pdMS_TO_TICKS(20));
    while (serializer.printNext()) {
      vTaskDelay(pdMS_TO_TICKS(5));
    }
  }

  self->shutdown();
}

bool TaskDebugging::setup() {
  Serial.begin(9600);
  return pdFREERTOS_ERRNO_NONE == createTask(debuggerTask,
                                             "Debugger Task",
                                             TaskDebugging::STACK_DEPTH,
                                             NULL,
                                             glob::DEBUG_TASK_PRIORITY,
                                             Task::NonRealTimeCore);
}

void TaskDebugging::shutdown() {
  Serial.end();
  deleteHandle();
}

}  // namespace task
