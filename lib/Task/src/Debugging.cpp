#include <Task/TaskDebugging.hpp>
#include <Globals.hpp>

#include <Debugger/Serializer.hpp>
#include <Debugger/Queues.hpp>

namespace task {


void debuggerTask(void* pvParameters) {
  auto serializer = Serializer(debugQueue, debugWiFiQueue, debugMqttQueue, debugTaskQueue);

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
