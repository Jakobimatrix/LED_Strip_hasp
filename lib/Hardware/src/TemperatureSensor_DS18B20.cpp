#include <Hardware/TemperatureSensor_DS18B20.hpp>

namespace hw {

TemperatureSensor_DS18B20::TemperatureSensor_DS18B20(OneWire& bus, int id)
    : OneWireDeviceTemplate(bus, id) {}

uint8_t TemperatureSensor_DS18B20::familyCode() const { return FAMILY_CODE; }

bool TemperatureSensor_DS18B20::Result::valid() const {
  return error == Error::NONE;
}

const char* TemperatureSensor_DS18B20::staticTypeToken() {
  static const char token = 0;
  return &token;
}

const char* TemperatureSensor_DS18B20::typeToken() const {
  return staticTypeToken();
}

TemperatureSensor_DS18B20::Result TemperatureSensor_DS18B20::errorResult(
  TemperatureSensor_DS18B20::Error error) {
  TemperatureSensor_DS18B20::Result result;
  result.error = error;
  return result;
}

// Start conversion for this particular sensor.
bool TemperatureSensor_DS18B20::startMeasurement() {
  if (!m_addressValid || !m_bus.reset()) {
    return false;
  }

  m_bus.select(m_sensor_address.data());
  m_bus.write(0x44);  // Convert T

  return true;
}

// Read temperature from this sensor.
TemperatureSensor_DS18B20::Result TemperatureSensor_DS18B20::read() {
  if (!m_addressValid) {
    return errorResult(Error::INVALID_DEVICE);
  }

  uint8_t data[9];

  if (!m_bus.reset()) {
    return errorResult(Error::NOT_CONNECTED);
  }

  m_bus.select(m_sensor_address.data());
  m_bus.write(0xBE);  // Read Scratchpad

  for (uint8_t i = 0; i < 9; i++) {
    data[i] = m_bus.read();
  }

  if (OneWire::crc8(data, 8) != data[8]) {
    return errorResult(Error::CRC_ERROR);
  }

  int16_t raw = static_cast<int16_t>((static_cast<uint16_t>(data[1]) << 8) | data[0]);

  Result result;
  result.temperature = raw / 16.0f;

  return result;
}

bool TemperatureSensor_DS18B20::sleep() { return false; }

bool TemperatureSensor_DS18B20::wake() { return false; }

OneWireDeviceTemplate::PowerState TemperatureSensor_DS18B20::powerState() const {
  return OneWireDeviceTemplate::PowerState::UNSUPPORTED;
}

}  // namespace hw