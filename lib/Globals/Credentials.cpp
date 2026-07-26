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
  glob::prefs.begin("wifi", false);
  ssid     = glob::prefs.getString("wifi_ssid", "");
  password = glob::prefs.getString("wifi_password", "");
  glob::prefs.end();
}

bool Credentials::hasCredentials() const {
  return !ssid.isEmpty() && !password.isEmpty();
}

void Credentials::save(const String& new_ssid, const String& new_password) {
  ssid     = new_ssid;
  password = new_password;

  glob::prefs.begin("wifi", false);
  glob::prefs.putString("wifi_ssid", ssid);
  glob::prefs.putString("wifi_password", password);
  glob::prefs.end();
}

void Credentials::clear() {
  ssid     = "";
  password = "";

  glob::prefs.begin("wifi", false);
  glob::prefs.remove("wifi_ssid");
  glob::prefs.remove("wifi_password");
  glob::prefs.end();
}



}  // namespace glob
