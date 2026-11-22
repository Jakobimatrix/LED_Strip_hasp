
#pragma once

#include <Hardware/OneWireDeviceTemplate.hpp>

namespace hw {

class TemperatureSensor_DS18B20 : public OneWireDeviceTemplate {
 public:
  static constexpr uint8_t FAMILY_CODE = 0x28;

  enum class Error { NONE, INVALID_DEVICE, NOT_CONNECTED, CRC_ERROR };

  static constexpr std::string_view toString(Error error) {
    switch (error) {
      case Error::NONE:
        return "No error";
      case Error::INVALID_DEVICE:
        return "Invalid device";
      case Error::NOT_CONNECTED:
        return "Device not connected";
      case Error::CRC_ERROR:
        return "CRC error";
      default:
        return "Unknown error";
    }
  }

  struct Result {
    float temperature = 0.0f;
    Error error       = Error::NONE;

    bool valid() const;
  };

  TemperatureSensor_DS18B20(OneWire& bus, int id);

  uint8_t familyCode() const override;

  static const char* staticTypeToken();

  const char* typeToken() const override;

  static Result errorResult(Error error);

  // Start conversion for this particular sensor.
  bool startMeasurement() override;

  // Read temperature from this sensor.
  Result read();

  bool sleep() override;

  bool wake() override;

  PowerState powerState() const override;
};


}  // namespace hw