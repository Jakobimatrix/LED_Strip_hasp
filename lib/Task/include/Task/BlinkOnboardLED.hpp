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

#include <array>

#include <Hardware.hpp>

#include <Task/Task.hpp>
#include <Task/WIFI.hpp>

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

  TaskWIFI taskWiFi;

  void handleWiFiTask();

  /**
   * @brief Perform one-time initialization for the blink task.
   *
   * Configure the LED GPIO and any state required before the task loop
   * begins.
   *
   * @return `true` on successful setup, `false` on failure.
   */
  [[nodiscard]] bool setup() override;

  /** @brief Current LED output state (0 = on, non-zero = off). */
  uint8_t ledState{1};

  /** @brief GPIO pin number used for the onboard LED. */
  int ledPin{ON_BOARD_LED_PIN};

  constexpr static size_t NUM_PERIODS{6};
  size_t current_period_index{NUM_PERIODS};
  TickType_t getPeriodSleep();

  enum class LED_PATTERN {
    HEART_BEAT,
    NEED_PROVISIONING,
    WIFI_CONNECTION,
    NO_WIFI_ERROR,
    IDLE,
  };

  LED_PATTERN current_led_pattern{LED_PATTERN::HEART_BEAT};

  static constexpr std::array<TickType_t, NUM_PERIODS> HEART_BEAT_PATTERN{
    pdMS_TO_TICKS(500),
    pdMS_TO_TICKS(500),
    pdMS_TO_TICKS(500),
    pdMS_TO_TICKS(500),
    pdMS_TO_TICKS(500),
    pdMS_TO_TICKS(500)};

  static constexpr std::array<TickType_t, NUM_PERIODS> NEED_PROVISIONING_PATTERN{
    pdMS_TO_TICKS(10000),
    pdMS_TO_TICKS(1),
    pdMS_TO_TICKS(10000),
    pdMS_TO_TICKS(1),
    pdMS_TO_TICKS(10000),
    pdMS_TO_TICKS(1)};

  static constexpr std::array<TickType_t, NUM_PERIODS> WIFI_CONNECTION_PATTERN{
    pdMS_TO_TICKS(200),
    pdMS_TO_TICKS(200),
    pdMS_TO_TICKS(200),
    pdMS_TO_TICKS(200),
    pdMS_TO_TICKS(200),
    pdMS_TO_TICKS(200)};

  static constexpr std::array<TickType_t, NUM_PERIODS> NO_WIFI_ERROR_PATTERN{
    pdMS_TO_TICKS(1000),
    pdMS_TO_TICKS(100),
    pdMS_TO_TICKS(1000),
    pdMS_TO_TICKS(100),
    pdMS_TO_TICKS(1000),
    pdMS_TO_TICKS(100)};

  static constexpr std::array<TickType_t, NUM_PERIODS> IDLE_PATTERN{
    pdMS_TO_TICKS(100),
    pdMS_TO_TICKS(2000),
    pdMS_TO_TICKS(100),
    pdMS_TO_TICKS(2000),
    pdMS_TO_TICKS(100),
    pdMS_TO_TICKS(2000)};


 public:
  /**
   * @brief Release resources and perform clean shutdown for the task.
   */
  void shutdown() override;
};

}  // namespace task
