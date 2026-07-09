#include <Task/WiFiProvisioning.hpp>
#include <Credentials.hpp>
#include <Globals.hpp>
#include <Task/TaskPriority.hpp>

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#include <algorithm>

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

  constexpr int HTTP_PORT             = 80;
  constexpr static char AP_SSID[]     = "ESP32-Setup";
  constexpr static char AP_PASSWORD[] = "";

  const IPAddress AP_IP(192, 168, 4, 1);
  const IPAddress AP_SUBNET(255, 255, 255, 0);

  WebServer server{HTTP_PORT};
  DNSServer dns;

  glob::dbgWiFiProvLogger.log(dbg::LEVEL::INFO,
                              dbg::TOPIC::WIFIPROV,
                              "Starting WiFi provisioning task...");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(AP_IP, AP_IP, AP_SUBNET);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  glob::Credentials credentials;
  const bool hadCredentials = credentials.hasCredentials();
  if (hadCredentials) {
    WiFi.begin(credentials.getSSID().c_str(), credentials.getPassword().c_str());
    WiFi.setAutoReconnect(true);
    glob::dbgWiFiProvLogger.log(
      dbg::LEVEL::INFO,
      dbg::TOPIC::WIFIPROV,
      "Attempting to connect to stored WiFi credentials...");
  } else {
    self->scanNetworks();
  }

  glob::dbgWiFiProvLogger.log(dbg::LEVEL::INFO,
                              dbg::TOPIC::WIFIPROV,
                              "WiFi provisioning AP IP: ",
                              WiFi.softAPIP().toString().c_str(),
                              " SSID: ",
                              AP_SSID,
                              " Password: ",
                              AP_PASSWORD);

  // redirect all DNS queries to our IP, this triggers the captive portal on phones
  dns.start(53, "*", AP_IP);

  server.on("/", [self, &server]() { self->handleRoot(server); });
  server.on("/save", HTTP_POST, [self, &server]() { self->handleSave(server); });
  server.onNotFound([self, &server]() { self->handleRoot(server); });
  server.begin();

  glob::dbgWiFiProvLogger.log(dbg::LEVEL::INFO,
                              dbg::TOPIC::WIFIPROV,
                              "Starting WiFi provisioning Server Online.");

  while (!self->isStopRequested()) {
    if (WiFi.status() == WL_CONNECTED) {
      if (hadCredentials) {
        glob::dbgWiFiProvLogger.log(
          dbg::LEVEL::INFO,
          dbg::TOPIC::WIFIPROV,
          "Successfully connected to WiFi with stored "
          "credentials. No provisioning needed.");
      } else {

        glob::dbgWiFiProvLogger.log(
          dbg::LEVEL::INFO,
          dbg::TOPIC::WIFIPROV,
          "Successfully connected to WiFi. Provisioning "
          "complete.");
      }
      break;
    }

    dns.processNextRequest();
    server.handleClient();

    self->logStackHighWaterMark(dbg::TOPIC::WIFIPROV);
    self->sleepFixedDelay(pdMS_TO_TICKS(300));
  }

  server.stop();
  dns.stop();
  WiFi.softAPdisconnect(true);

  self->shutdown();
}

void TaskWiFiProvisioning::scanNetworks() const {
  constexpr bool async_scan = true;
  const uint32_t max_ms_per_chan =
    scanForWeakNetworks ? maxScanTimePerChannelForWeakNetworks : maxScanTimePerChannel;
  constexpr uint8_t channel = 0;  // scan all channels
  const bool show_hidden    = false;
  const bool passive        = false;  // active scan
  WiFi.scanNetworks(
    async_scan, show_hidden, passive, max_ms_per_chan, channel, nullptr, nullptr);
}


String TaskWiFiProvisioning::getNetworkNames() const {
  const int num_networks = WiFi.scanComplete();

  if (num_networks < 1) {
    return "";
  }

  std::vector<String> ssid_list;
  ssid_list.reserve(static_cast<size_t>(num_networks));

  size_t num_chars{0};
  constexpr size_t HTML_OPTION_OVERHEAD{17};  // <option value="">
  for (int i = 0; i < num_networks; ++i) {
    const String ssid = WiFi.SSID(i);

    // Skip hidden networks
    if (ssid.isEmpty())
      continue;

    glob::dbgWiFiProvLogger.log(dbg::LEVEL::INFO,
                                dbg::TOPIC::WIFIPROV,
                                "Network ",
                                i,
                                ": SSID='",
                                ssid.c_str(),
                                "', RSSI=",
                                WiFi.RSSI(i),
                                " dBm");
    ssid_list.push_back(ssid);
    num_chars += ssid.length() + HTML_OPTION_OVERHEAD;
  }
  std::sort(ssid_list.begin(), ssid_list.end());

  String options;
  options.reserve(num_chars);
  for (const auto& ssid : ssid_list) {
    options += "<option value=\"";
    options += ssid;
    options += "\">";
  }
  return options;
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
  html.replace("<!-- networks -->", getNetworkNames());

  return html;
}

void TaskWiFiProvisioning::handleSave(WebServer& server) {
  if (!server.hasArg("ssid") || server.arg("ssid").isEmpty()) {
    server.send(400, "text/plain", "SSID required");
    return;
  }
  glob::Credentials credentials;
  if (credentials.hasCredentials()) {
    glob::dbgWiFiProvLogger.log(dbg::LEVEL::INFO,
                                dbg::TOPIC::WIFIPROV,
                                "Overwriting existing WiFi credentials.");
  }
  credentials.save(server.arg("ssid"), server.arg("password"));
  glob::dbgWiFiProvLogger.log(dbg::LEVEL::INFO,
                              dbg::TOPIC::WIFIPROV,
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
    glob::dbgWiFiProvLogger.log(dbg::LEVEL::INFO,
                                dbg::TOPIC::WIFIPROV,
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

  glob::dbgWiFiProvLogger.log(
    dbg::LEVEL::WARN, dbg::TOPIC::WIFIPROV, "Failed to connect to WiFi.");

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
