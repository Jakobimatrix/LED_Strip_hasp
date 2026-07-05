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
  constexpr static char deviceName[]        = "ESP32_LED_STRIP";
  constexpr static char proveOfPossession[] = "123456789";

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
      <label>SSID</label>
      <input type="text" name="ssid" value="" required>
      <label>Password</label>
      <input type="password" name="password">
      <button type="submit">Save and Reboot</button>
    </form>
  </body>
  </html>
  )";

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
};

}  // namespace task