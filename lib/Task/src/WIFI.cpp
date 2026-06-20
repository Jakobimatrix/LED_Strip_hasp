#include <Task/WIFI.hpp>
#include <Globals.hpp>

#include <WiFi.h>

#include <Arduino.h>

#include <Types/StaticString.hpp>

#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"


typ::StaticString<16> toString(const wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:
      return "IDLE";
    case WL_NO_SSID_AVAIL:
      return "NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "SCAN_COMPLETED";
    case WL_CONNECTED:
      return "CONNECTED";
    case WL_CONNECT_FAILED:
      return "CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "DISCONNECTED";
    case WL_NO_SHIELD:
      return "NO_SHIELD";
    case WL_STOPPED:
      return "STOPPED";
    default:
      return "UNKNOWN_STATUS";
  }
}


namespace task {

void wifiTask(void* pvParameters) {
  auto* self = static_cast<TaskWIFI*>(pvParameters);

  while (!self->isStopRequested()) {
    const auto [old_status, new_status] = self->checkWiFiStatusChange();
    const bool status_has_changed{old_status != new_status};
    if (status_has_changed) {
      glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                             dbg::TOPIC::WIFI,
                             "WiFi status changed: ",
                             toString(old_status).c_str(),
                             " -> ",
                             toString(new_status).c_str());
    }
    switch (new_status) {
      case WL_STOPPED: {
        self->dealWithWiFiStopped(old_status);
        break;
      }
      case WL_IDLE_STATUS: {
        self->dealWithWiFiIdle(old_status);
        break;
      }
      case WL_NO_SHIELD: {
        self->dealWithWiFiNoShield(old_status);
        break;
      }
      case WL_NO_SSID_AVAIL: {
        self->dealWithWiFiNoSSIDAvail(old_status);
        break;
      }
      case WL_SCAN_COMPLETED: {
        self->dealWithWiFiScanCompleted(old_status);
        break;
      }
      case WL_CONNECT_FAILED: {
        self->dealWithWiFiConnectFailed(old_status);
        break;
      }
      case WL_CONNECTION_LOST: {
        self->dealWithWiFiConnectionLost(old_status);
        break;
      }
      case WL_DISCONNECTED: {
        self->dealWithWiFiDisconnected(old_status);
        break;
      }
      case WL_CONNECTED: {
        self->dealWithWiFiConnected(old_status);
        break;
      }
      default: {
        self->dealWithWiFiUnknown(old_status);
        break;
      }
    }

    self->logStackHighWaterMark(dbg::TOPIC::WIFI);
    self->sleepFixedDelay(pdMS_TO_TICKS(1000));
  }
  self->shutdown();
}

void TaskWIFI::startWiFi() {
  if (!isProvisioned()) {
    doWifiProvisioning();
  }
  WiFi.begin();
  WiFi.setAutoReconnect(true);
}

void TaskWIFI::stopWiFi() {
  const bool turnWiFiRadioOff{true};
  const bool eraseCredentials{false};
  WiFi.disconnectAsync(turnWiFiRadioOff, eraseCredentials);
}

void TaskWIFI::resetWiFi() {
  const bool turnWiFiRadioOff{true};
  const bool eraseCredentials{true};
  WiFi.disconnectAsync(turnWiFiRadioOff, eraseCredentials);
  wifi_prov_mgr_reset_provisioning();
  wifi_prov_mgr_reset_sm_state_on_failure();
}

void TaskWIFI::dealWithWiFiStopped(wl_status_t prev_status) {
  if (prev_status == WL_STOPPED) {
    return;
  }
  if (!wifi_on) {
    return;
  }
  startWiFi();
}

