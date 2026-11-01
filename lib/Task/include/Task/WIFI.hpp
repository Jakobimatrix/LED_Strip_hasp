/**
 * @file WIFI.hpp
 * @brief Simple RTOS task for handling WIFI communication.
 *
 * This header declares a helper task for managing WiFi connections. `TaskWIFI`
 * implements the `Task` lifecycle and coordinates a registered provisioning
 * task with the device's WiFi status.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <cstdint>
#include <WiFi.h>

#include <Types/StaticString.hpp>
#include <Task/Task.hpp>
#include <Task/Provisioning.hpp>
#include <Credentials.hpp>

#include <utility>

typ::StaticString<16> toString(const wl_status_t status);

enum class WIFI_STATUS : std::uint8_t {
  NEED_PROVISIONING,
  NO_WIFI,
  CONNECTED,
  CONNECTING,
  IDLE,
  OFF
};

namespace task {

/**
 * @brief FreeRTOS task entry function used by `TaskWIFI`.
 *
 * @param pvParameters Pointer passed to the task on creation (may be `nullptr`).
 */
void wifiTask(void* pvParameters);

class TaskWIFI : public Task {
  friend void wifiTask(void* pvParameters);

  constexpr static uint32_t STACK_DEPTH{4096};
  constexpr static char deviceName[]        = "ESP32_LED_STRIP";
  constexpr static char proveOfPossession[] = "123456789";

  TickType_t currentMaxExpectedLoopTime     = pdMS_TO_TICKS(100);
  constexpr static TickType_t SLEEP_TIME_MS = pdMS_TO_TICKS(3000);
  wl_status_t last_known_status             = wl_status_t::WL_STOPPED;

  bool wifi_on{false};
  TickType_t wifi_disconnected_since{0};
  constexpr static TickType_t DISCONNECTION_TIMEOUT_TICKS = pdMS_TO_TICKS(30000);
  unsigned short wifi_reconnect_attempts{0};
  constexpr static unsigned short MAX_WIFI_RECONNECT_ATTEMPTS = 5;

  TaskProvisioning provisioning_task;

 private:
  /**
   * @brief Perform one-time initialization for the WiFi task.
   * @return `true` on successful initialization, `false` otherwise.
   */
  [[nodiscard]] bool setup() override;

  void doWifiProvisioning();
  void dealWithWiFiStopped(wl_status_t prev_status);
  /**
   * @brief Handle a disconnect or a prolonged loss of connectivity.
   * @param prev_status The previous WiFi status.
   */
  void dealWithWiFiDisconnected(wl_status_t prev_status);
  void dealWithWiFiConnected(wl_status_t prev_status);
  void dealWithWiFiIdle(wl_status_t prev_status);
  void dealWithWiFiNoShield(wl_status_t prev_status);
  void dealWithWiFiNoSSIDAvail(wl_status_t prev_status);
  void dealWithWiFiScanCompleted(wl_status_t prev_status);
  void dealWithWiFiConnectFailed(wl_status_t prev_status);
  /**
   * @brief Handle loss of signal after a successful connection.
   * @param prev_status The previous WiFi status.
   */
  void dealWithWiFiConnectionLost(wl_status_t prev_status);
  void dealWithWiFiUnknown(wl_status_t prev_status);
  void startWiFi();
  void stopWiFi();
  void logWiFiStatusChange(const wl_status_t old_status, const wl_status_t new_status) const;
  std::pair<wl_status_t, wl_status_t> checkWiFiStatusChange();
  bool startProvisioningTask();

 public:
  TaskWIFI();

  /** @brief Clear stored credentials and restart WiFi provisioning. */
  void resetWiFi();
  /** @brief Return the current high-level WiFi state. */
  WIFI_STATUS getWifiStatus() const;
  /** @brief Enable or disable WiFi. */
  void setWifiOn(bool on);
  /** @brief Release resources and stop the task. */
  void shutdown() override;
  /** @brief Return whether saved WiFi credentials are available. */
  bool needsProvisioning() const;
};

}  // namespace task
