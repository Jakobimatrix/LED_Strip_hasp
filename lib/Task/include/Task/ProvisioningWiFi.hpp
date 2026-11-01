#pragma once

#include <WiFi.h>

#include <Task/ProvisioningTemplate.hpp>

namespace task {

class ProvisioningWiFi final : public ProvisioningTemplate {
  bool scanForWeakNetworks{false};
  String ssid;
  String manualSSID;
  String password;
  bool connectionFailed{false};

  constexpr static uint32_t maxScanTimePerChannel{80};
  constexpr static uint32_t maxScanTimePerChannelForWeakNetworks{300};

  void scanNetworks();
  String getNetworkNames() const;

 public:
  std::vector<ProvisioningId> setUp() override;
  String provide() override;
  std::vector<ProvisioningError> validate(std::vector<String> inputs) override;
  void save() override;
  bool isComplete() const override;
};

}  // namespace task

typ::StaticString<16> toString(const wl_status_t status);