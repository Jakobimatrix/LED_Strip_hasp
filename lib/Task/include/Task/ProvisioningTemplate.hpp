#pragma once

#include <Arduino.h>

#include <Types/StaticString.hpp>

#include <vector>

namespace task {

using ProvisioningId    = typ::StaticString<32>;
using ProvisioningError = typ::StaticString<128>;

class ProvisioningTemplate {
 public:
  virtual ~ProvisioningTemplate() = default;

  // Called once before the AP starts; returned IDs define the field order.
  virtual std::vector<ProvisioningId> setUp() = 0;

  // Return fields using matching HTML id/name values and <!-- error:ID --> slots.
  virtual String provide() = 0;

  // Return one error per input in ID order; an empty error marks a valid value.
  virtual std::vector<ProvisioningError> validate(std::vector<String> inputs) = 0;

  // Called only after every registered template validates successfully.
  virtual void save() = 0;

  // Return false while asynchronous post-save work should keep the AP open.
  virtual bool isComplete() const { return true; }
};

}  // namespace task