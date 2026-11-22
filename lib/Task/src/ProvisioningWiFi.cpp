#include <Task/ProvisioningWiFi.hpp>

#include <Settings/Credentials.hpp>

#include <algorithm>
#include <vector>

namespace {

String escapeHtml(const String& input) {
  String escaped;
  escaped.reserve(input.length());
  for (unsigned int index = 0; index < input.length(); ++index) {
    switch (input[index]) {
      case '&':
        escaped += F("&amp;");
        break;
      case '<':
        escaped += F("&lt;");
        break;
      case '>':
        escaped += F("&gt;");
        break;
      case '"':
        escaped += F("&quot;");
        break;
      case '\'':
        escaped += F("&#39;");
        break;
      default:
        escaped += input[index];
        break;
    }
  }
  return escaped;
}

}  // namespace

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

std::vector<ProvisioningId> ProvisioningWiFi::setUp() {
  settings::Credentials& credentials = settings::Credentials::getInstance();
  if (credentials.hasCredentials()) {
    ssid       = credentials.getSSID();
    manualSSID = ssid;
    password   = credentials.getPassword();
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setAutoReconnect(true);
  } else {
    scanNetworks();
  }

  return {"wifi_ssid", "wifi_ssid_manual", "wifi_password"};
}

String ProvisioningWiFi::provide() {
  String markup;
  markup.reserve(640);
  markup += F("<label for=\"wifi_ssid\">WiFi network</label>");
  markup += F("<select id=\"wifi_ssid\" name=\"wifi_ssid\">");
  markup += getNetworkNames();
  markup += F("</select><!-- error:wifi_ssid -->");
  markup += F("<label for=\"wifi_ssid_manual\">Or enter manually</label>");
  markup +=
    F("<input id=\"wifi_ssid_manual\" name=\"wifi_ssid_manual\" type=\"text\" "
      "value=\"");
  markup += escapeHtml(manualSSID);
  markup += F("\" placeholder=\"Enter SSID manually\">");
  markup += F("<!-- error:wifi_ssid_manual -->");
  markup += F("<label for=\"wifi_password\">Password</label>");
  markup +=
    F("<input id=\"wifi_password\" name=\"wifi_password\" type=\"password\">");
  markup += F("<!-- error:wifi_password -->");

  if (connectionFailed) {
    markup +=
      F("<p class=\"provisioning-error\">Failed to connect. Check the "
        "credentials and try again. Status: ");
    const auto status  = toString(WiFi.status());
    markup            += escapeHtml(String(status.c_str()));
    markup            += F("</p>");
  }
  return markup;
}

std::vector<ProvisioningError> ProvisioningWiFi::validate(std::vector<String> inputs) {
  std::vector<ProvisioningError> errors(inputs.size());
  if (inputs.size() != 3) {
    return errors;
  }

  const String selectedSSID = inputs[0];
  manualSSID                = inputs[1];
  ssid                      = manualSSID.isEmpty() ? selectedSSID : manualSSID;
  password                  = inputs[2];

  if (ssid.isEmpty()) {
    errors[1].assign("SSID required");
  } else if (ssid.length() > 32) {
    errors[1].assign("SSID must be 32 bytes or fewer");
  }

  if (!password.isEmpty() && (password.length() < 8 || password.length() > 64)) {
    errors[2].assign("Password must be empty or 8 to 64 characters");
  }
  connectionFailed = false;
  return errors;
}

void ProvisioningWiFi::save() {
  settings::Credentials& credentials = settings::Credentials::getInstance();
  credentials.save(ssid, password);
  WiFi.begin(credentials.getSSID().c_str(), credentials.getPassword().c_str());
  WiFi.setAutoReconnect(true);

  const TickType_t startTime        = xTaskGetTickCount();
  constexpr TickType_t timeoutTicks = pdMS_TO_TICKS(10000);
  while (WiFi.status() != WL_CONNECTED && xTaskGetTickCount() - startTime <= timeoutTicks) {
    vTaskDelay(pdMS_TO_TICKS(300));
  }
  connectionFailed = WiFi.status() != WL_CONNECTED;
}

bool ProvisioningWiFi::isComplete() const {
  return WiFi.status() == WL_CONNECTED;
}

void ProvisioningWiFi::scanNetworks() {
  constexpr bool asyncScan = true;
  const uint32_t maxMsPerChannel =
    scanForWeakNetworks ? maxScanTimePerChannelForWeakNetworks : maxScanTimePerChannel;
  constexpr uint8_t channel = 0;
  constexpr bool showHidden = false;
  constexpr bool passive    = false;
  WiFi.scanNetworks(asyncScan, showHidden, passive, maxMsPerChannel, channel, nullptr, nullptr);
}

String ProvisioningWiFi::getNetworkNames() const {
  const int numberOfNetworks = WiFi.scanComplete();
  String options = F("<option value=\"\">Select a network</option>");
  if (numberOfNetworks == WIFI_SCAN_RUNNING) {
    options += F("<option value=\"\" disabled>Scanning networks...</option>");
    return options;
  }
  if (numberOfNetworks < 1) {
    return options;
  }

  std::vector<String> networks;
  networks.reserve(static_cast<size_t>(numberOfNetworks));
  for (int index = 0; index < numberOfNetworks; ++index) {
    String network = WiFi.SSID(index);
    if (!network.isEmpty()) {
      networks.push_back(network);
    }
  }
  std::sort(networks.begin(), networks.end());

  for (const auto& network : networks) {
    const String safeNetwork  = escapeHtml(network);
    options                  += F("<option value=\"");
    options                  += safeNetwork;
    options                  += F("\"");
    if (network == ssid) {
      options += F(" selected");
    }
    options += F(">");
    options += safeNetwork;
    options += F("</option>");
  }
  return options;
}

}  // namespace task