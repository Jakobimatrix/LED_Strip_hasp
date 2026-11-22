
#pragma once

#include <Arduino.h>
#include <OneWire.h>
#include <Preferences.h>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace hw {

class OneWireDeviceTemplate {
 public:
  using Address = std::array<uint8_t, 8>;

  enum class PowerState { UNKNOWN, ACTIVE, SLEEPING, UNSUPPORTED };

  virtual bool sleep() = 0;

  virtual bool wake() = 0;

  virtual PowerState powerState() const = 0;

  OneWireDeviceTemplate(OneWire& bus, int id);

  virtual ~OneWireDeviceTemplate() = default;

  int id() const;

  const Address& address() const;

  bool hasAddress() const;

  virtual uint8_t familyCode() const = 0;

  /**
   * @brief Returns a unique token (address) representing the device type.
   * This token is used to identify the device type in a generic way, allowing
   * for polymorphic handling of different device types.
   * @return A pointer to a unique character token representing the device type.
   */
  virtual const char* typeToken() const = 0;

  virtual bool startMeasurement() = 0;

  void setAddress(const Address& address);

  void clearAddress();

 protected:
  OneWire& m_bus;

  int m_id;

  Address m_sensor_address{};

  bool m_addressValid = false;
};

}  // namespace hw