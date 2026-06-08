/**
 * @file BlinkOnboardLED.hpp
 * @brief Simple RTOS task that blinks the board's onboard LED.
 *
 * This header declares a small helper task used to blink the onboard
 * LED at a fixed period. It provides a `TaskBlinkOnboardLED` subclass of
 * `task::Task` which implements the `setup()`/`shutdown()` hooks and a
 * FreeRTOS-compatible `ledBlinkTask` entry function used as the task
 * entry point.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Arduino.h>
#include <Hardware.hpp>

#include <Task/Task.hpp>

/** @namespace task Task subsystem: task wrappers and helpers used by the firmware. */
namespace task {
/**
 * @brief FreeRTOS task entry function used by `TaskBlinkOnboardLED`.
 *
 * @param pvParameters Pointer passed to the task on creation (may be `nullptr`).
 */
void ledBlinkTask(void* pvParameters);

class TaskBlinkOnboardLED : public Task {
  friend void ledBlinkTask(void* pvParameters);

  /** @brief Stack depth (bytes) allocated for the blink task. */
  constexpr static uint32_t STACK_DEPTH{2048};

  /**
   * @brief Perform one-time initialization for the blink task.
   *
   * Configure the LED GPIO and any state required before the task loop
   * begins.
   *
   * @return `true` on successful setup, `false` on failure.
   */
  [[nodiscard]] bool setup() override;

  /**
   * @brief Release resources and perform clean shutdown for the task.
   *
   */
  void shutdown() override;

  /** @brief Current LED output state (0 = on, non-zero = off). */
  uint8_t ledState{1};

  /** @brief GPIO pin number used for the onboard LED. */
  int ledPin{ON_BOARD_LED_PIN};

  /** @brief Period between LED toggles expressed as RTOS ticks. */
  static constexpr TickType_t period = pdMS_TO_TICKS(1000);
};

}  // namespace task
