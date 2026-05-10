#pragma once

#include <Task/Task.hpp>

namespace task {

void debuggerTask(void* pvParameters);

class TaskDebugging : public Task {

  constexpr static uint32_t STACK_DEPTH{2048};
  [[nodiscard]] bool setup() override;
  void shutdown() override;
};

}  // namespace task