void TaskWIFI::dealWithWiFiDisconnected(wl_status_t prev_status) {
  if (!wifi_on) {
    return;
  }
  if (prev_status == WL_DISCONNECTED) {
    const TickType_t now = xTaskGetTickCount();
    if (now - wifi_disconnected_since < DISCONNECTION_TIMEOUT_TICKS) {
      return;
    }
    WiFi.disconnect();
    startWiFi();
    wifi_disconnected_since = xTaskGetTickCount();
    return;
  }
  glob::dbgWiFiLogger.log(dbg::LEVEL::WARN,
                          dbg::TOPIC::WIFI,
                          "WiFi disconnected. Previous status: ",
                          toString(prev_status).c_str());

  wifi_disconnected_since = xTaskGetTickCount();
}

void TaskWIFI::dealWithWiFiConnected([[maybe_unused]] wl_status_t prev_status) {
  if (!wifi_on) {
    const bool turnWiFiRadioOff{true};
    const bool eraseCredentials{false};
    WiFi.disconnectAsync(turnWiFiRadioOff, eraseCredentials);
    sleepFixedDelay(pdMS_TO_TICKS(2000));
  }
}

void TaskWIFI::dealWithWiFiIdle([[maybe_unused]] wl_status_t prev_status) {
  if (wifi_on) {
    startWiFi();
  }
}

void TaskWIFI::dealWithWiFiNoShield(wl_status_t prev_status) {
  if (prev_status == WL_NO_SHIELD) {
    return;
  }
  glob::dbgWiFiLogger.log(dbg::LEVEL::ERROR,
                          dbg::TOPIC::WIFI,
                          "No WiFi shield detected. Check your hardware.");
}

void TaskWIFI::dealWithWiFiNoSSIDAvail(wl_status_t prev_status) {
  if (prev_status == WL_NO_SSID_AVAIL) {
    return;
  }
  glob::dbgWiFiLogger.log(
    dbg::LEVEL::WARN,
    dbg::TOPIC::WIFI,
    "No SSID available. Check WiFi credentials and signal strength.");
}

void TaskWIFI::dealWithWiFiScanCompleted(wl_status_t prev_status) {
  if (prev_status == WL_SCAN_COMPLETED) {
    return;
  }
  // get all available networks:
  int num_networks = WiFi.scanComplete();
  glob::dbgWiFiLogger.log(dbg::LEVEL::INFO,
                          dbg::TOPIC::WIFI,
                          "WiFi scan completed. Found ",
                          num_networks,
                          " networks.");
  for (int i = 0; i < num_networks; ++i) {
    glob::dbgWiFiLogger.log(dbg::LEVEL::INFO,
                            dbg::TOPIC::WIFI,
                            "Network ",
                            i,
                            ": SSID='",
                            WiFi.SSID(i).c_str(),
                            "', RSSI=",
                            WiFi.RSSI(i),
                            " dBm, Encryption=",
                            static_cast<short>(WiFi.encryptionType(i)));
  }
}

void TaskWIFI::dealWithWiFiConnectFailed(wl_status_t prev_status) {
  if (prev_status != WL_CONNECT_FAILED) {
    wifi_reconnect_attempts = 0;
    return;
  }
  ++wifi_reconnect_attempts;

  glob::dbgWiFiLogger.log(dbg::LEVEL::ERROR,
                          dbg::TOPIC::WIFI,
                          "WiFi connection failed! Attempt ",
                          wifi_reconnect_attempts,
                          "of ",
                          MAX_WIFI_RECONNECT_ATTEMPTS);

  if (MAX_WIFI_RECONNECT_ATTEMPTS <= wifi_reconnect_attempts) {
    glob::dbgWiFiLogger.log(
      dbg::LEVEL::ERROR,
      dbg::TOPIC::WIFI,
      "Maximum WiFi reconnect attempts reached. Stopping WiFi.");
    wifi_reconnect_attempts = 0;

    resetWiFi();
    return;
  }
}

void TaskWIFI::dealWithWiFiConnectionLost(wl_status_t prev_status) {
  if (prev_status == WL_CONNECTION_LOST) {
    return;
  }
  glob::dbgWiFiLogger.log(dbg::LEVEL::WARN,
                          dbg::TOPIC::WIFI,
                          "WiFi connection lost. Attempting to reconnect.");
}

