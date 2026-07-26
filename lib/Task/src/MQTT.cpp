#include <Task/MQTT.hpp>
#include <Globals.hpp>
#include <Task/TaskPriority.hpp>


namespace task {

void mqttTask(void* pvParameters) {
  auto* self = static_cast<TaskMQTT*>(pvParameters);

  while (true) {
  }
  self->shutdown();
}

bool TaskMQTT::setup() {
  return pdPASS ==
         createTask(
           mqttTask, "MQTT", TaskMQTT::STACK_DEPTH, this, task::MQTT_TASK_PRIORITY, Task::NonRealTimeCore);
}

void TaskMQTT::shutdown() { deleteHandle(); }
}  // namespace task