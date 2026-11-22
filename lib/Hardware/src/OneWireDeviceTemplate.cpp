
#include <Hardware/OneWireDeviceTemplate.hpp>

namespace hw {

OneWireDeviceTemplate::OneWireDeviceTemplate(OneWire& bus, int id)
    : m_bus(bus),
      m_id(id) {}


int OneWireDeviceTemplate::id() const { return m_id; }

const OneWireDeviceTemplate::Address& OneWireDeviceTemplate::address() const {
  return m_sensor_address;
}

bool OneWireDeviceTemplate::hasAddress() const { return m_addressValid; }

void OneWireDeviceTemplate::setAddress(const Address& address) {
  m_sensor_address = address;
  m_addressValid   = true;
}

void OneWireDeviceTemplate::clearAddress() {
  m_sensor_address.fill(0);
  m_addressValid = false;
}

}  // namespace hw