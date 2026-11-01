#include <Task/Provisioning.hpp>

#include <Task/TaskPriority.hpp>

#include <algorithm>

namespace task {
namespace {

constexpr char PAGE_TEMPLATE[] = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Device Setup</title>
  <style>
    body { font-family: sans-serif; max-width: 400px; margin: 60px auto; padding: 0 20px; }
    input, select { width: 100%; padding: 8px; margin: 8px 0; box-sizing: border-box; }
    button { width: 100%; padding: 10px; background: #0078d4; color: white; border: none; cursor: pointer; }
    .provisioning-error { color: #b00020; }
  </style>
</head>
<body>
  <h2>Device Setup</h2>
  <form action="/save" method="POST">
    <!-- fields -->
    <button type="submit">Save and Connect</button>
  </form>
</body>
</html>
)";

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

bool isValidId(const ProvisioningId& id) {
  if (id.empty()) {
    return false;
  }
  for (const char character : id.view()) {
    const bool isLetter = (character >= 'a' && character <= 'z') ||
                          (character >= 'A' && character <= 'Z');
    const bool isDigit = character >= '0' && character <= '9';
    if (!isLetter && !isDigit && character != '_' && character != '-') {
      return false;
    }
  }
  return true;
}

}  // namespace

bool TaskProvisioning::registerProvisioning(std::unique_ptr<ProvisioningTemplate> provisioningTemplate) {
  if (!provisioningTemplate || isRunning() || registrationClosed) {
    return false;
  }
  templates.push_back({std::move(provisioningTemplate), {}, {}});
  return true;
}

bool TaskProvisioning::setup() {
  if (templates.empty()) {
    return false;
  }
  registrationClosed = true;
  return pdPASS == createTask(provisioningTask,
                              "Provisioning",
                              TaskProvisioning::STACK_DEPTH,
                              this,
                              task::WIFI_TASK_PRIORITY,
                              Task::NonRealTimeCore);
}

void provisioningTask(void* pvParameters) {
  auto* self = static_cast<TaskProvisioning*>(pvParameters);

  constexpr char AP_SSID_TEMPLATE[] = "ESP32-Setup-%04X";
  constexpr char AP_PASSWORD[]      = "";
  const IPAddress AP_IP(192, 168, 4, 1);
  const IPAddress AP_SUBNET(255, 255, 255, 0);

  self->server.stop();
  self->dns.stop();
  WiFi.disconnect(false, true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(AP_IP, AP_IP, AP_SUBNET);

  if (!self->initializeTemplates()) {
    self->shutdown();
    return;
  }

  char apSsid[32];
  snprintf(apSsid, sizeof(apSsid), AP_SSID_TEMPLATE, static_cast<unsigned>(esp_random() & 0xFFFF));
  if (!WiFi.softAP(apSsid, AP_PASSWORD)) {
    self->shutdown();
    return;
  }

  self->dns.start(53, "*", AP_IP);
  self->server.on("/", [self]() { self->handleRoot(); });
  self->server.on("/save", HTTP_POST, [self]() { self->handleSave(); });
  self->server.onNotFound([self]() { self->handleRoot(); });
  self->server.begin();

  self->sleepFixedDelay(pdMS_TO_TICKS(300));
  while (!self->isStopRequested()) {
    self->dns.processNextRequest();
    self->server.handleClient();
    self->logStackHighWaterMark(dbg::TOPIC::WIFIPROV);
    if (self->submissionSaved && self->allTemplatesComplete()) {
      break;
    }
    self->sleepFixedDelay(pdMS_TO_TICKS(300));
  }

  self->shutdown();
}

bool TaskProvisioning::initializeTemplates() {
  for (size_t index = 0; index < templates.size(); ++index) {
    auto& registered = templates[index];
    registered.ids   = registered.value->setUp();
    if (!hasUniqueValidIds(registered.ids, index)) {
      return false;
    }
    registered.errors.resize(registered.ids.size());
  }
  return true;
}

bool TaskProvisioning::hasUniqueValidIds(const std::vector<ProvisioningId>& ids,
                                         const size_t currentTemplate) const {
  for (size_t idIndex = 0; idIndex < ids.size(); ++idIndex) {
    if (!isValidId(ids[idIndex])) {
      return false;
    }
    for (size_t previousTemplate = 0; previousTemplate <= currentTemplate; ++previousTemplate) {
      const auto& previousIds =
        previousTemplate == currentTemplate ? ids : templates[previousTemplate].ids;
      const size_t previousLimit =
        previousTemplate == currentTemplate ? idIndex : previousIds.size();
      for (size_t previousIndex = 0; previousIndex < previousLimit; ++previousIndex) {
        if (ids[idIndex].view() == previousIds[previousIndex].view()) {
          return false;
        }
      }
    }
  }
  return true;
}

bool TaskProvisioning::allTemplatesComplete() const {
  return std::all_of(
    templates.begin(), templates.end(), [](const RegisteredTemplate& registered) {
      return registered.value->isComplete();
    });
}

String TaskProvisioning::makePage() {
  String page = PAGE_TEMPLATE;
  String fields;

  for (auto& registered : templates) {
    String templateFields = registered.value->provide();
    for (size_t index = 0; index < registered.ids.size(); ++index) {
      String errorToken  = F("<!-- error:");
      errorToken        += registered.ids[index].c_str();
      errorToken        += F(" -->");
      String escapedError = escapeHtml(String(registered.errors[index].c_str()));
      String errorMarkup;
      if (!escapedError.isEmpty()) {
        errorMarkup  = F("<span class=\"provisioning-error\">");
        errorMarkup += escapedError;
        errorMarkup += F("</span>");
      }
      templateFields.replace(errorToken, errorMarkup);
    }
    fields += templateFields;
  }

  page.replace("<!-- fields -->", fields);
  return page;
}

void TaskProvisioning::handleRoot() {
  server.send(200, "text/html", makePage());
}

void TaskProvisioning::handleSave() {
  bool allValid = true;
  for (auto& registered : templates) {
    std::vector<String> inputs;
    inputs.reserve(registered.ids.size());
    for (const auto& id : registered.ids) {
      inputs.push_back(server.arg(id.c_str()));
    }

    auto errors = registered.value->validate(std::move(inputs));
    if (errors.size() != registered.ids.size()) {
      server.send(500,
                  "text/plain",
                  "Provisioning template returned an invalid error count.");
      return;
    }
    registered.errors = std::move(errors);
    allValid          = allValid && std::all_of(registered.errors.begin(),
                                       registered.errors.end(),
                                       [](const ProvisioningError& error) {
                                         return error.empty();
                                       });
  }

  if (!allValid) {
    server.send(200, "text/html", makePage());
    return;
  }

  for (auto& registered : templates) {
    registered.value->save();
  }
  submissionSaved = true;
  server.send(200, "text/html", makePage());
}

void TaskProvisioning::shutdown() {
  server.stop();
  dns.stop();
  WiFi.softAPdisconnect(true);
  deleteHandle();
}

}  // namespace task