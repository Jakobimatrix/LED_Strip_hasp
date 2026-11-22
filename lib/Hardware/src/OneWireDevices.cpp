
#include <Hardware/OneWireDevices.hpp>

#include <cstdio>

namespace hw {

OneWireDevices::OneWireDevices(uint8_t pin, int bus_id)
    : m_one_wire(pin),
      m_bus_id(bus_id) {}



bool OneWireDevices::discovered() const { return m_discovered; }

size_t OneWireDevices::deviceCount() const { return m_devices.size(); }


// --------------------------------------------------
// Device lookup
// --------------------------------------------------

OneWireDeviceTemplate* OneWireDevices::findDevice(int device_id) {
  for (auto& device : m_devices) {
    if (device->id() == device_id) {
      return device.get();
    }
  }

  return nullptr;
}


// --------------------------------------------------
// Preferences namespace / keys
// --------------------------------------------------

String OneWireDevices::preferenceNamespace(int bus_id) const {
  char buffer[12];

  snprintf(buffer, sizeof(buffer), "ow%08X", static_cast<uint32_t>(bus_id));

  return String(buffer);
}

String OneWireDevices::preferenceKey(int device_id) const {
  char buffer[12];

  snprintf(buffer, sizeof(buffer), "a%08X", static_cast<uint32_t>(device_id));

  return String(buffer);
}


// --------------------------------------------------
// Load previously saved addresses
// --------------------------------------------------

bool OneWireDevices::loadAddresses(int bus_id) {
  Preferences preferences;

  if (!preferences.begin(preferenceNamespace(bus_id).c_str(), true)) {
    return false;
  }

  for (auto& device : m_devices) {
    const String key = preferenceKey(device->id());

    if (preferences.getBytesLength(key.c_str()) != 8) {
      continue;
    }

    OneWireDeviceTemplate::Address address{};

    if (preferences.getBytes(key.c_str(), address.data(), address.size()) != 8) {
      continue;
    }

    // Validate ROM CRC and family code.
    if (OneWire::crc8(address.data(), 7) != address[7] ||
        address[0] != device->familyCode()) {
      continue;
    }

    device->setAddress(address);
  }

  preferences.end();
  return true;
}


// --------------------------------------------------
// Discover and assign missing addresses
// --------------------------------------------------

bool OneWireDevices::assignAddresses() {
  std::vector<OneWireDeviceTemplate::Address> discoveredAddresses;

  OneWireDeviceTemplate::Address address{};

  m_one_wire.reset_search();

  while (m_one_wire.search(address.data())) {
    if (OneWire::crc8(address.data(), 7) != address[7]) {
      continue;
    }

    discoveredAddresses.push_back(address);
  }

  std::vector<bool> used(discoveredAddresses.size(), false);

  // Reserve addresses already loaded from Preferences.
  for (const auto& device : m_devices) {
    if (!device->hasAddress()) {
      continue;
    }

    for (size_t i = 0; i < discoveredAddresses.size(); i++) {
      if (device->address() == discoveredAddresses[i]) {
        used[i] = true;
        break;
      }
    }
  }

  // Assign addresses to devices without saved addresses.
  for (auto& device : m_devices) {
    if (device->hasAddress()) {
      continue;
    }

    for (size_t i = 0; i < discoveredAddresses.size(); i++) {
      if (used[i]) {
        continue;
      }

      if (discoveredAddresses[i][0] != device->familyCode()) {
        continue;
      }

      device->setAddress(discoveredAddresses[i]);
      used[i] = true;

      break;
    }
  }

  // All registered devices must have an address.
  for (const auto& device : m_devices) {
    if (!device->hasAddress()) {
      return false;
    }
  }

  return true;
}


// --------------------------------------------------
// Save addresses
// --------------------------------------------------

bool OneWireDevices::saveAddresses(int bus_id) {
  Preferences preferences;

  if (!preferences.begin(preferenceNamespace(bus_id).c_str(), false)) {
    return false;
  }

  bool success = true;

  for (const auto& device : m_devices) {
    if (!device->hasAddress()) {
      success = false;
      continue;
    }

    const String key = preferenceKey(device->id());

    if (preferences.putBytes(
          key.c_str(), device->address().data(), device->address().size()) != 8) {
      success = false;
    }
  }

  preferences.end();
  return success;
}


// --------------------------------------------------
// Public functions
// --------------------------------------------------

bool OneWireDevices::discover() {
  if (m_discovered) {
    return true;
  }

  loadAddresses(m_bus_id);
  const bool success = assignAddresses();
  if (m_bus_id >= 0) {
    saveAddresses(m_bus_id);
  }

  m_discovered = true;

  return success;
}


void OneWireDevices::startMeasurement() {
  if (!m_discovered) {
    return;
  }

  for (auto& device : m_devices) {
    device->startMeasurement();
  }
}

}  // namespace hw