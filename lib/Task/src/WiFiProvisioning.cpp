#include <Task/WiFiProvisioning.hpp>
#include <Credentials.hpp>
#include <Globals.hpp>
#include <Task/TaskPriority.hpp>

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

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
void provisioningTask(void* pvParameters) {
  auto* self = static_cast<TaskWiFiProvisioning*>(pvParameters);

  constexpr int httpPort              = 80;
  constexpr static char AP_SSID[]     = "ESP32-Setup";
  constexpr static char AP_PASSWORD[] = "configure";

  const IPAddress AP_IP(192, 168, 4, 1);
  const IPAddress AP_SUBNET(255, 255, 255, 0);

  WebServer server{httpPort};
  DNSServer dns;

  glob::dbgLedLogger.log(
    dbg::LEVEL::INFO, dbg::TOPIC::WIFI, "Starting WiFi provisioning task...");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(AP_IP, AP_IP, AP_SUBNET);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  glob::Credentials credentials;
  const bool hadCredentials = credentials.hasCredentials();
  if (hadCredentials) {
    WiFi.begin(credentials.getSSID().c_str(), credentials.getPassword().c_str());
    WiFi.setAutoReconnect(true);
  }

  glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                         dbg::TOPIC::WIFI,
                         "WiFi provisioning AP IP: ",
                         WiFi.softAPIP().toString().c_str());

  // redirect all DNS queries to our IP, this triggers the captive portal on phones
  dns.start(53, "*", AP_IP);

  server.on("/", [self, &server]() { self->handleRoot(server); });
  server.on("/save", HTTP_POST, [self, &server]() { self->handleSave(server); });
  server.onNotFound([self, &server]() { self->handleRoot(server); });
  server.begin();

  glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                         dbg::TOPIC::WIFI,
                         "Starting WiFi provisioning Server Online.");

  while (!self->isStopRequested()) {
    if (hadCredentials && WiFi.status() == WL_CONNECTED) {
      glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                             dbg::TOPIC::WIFI,
                             "Successfully connected to WiFi with stored "
                             "credentials. No provisioning needed.");
      break;
    }
    dns.processNextRequest();
    server.handleClient();

    // self->logStackHighWaterMark(dbg::TOPIC::WIFI);
    self->sleepFixedDelay(pdMS_TO_TICKS(300));
  }

  server.stop();
  dns.stop();
  WiFi.softAPdisconnect(true);

  self->shutdown();
}


void TaskWiFiProvisioning::handleRoot(WebServer& server) {
  glob::Credentials credentials;
  server.send(
    200,
    "text/html",
    getHTML(credentials.getSSID(), "Please enter your WiFi credentials."));
}

String TaskWiFiProvisioning::getHTML(const String& prev_ssid, const String& message) const {

  String html = PROVISION_HTML;
  html.replace("value=\"\"", String("value=\"") + prev_ssid + "\"");
  html.replace("<!-- message -->", message);
  return html;
}

void TaskWiFiProvisioning::handleSave(WebServer& server) {
  if (!server.hasArg("ssid") || server.arg("ssid").isEmpty()) {
    server.send(400, "text/plain", "SSID required");
    return;
  }
  glob::Credentials credentials;
  if (credentials.hasCredentials()) {
    glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                           dbg::TOPIC::WIFI,
                           "Overwriting existing WiFi credentials.");
  }
  credentials.save(server.arg("ssid"), server.arg("password"));
  glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                         dbg::TOPIC::WIFI,
                         "Saved WiFi credentials: SSID='",
                         server.arg("ssid").c_str(),
                         "', Password_size: %ul",
                         server.arg("password").length(),
                         "'");

  server.send(200,
              "text/html",
              "<html><body><h2>Try connecting to the new WiFi "
              "network...</h2></body></html>");
  server.handleClient();

  WiFi.begin(credentials.getSSID().c_str(), credentials.getPassword().c_str());
  WiFi.setAutoReconnect(true);

  TickType_t startTime = xTaskGetTickCount();
  constexpr TickType_t timeoutTicks = pdMS_TO_TICKS(10000);  // 10 seconds timeout
  while (WiFi.status() != WL_CONNECTED) {
    if (xTaskGetTickCount() - startTime > timeoutTicks) {
      break;
    }
    sleepFixedDelay(pdMS_TO_TICKS(300));
  }

  if (WiFi.status() == WL_CONNECTED) {
    glob::dbgLedLogger.log(dbg::LEVEL::INFO,
                           dbg::TOPIC::WIFI,
                           "Connected to WiFi. IP address: ",
                           WiFi.localIP().toString().c_str());
    server.send(200,
                "text/html",
                "<html><body><h2>Connected to WiFi. IP address: " +
                  WiFi.localIP().toString() + "</h2></body></html>");
    server.handleClient();
    delay(pdMS_TO_TICKS(2000));
    stop();
  }

  glob::dbgLedLogger.log(
    dbg::LEVEL::WARN, dbg::TOPIC::WIFI, "Failed to connect to WiFi.");

  const auto status{toString(WiFi.status())};
  const String statusMessage =
    "Failed to connect to WiFi. Please check your credentials "
    "and try again. Current status: ";

  server.send(401,
              "text/html",
              getHTML(server.arg("ssid"), statusMessage + String(status.c_str())));
  server.handleClient();
}

bool TaskWiFiProvisioning::setup() {
  return pdPASS == createTask(provisioningTask,
                              "WIFI Provisioning",
                              TaskWiFiProvisioning::STACK_DEPTH,
                              this,
                              task::WIFI_TASK_PRIORITY,
                              Task::NonRealTimeCore);
}

void TaskWiFiProvisioning::shutdown() { deleteHandle(); }
}  // namespace task
