/**
 * @file Credentials.cpp
 * @brief Credential management for WiFi provisioning.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#include <Credentials.hpp>

#include <Globals.hpp>

namespace glob {


Credentials::Credentials() { load(); }

void Credentials::load() {
  ssid     = glob::prefs.getString("wifi_ssid", "");
  password = glob::prefs.getString("wifi_password", "");
}

bool Credentials::hasCredentials() const {
  return !ssid.isEmpty() && !password.isEmpty();
}

void Credentials::save(const String& new_ssid, const String& new_password) {
  ssid     = new_ssid;
  password = new_password;
  glob::prefs.putString("wifi_ssid", ssid);
  glob::prefs.putString("wifi_password", password);
}

void Credentials::clear() {
  ssid     = "";
  password = "";
  glob::prefs.remove("wifi_ssid");
  glob::prefs.remove("wifi_password");
}



}  // namespace glob
