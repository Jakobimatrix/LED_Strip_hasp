/**
 * @file MQTT.hpp
 * @brief Simple RTOS task for handling MQTT communication.
 *
 * This header declares a small helper task used to manage MQTT connections
 * and message handling. It provides a `TaskMQTT` subclass of `task::Task`
 * which implements the `setup()`/`shutdown()` hooks and a FreeRTOS-compatible
 * `mqttTask` entry function used as the task entry point.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Task/Task.hpp>

namespace task {

/**
 * @brief FreeRTOS task entry function used by `TaskMQTT`.
 *
 * @param pvParameters Pointer passed to the task on creation (may be `nullptr`).
 */
void mqttTask(void* pvParameters);

class TaskMQTT : public Task {
  friend void mqttTask(void* pvParameters);

  /** @brief Stack depth (bytes) allocated for the MQTT task. */
  constexpr static uint32_t STACK_DEPTH{4096};

  /**
   * @brief Perform one-time initialization for the MQTT task.
   *
   * Set up the MQTT client, configure callbacks and prepare for
   * message handling.
   *
   * @return `true` on successful setup, `false` on failure.
   */
  [[nodiscard]] bool setup() override;

 public:
  /**
   * @brief Release resources and perform clean shutdown for the task.
   */
  void shutdown() override;
};

}  // namespace task