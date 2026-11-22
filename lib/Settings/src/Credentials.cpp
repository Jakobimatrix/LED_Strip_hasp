/**
 * @file Credentials.cpp
 * @brief Credential management for WiFi provisioning.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */

#include <Settings/Credentials.hpp>

namespace settings {


Credentials::Credentials() { load(); }

void Credentials::load() {
  m_preferences.begin("wifi", true);
  m_ssid     = m_preferences.getString("wifi_ssid", "");
  m_password = m_preferences.getString("wifi_password", "");
  m_preferences.end();
}

bool Credentials::hasCredentials() const {
  return !m_ssid.isEmpty() && !m_password.isEmpty();
}

void Credentials::save(const String& new_ssid, const String& new_password) {
  m_ssid     = new_ssid;
  m_password = new_password;

  m_preferences.begin("wifi", false);
  m_preferences.putString("wifi_ssid", m_ssid);
  m_preferences.putString("wifi_password", m_password);
  m_preferences.end();
}

void Credentials::clear() {
  m_ssid     = "";
  m_password = "";

  m_preferences.begin("wifi", false);
  m_preferences.remove("wifi_ssid");
  m_preferences.remove("wifi_password");
  m_preferences.end();
}



}  // namespace settings
