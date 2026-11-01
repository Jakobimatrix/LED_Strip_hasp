#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#include <Task/ProvisioningTemplate.hpp>
#include <Task/Task.hpp>

#include <memory>
#include <vector>

namespace task {

/**
 * @brief FreeRTOS task entry function used by `TaskProvisioning`.
 *
 * @param pvParameters Pointer passed to the task on creation (may be `nullptr`).
 */
void provisioningTask(void* pvParameters);

class TaskProvisioning : public Task {
  friend void provisioningTask(void* pvParameters);

  struct RegisteredTemplate {
    std::unique_ptr<ProvisioningTemplate> value;
    std::vector<ProvisioningId> ids;
    std::vector<ProvisioningError> errors;
  };

  constexpr static uint32_t STACK_DEPTH{4096};

  std::vector<RegisteredTemplate> templates;
  WebServer server{80};
  DNSServer dns;
  bool submissionSaved{false};
  bool registrationClosed{false};

  /**
   * @brief Perform one-time initialization for the Provisioning task.
   *
   * Set up the task if the templates are not empty.
   *
   * @return `true` on successful setup, `false` on failure.
   */
  [[nodiscard]] bool setup() override;

  /**
   * @brief Release resources and perform clean shutdown for the task.
   */
  void shutdown() override;

  /**
   * @brief Reads all templates and initializes templates
   */
  bool initializeTemplates();

  /**
   * @brief Given all registered ids, checks if the given template's ids are unique
   * @param ids The ids to check for uniqueness
   * @param currentTemplate The index of the template being checked
   * @return true if no ids in the template collide with the registered template's ids, false otherwise
   */
  bool hasUniqueValidIds(const std::vector<ProvisioningId>& ids, size_t currentTemplate) const;

  /**
   * @brief Checks if all registered templates are completely provisioned
   * @return true if all templates are complete, false otherwise
   */
  bool allTemplatesComplete() const;

  /**
   * @brief Generates the HTML page for the provisioning web server.
   *
   * This function constructs the HTML page by combining the template
   * fields and any validation errors. It replaces placeholders in the
   * page template with actual content.
   *
   * @return The complete HTML page as a `String`.
   */
  String makePage();

  /**
   * @brief Handles the root URL request by creating then sending the provisioning page.
   */
  void handleRoot();

  /**
   * @brief Handles the save request from the provisioning page.
   *
   * This function validates the input fields from all registered templates.
   * If all inputs are valid, it calls the `save()` method of each template.
   * If any input is invalid, it regenerates the page with error messages.
   */
  void handleSave();

 public:
  /**
   * @brief Registers a new provisioning template as long as the task was not started.
   *
   * This function adds a new provisioning template to the list of registered templates.
   * It checks for unique IDs if registration is still open.
   *
   * @param provisioningTemplate A unique pointer to the provisioning template to register.
   * @return `true` if the template was successfully registered; `false` if registration is closed or IDs are not unique.
   */
  bool registerProvisioning(std::unique_ptr<ProvisioningTemplate> provisioningTemplate);
};

}  // namespace task