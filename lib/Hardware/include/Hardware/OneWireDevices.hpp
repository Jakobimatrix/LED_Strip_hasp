/**
 * @file OneWireDevices.hpp
 * @brief Management class for handling OneWire devices on a bus.
 *
 * This header declares `OneWireDevices`, a class for managing and interacting
 * with multiple OneWire devices connected to a single bus. It provides methods
 * for device registration, discovery, and measurement control.
 *
 * @date 11.11.2026
 * @author Jakob Wandel
 * @version 1.0
 */

#pragma once

#include <Hardware/OneWireDeviceTemplate.hpp>

#include <Arduino.h>

#include <Preferences.h>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace hw {

/**
 * @class OneWireDevices
 * @brief Manages multiple OneWire devices on a single bus.
 *
 * Uses the Storage Preferences to store and retrieve device addresses:
 * key := bus_id + device_id, value := 8-byte device address.
 */
class OneWireDevices {
 public:
  /**
   * @brief Construct a new OneWireDevices object.
   * @param pin The GPIO pin number used for the OneWire bus.
   * @param bus_id An integer identifier for the bus (used for address storage).
   */
  OneWireDevices(uint8_t pin, int bus_id);

  /**
   * @brief Add a device to the list of managed devices.
   * @param device_id The ID of the device to add. With that ID you can later access the device.
   * @tparam T The type of the device to add (must derive from OneWireDeviceTemplate).
   * @return true if the device was added, false otherwise.
   */
  template <typename T>
    requires(std::is_base_of_v<OneWireDeviceTemplate, T>)
  bool add(int device_id) {
    if (m_discovered || findDevice(device_id) != nullptr) {
      return false;
    }

    m_devices.push_back(std::make_unique<T>(m_one_wire, device_id));

    return true;
  }

  /**
   * @brief Discover devices on the OneWire bus and assign addresses.
   * @return true if discovery was successful, false otherwise.
   */
  bool discover();

  /**
   * @brief Start measurement on all registered devices.
   */
  void startMeasurement();

  /**
   * @brief Read the measurement result from a specific device.
   * @param device_id The ID of the device to read from.
   * @tparam T The type of the device to read (must derive from OneWireDeviceTemplate).
   * @return The measurement result of the device.
   */
  template <typename T>
    requires(std::is_base_of_v<OneWireDeviceTemplate, T>)
  typename T::Result read(int device_id) {
    auto* device = findDevice(device_id);

    if (device == nullptr || device->typeToken() != T::staticTypeToken()) {
      return T::errorResult(T::Error::INVALID_DEVICE);
    }

    auto* typedDevice = static_cast<T*>(device);

    return typedDevice->read();
  }

  /**
   * @brief Check if discovery has been performed.
   * @return true if discovery has been performed, false otherwise.
   */
  bool discovered() const;

  /**
   * @brief Get the number of registered devices.
   * @return The number of registered devices.
   */
  size_t deviceCount() const;

 private:
  OneWire m_one_wire;

  int m_bus_id;

  bool m_discovered = false;

  std::vector<std::unique_ptr<OneWireDeviceTemplate>> m_devices;

  /**
   * @brief Find a device by its ID.
   * @param device_id The ID of the device to find.
   * @return A pointer to the device if found, nullptr otherwise.
   */
  OneWireDeviceTemplate* findDevice(int device_id);

  /**
   * @brief Save the addresses of all registered devices to Preferences.
   * @param bus_id The ID of the bus for which to save addresses.
   * @return true if saving was successful, false otherwise.
   */
  bool saveAddresses(int bus_id);

  /**
   * @brief Load the addresses of all registered devices from Preferences.
   * @param bus_id The ID of the bus for which to load addresses.
   * @return true if loading was successful, false otherwise.
   */
  bool loadAddresses(int bus_id);

  /**
   * @brief Assign addresses to devices based on discovered addresses and saved preferences.
   * @return true if all devices have been assigned an address, false otherwise.
   */
  bool assignAddresses();

  String preferenceNamespace(int bus_id) const;

  String preferenceKey(int device_id) const;
};

}  // namespace hw