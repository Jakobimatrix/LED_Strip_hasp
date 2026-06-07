#include <Task/Debugging.hpp>
#include <Globals.hpp>

#include <Debugger/Serializer.hpp>

namespace task {


void debuggerTask(void* pvParameters) {
  auto serializer = dbg::Serializer(
    glob::debugTaskQueue, glob::debugWiFiQueue, glob::debugMqttQueue, glob::debugLedQueue);

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(20));
    while (serializer.printNext()) {
      vTaskDelay(pdMS_TO_TICKS(5));
    }
  }
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

void TaskDebugging::shutdown() { Serial.end(); }

}  // namespace task
