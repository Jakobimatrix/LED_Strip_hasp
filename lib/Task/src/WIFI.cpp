#include <Task/WIFI.hpp>
#include <Globals.hpp>
#include <Task/TaskPriority.hpp>


#include <WiFi.h>
#include <Arduino.h>


namespace task {


void wifiTask(void* pvParameters) {
  auto* self = static_cast<TaskWIFI*>(pvParameters);

  self->startWiFi();

  while (!self->isStopRequested()) {
    const auto [old_status, new_status] = self->checkWiFiStatusChange();
    const bool status_has_changed{old_status != new_status};
    if (status_has_changed) {
      glob::dbgWiFiLogger.log(dbg::LEVEL::INFO,
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

bool TaskWIFI::needsProvisioning() const {
  glob::Credentials creds;
  creds.load();
  return !creds.hasCredentials();
}

bool TaskWIFI::startProvisioningTask() {
  if (!provisioning_task.isRunning()) {
    // this task will run WiFi.begin after provisioning is done and then terminate itself
    return provisioning_task.start();
  }
  return false;
}

void TaskWIFI::startWiFi() {
  if (startProvisioningTask()) {
    glob::dbgWiFiLogger.log(
      dbg::LEVEL::INFO, dbg::TOPIC::WIFI, "Starting provisioning softAP.");
  }
}

void TaskWIFI::stopWiFi() {
  const bool turnWiFiRadioOff{true};
  const bool eraseCredentials{false};
  WiFi.disconnectAsync(turnWiFiRadioOff, eraseCredentials);
}

void TaskWIFI::resetWiFi() {
  const bool turnWiFiRadioOff{true};
  const bool eraseCredentials{true};
  WiFi.scanDelete();
  WiFi.disconnectAsync(turnWiFiRadioOff, eraseCredentials);
  glob::Credentials creds;
  creds.clear();
  startWiFi();
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
    glob::dbgWiFiLogger.log(dbg::LEVEL::INFO,
                            dbg::TOPIC::WIFI,
                            "WIFI is connected but WiFi was requested to be "
                            "turned off. Disconnecting...");
    stopWiFi();
    sleepFixedDelay(pdMS_TO_TICKS(2000));
    return;
  }
  if (provisioning_task.isRunning()) {
    glob::dbgWiFiLogger.log(dbg::LEVEL::INFO,
                            dbg::TOPIC::WIFI,
                            "Provisioning task still running. Stopping it now "
                            "since WiFi is connected.");
    provisioning_task.stop();
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
  // WiFiProvisioning is doing things, let it cook.
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
    const TickType_t now = xTaskGetTickCount();
    if (now - wifi_disconnected_since < DISCONNECTION_TIMEOUT_TICKS) {
      return;
    }
    if (startProvisioningTask()) {
      glob::dbgWiFiLogger.log(
        dbg::LEVEL::WARN,
        dbg::TOPIC::WIFI,
        "WiFi connection lost. Restarting Provisioning in case User wants to "
        "change to different SSID.");
    }
  }
  glob::dbgWiFiLogger.log(dbg::LEVEL::WARN,
                          dbg::TOPIC::WIFI,
                          "WiFi connection lost. Attempting to reconnect.");

  wifi_disconnected_since = xTaskGetTickCount();
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
      return WIFI_STATUS::OFF;
    case WL_IDLE_STATUS:
      return WIFI_STATUS::IDLE;
    case WL_NO_SHIELD:
    case WL_NO_SSID_AVAIL:
      return WIFI_STATUS::NO_WIFI;
    case WL_SCAN_COMPLETED:
    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
    case WL_DISCONNECTED: {
      if (needsProvisioning()) {
        return WIFI_STATUS::NEED_PROVISIONING;
      }
      return WIFI_STATUS::CONNECTING;
    }
    case WL_CONNECTED:
      return WIFI_STATUS::CONNECTED;
    default:
      glob::dbgWiFiLogger.log(
        dbg::LEVEL::ERROR, dbg::TOPIC::WIFI, "Unknown WiFi status: ", static_cast<int>(wifi_status));
      return WIFI_STATUS::NO_WIFI;
  }
}


bool TaskWIFI::setup() {
  return pdPASS ==
         createTask(
           wifiTask, "WiFi", TaskWIFI::STACK_DEPTH, this, task::WIFI_TASK_PRIORITY, Task::NonRealTimeCore);
}

void TaskWIFI::setWifiOn(bool on) {
  if (wifi_on == on) {
    return;
  }
  wifi_on = on;
  if (!on) {
    stopWiFi();
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