#pragma once

#include <Arduino.h>

namespace task {

class Task {
 protected:
  Task();

  TaskHandle_t handle;

  [[nodiscard]] virtual bool setup() = 0;
  virtual void shutdown()            = 0;

  uint32_t stackDepth{0};

  BaseType_t createTask(TaskFunction_t pxTaskCode,
                        const char* const pcName,
                        const uint32_t ulStackDepth,
                        void* const pvParameters,
                        UBaseType_t uxPriority,
                        const BaseType_t xCoreID);

 public:
  constexpr static BaseType_t RealTimeCore{0};
  constexpr static BaseType_t NonRealTimeCore{1};

  constexpr static UBaseType_t LowPriority{1};
  constexpr static UBaseType_t MediumPriority{2};
  constexpr static UBaseType_t HighPriority{3};
  constexpr static UBaseType_t RealTimePriority{4};

  ~Task();

  [[nodiscard]] bool isRunning() const;

  [[nodiscard]] bool start();

  [[nodiscard]] bool stop();

  [[nodiscard]] uint32_t getStackDepth() const { return stackDepth; }

  [[nodiscard]] UBaseType_t getPriority() const {
    return uxTaskPriorityGet(handle);
  }
  [[nodiscard]] UBaseType_t getCore() const { return xPortGetCoreID(); }
  [[nodiscard]] UBaseType_t getStackHighWaterMark() const {
    return uxTaskGetStackHighWaterMark(handle);
  }
};

}  // namespace task