void TaskWIFI::dealWithWiFiUnknown(wl_status_t prev_status) {
  glob::dbgWiFiLogger.log(dbg::LEVEL::ERROR,
                          dbg::TOPIC::WIFI,
                          "Unknown WiFi status: ",
                          toString(prev_status).c_str());
}

std::pair<wl_status_t, wl_status_t> TaskWIFI::checkWiFiStatusChange() {

  const wl_status_t new_status = WiFi.status();
  if (new_status != last_known_status) {
    const wl_status_t prev_status = last_known_status;
    last_known_status             = new_status;
    return std::make_pair(prev_status, new_status);
  }
  return std::make_pair(last_known_status, last_known_status);
}


WIFI_STATUS TaskWIFI::getWifiStatus() const {
  const wl_status_t wifi_status = WiFi.status();
  switch (wifi_status) {
    case WL_STOPPED:
    case WL_IDLE_STATUS:
      return WIFI_STATUS::IDLE;
    case WL_NO_SHIELD:
    case WL_NO_SSID_AVAIL:
      return WIFI_STATUS::NO_WIFI;
    case WL_SCAN_COMPLETED:
    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
    case WL_DISCONNECTED: {
      if (isProvisioned()) {
        return WIFI_STATUS::CONNECTING;
      }
      return WIFI_STATUS::NEED_PROVISIONING;
    }
    case WL_CONNECTED:
      return WIFI_STATUS::CONNECTED;
    default:
      glob::dbgWiFiLogger.log(
        dbg::LEVEL::ERROR, dbg::TOPIC::WIFI, "Unknown WiFi status: ", static_cast<int>(wifi_status));
      return WIFI_STATUS::NO_WIFI;
  }
}

bool TaskWIFI::isProvisioned() const {
  bool provisioned = false;
  wifi_prov_mgr_is_provisioned(&provisioned);
  return provisioned;
}

void TaskWIFI::doWifiProvisioning() {
  glob::dbgWiFiLogger.log(
    dbg::LEVEL::INFO, dbg::TOPIC::WIFI, "Starting WiFi provisioning...");

  const bool provisioned = isProvisioned();
  if (!provisioned) {
    wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

    wifi_prov_mgr_start_provisioning(security, proveOfPossession, deviceName, nullptr);

    glob::dbgWiFiLogger.log(dbg::LEVEL::INFO,
                            dbg::TOPIC::WIFI,
                            "Open ESP SoftAP Prov app.\nDevice name: ",
                            deviceName,
                            "\nPOP: ",
                            proveOfPossession);
  } else {
    glob::dbgWiFiLogger.log(dbg::LEVEL::ERROR,
                            dbg::TOPIC::WIFI,
                            "Already provisioned. But inner state is "
                            "DISCONNECTED. This should not happen.");
    wifi_prov_mgr_deinit();
  }
}

bool TaskWIFI::setup() {
  wifi_prov_mgr_config_t config = {
    .scheme               = wifi_prov_scheme_ble,
    .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM};

  wifi_prov_mgr_init(config);

  return pdFREERTOS_ERRNO_NONE == createTask(wifiTask,
                                             "WIFI Task",
                                             TaskWIFI::STACK_DEPTH,
                                             this,
                                             glob::WIFI_TASK_PRIORITY,
                                             Task::NonRealTimeCore);
}

void TaskWIFI::setWifiOn(bool on) {
  if (wifi_on == on) {
    return;
  }
  wifi_on = on;
  if (!on) {
    const bool turnWiFiRadioOff{true};
    const bool eraseCredentials{false};
    WiFi.disconnect(turnWiFiRadioOff, eraseCredentials);
    WiFi.mode(wifi_mode_t::WIFI_MODE_NULL);
  } else {
    WiFi.mode(wifi_mode_t::WIFI_MODE_STA);
  }
}

void TaskWIFI::shutdown() {
  setWifiOn(false);
  deleteHandle();
}
}  // namespace task