/**
 * @file WiFiProvisioning.hpp
 * @brief Starts a soft access point provisioning service for the ESP32 to allow users to configure WiFi credentials.
 *
 *
 * @date 05.07.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once
#include <cstdint>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include <Types/StaticString.hpp>
#include <Task/Task.hpp>


typ::StaticString<16> toString(const wl_status_t status);

namespace task {

/**
 * @brief FreeRTOS task entry function used by `TaskWIFI`.
 *
 * @param pvParameters Pointer passed to the task on creation (may be `nullptr`).
 */
void provisioningTask(void* pvParameters);


class TaskWiFiProvisioning : public Task {
  friend void provisioningTask(void* pvParameters);

  constexpr static uint32_t STACK_DEPTH{4096};

  inline static constexpr const char* PROVISION_HTML = R"(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>WiFi Setup</title>
    <style>
      body { font-family: sans-serif; max-width: 400px; margin: 60px auto; padding: 0 20px; }
      input { width: 100%; padding: 8px; margin: 8px 0; box-sizing: border-box; }
      button { width: 100%; padding: 10px; background: #0078d4; color: white; border: none; cursor: pointer; }
    </style>
  </head>
  <body>
    <h2>WiFi Setup</h2>
    <p><!-- message --></p>
    <form action="/save" method="POST">
    <label for="ssid">SSID</label>
    <select id="ssid" name="ssid" required>
      <option value="">Select a network</option>
      <!-- networks -->
    </select>
    <label for="ssid_manual">Or enter manually</label>
    <input id="ssid_manual" name="ssid_manual" type="text" value="<!-- prev_ssid -->" placeholder="Enter SSID manually">
    <label>Password</label>
    <input type="password" name="password">
    <button type="submit">Save and Connect</button>
    </form>
  </body>
  </html>
  )";

  bool scanForWeakNetworks{false};
  bool scanNetworksSuccessful{false};
  bool scanningNetworks{false};
  constexpr static uint32_t maxScanTimePerChannel{80};
  constexpr static uint32_t maxScanTimePerChannelForWeakNetworks{300};

  String getHTML(const String& prev_ssid, const String& message) const;

 private:
  /**
   * @brief Perform one-time initialization for the provisioningTask .
   *
   * @return `true` on successful setup, `false` on failure.
   */
  [[nodiscard]] bool setup() override;

  void handleRoot(WebServer& server);
  void handleSave(WebServer& server);

 public:
  /**
   * @brief Release resources and perform clean shutdown for the task.
   */
  void shutdown() override;

  void scanNetworks();

  void onNetworkScanComplete();

  String getNetworkNames() const;
};

}  // namespace task