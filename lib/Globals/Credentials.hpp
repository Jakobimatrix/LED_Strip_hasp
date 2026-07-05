/**
 * @file Credentials.hpp
 * @brief Credential management for WiFi provisioning.
 *
 * @date 05.07.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#include <WString.h>

namespace glob {

class Credentials {
  String ssid;
  String password;

 public:
  /**
   * @brief Loads credentials from persistent storage on construction.
   */
  Credentials();

  /**
   * @brief Load credentials from persistent storage.
   *
   * This function retrieves the stored SSID and password from the
   * preferences and updates the internal state of the `Credentials`
   * object. If no credentials are found, the SSID and password will
   * be empty strings.
   */
  void load();

  /**
   * @brief Check if valid credentials are available.
   * @return `true` if both SSID and password are non-empty; `false`
   *         otherwise.
   */
  bool hasCredentials() const;

  /**
   * @brief Get the stored SSID.
   * @return The SSID as a `String`. If no SSID is stored, returns an empty string.
   */
  String getSSID() const { return ssid; }

  /**
   * @brief Get the stored password.
   * @return The password as a `String`. If no password is stored, returns an empty string.
   */
  String getPassword() const { return password; }

  /**
   * @brief Save new credentials to persistent storage.
   *
   * This function updates the stored SSID and password in the preferences
   * and updates the internal state of the `Credentials` object.
   *
   * @param new_ssid The new SSID to store.
   * @param new_password The new password to store.
   */
  void save(const String& new_ssid, const String& new_password);

  /**
   * @brief Clear stored credentials from persistent storage.
   *
   * This function removes the stored SSID and password from the preferences
   * and resets the internal state of the `Credentials` object to empty strings.
   */
  void clear();
};


}  // namespace glob
