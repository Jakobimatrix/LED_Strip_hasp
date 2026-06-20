#include <Task/MQTT.hpp>
#include <Globals.hpp>


namespace task {

void mqttTask(void* pvParameters) {
  auto* self = static_cast<TaskMQTT*>(pvParameters);

  while (true) {
  }
  self->shutdown();
}

bool TaskMQTT::setup() {
  return pdFREERTOS_ERRNO_NONE == createTask(mqttTask,
                                             "MQTT Task",
                                             TaskMQTT::STACK_DEPTH,
                                             this,
                                             glob::MQTT_TASK_PRIORITY,
                                             Task::NonRealTimeCore);
}

void TaskMQTT::shutdown() { deleteHandle(); }
}  // namespace task