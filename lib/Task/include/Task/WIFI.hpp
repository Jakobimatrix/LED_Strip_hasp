/**
 * @file WIFI.hpp
 * @brief Simple RTOS task for handling WIFI communication.
 *
 * This header declares a small helper task used to manage WIFI connections
 * and message handling. It provides a `TaskWIFI` subclass of `task::Task`
 * which implements the `setup()`/`shutdown()` hooks and a FreeRTOS-compatible
 * `wifiTask` entry function used as the task entry point.
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
#include <Task/WiFiProvisioning.hpp>
#include <Credentials.hpp>



enum class WIFI_STATUS : std::uint8_t {
  NEED_PROVISIONING,
  NO_WIFI,
  CONNECTED,
  CONNECTING,
  IDLE
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

  TaskWiFiProvisioning provisioning_task;

 private:
  /**
   * @brief Perform one-time initialization for the WIFI task.
   *
   * Set up the WIFI client, configure callbacks and prepare for
   * message handling.
   *
   * @return `true` on successful setup, `false` on failure.
   */
  [[nodiscard]] bool setup() override;

  void doWifiProvisioning();

  void dealWithWiFiStopped(wl_status_t prev_status);

  /**
   * @brief Handle the case where the WiFi connection got disconnected.
   *
   * Disconnect means DESCONNECT WAS TRIGGERED (gracefully):
   *  - AP vanished
   *  - AP rebooted
   *  - reconnect is in progress
   *  - disconnect was requested
   *
   * This function is called when the WiFi status is `WL_DISCONNECTED`.
   * It logs the event and may trigger reconnection attempts after a timeout if the disconnection persists.
   *
   * @param prev_status The previous WiFi status last time it was checked.
   */
  void dealWithWiFiDisconnected(wl_status_t prev_status);

  void dealWithWiFiConnected(wl_status_t prev_status);

  void dealWithWiFiIdle(wl_status_t prev_status);

  void dealWithWiFiNoShield(wl_status_t prev_status);

  void dealWithWiFiNoSSIDAvail(wl_status_t prev_status);

  void dealWithWiFiScanCompleted(wl_status_t prev_status);

  void dealWithWiFiConnectFailed(wl_status_t prev_status);

  /**
   * @brief Handle the case where the WiFi connection is lost.
   *
   * Connection Lost means SIGNAL_LOSS:
   *  - The ESP was connected.
   *  - The AP disappeared or the signal became unusable.
   *  - Credentials are still valid.
   *
   * This function is called when the WiFi status is `WL_CONNECTION_LOST`.
   * It logs the event and may trigger reconnection attempts or other recovery
   * actions as needed.
   *
   * @param prev_status The previous WiFi status last time it was checked.
   */
  void dealWithWiFiConnectionLost(wl_status_t prev_status);

  void dealWithWiFiUnknown(wl_status_t prev_status);

  void startWiFi();

  void stopWiFi();

  void logWiFiStatusChange(const wl_status_t old_status, const wl_status_t new_status) const;

  std::pair<wl_status_t, wl_status_t> checkWiFiStatusChange();

  bool startProvisioningTask();

 public:
  void resetWiFi();

  /**
   * @brief Returns the state of the Wifi:
   * @return
   * - WIFI_STATUS::NEED_PROVISIONING: if no (correct) network/password was given yet via "SoftAP Prov" App.
   * - WIFI_STATUS::NO_WIFI: if something is wrong with the hardware
   * - WIFI_STATUS::CONNECTED: if the wifi is connected to a network
   * - WIFI_STATUS::CONNECTING: if provisioning is done but the connection did not happen yet. (also on a disconnect or signal loss)
   * - WIFI_STATUS::IDLE: if WiFi is tunred off
   */
  WIFI_STATUS getWifiStatus() const;

  /**
   * @brief Enable or disable Wifi. You may need to provide Wifi and password via "SoftAP Prov" App.
   */
  void setWifiOn(bool on);

  /**
   * @brief Release resources and perform clean shutdown for the task.
   */
  void shutdown() override;

  /**
   * @brief Check if the device needs provisioning. (if ssid and password are stored)
   *
   * @return `true` if provisioning is needed; `false` otherwise.
   */
  bool needsProvisioning() const;
};

}  // namespace task