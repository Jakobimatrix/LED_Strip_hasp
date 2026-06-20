/**
 * @file Task.hpp
 * @brief Base Task abstraction wrapping RTOS task lifecycle and helpers.
 *
 * This header declares `task::Task`, an abstract RAII-style wrapper for
 * FreeRTOS tasks used across the firmware. Derive from `Task` and
 * implement `setup()` and `shutdown()` to provide task-specific
 * initialization and teardown. The class provides convenience helpers
 * for creating tasks, querying priority/core/stack usage and simple
 * start/stop control.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Arduino.h>
#include <Hardware.hpp>

#include <Types/StaticString.hpp>
#include <Debugger/Constants.hpp>


#include <atomic>

/** @namespace task Task subsystem: task wrappers and helpers used by the firmware. */
namespace task {


/**
 * @class Task
 * @brief Abstract base class providing an RTOS-compatible task wrapper.
 *
 * Derive from `Task` to create a firmware task. Subclasses must
 * implement the protected `setup()` and `shutdown()` lifecycle hooks.
 * The wrapper manages the underlying `TaskHandle_t` and offers helpers
 * to create the RTOS task with a controlled name, stack depth and
 * priority. `start()` and `stop()` manage the task lifecycle at runtime.
 */
class Task {
  /**
   * @brief Handle to the underlying RTOS task.
   */
  TaskHandle_t handle{nullptr};


  TickType_t cycle_start_tick;

 protected:
  /**
   * @brief Default protected constructor.
   *
   * Constructing a `Task` does not start the underlying RTOS task.
   * Use `start()` to create and run the task.
   */
  Task();

  /**
   * @brief Human-readable task name.
   */
  typ::StaticString<16> name{"unnamed"};

  /**
   * @brief Task initialization hook to implement by subclasses.
   *
   * Called during task startup to perform initialization required
   * before the task's main loop runs (e.g. resource allocation).
   *
   * @return `true` on successful initialization; `false` if setup failed.
   */
  [[nodiscard]] virtual bool setup() = 0;


  /**
   * @brief Delete the underlying RTOS task handle.
   * This should be called when the task is stopping to clean up the RTOS task.
   */
  void deleteHandle();

  /**
   * @brief Configured stack depth used when creating the task.
   *
   * Set by subclasses or during creation. Value is in bytes.
   */
  uint32_t stackDepth{0};

  std::atomic<bool> stopRequested{false};

  void sleepFixedRate(TickType_t period);
  void sleepFixedDelay(TickType_t delay);

  /**
   * @brief Thin wrapper that creates an RTOS task.
   *
   * @param pxTaskCode Pointer to the FreeRTOS-compatible task function.
   * @param pcName     Task name used by the RTOS.
   * @param ulStackDepth Stack depth (bytes) to allocate for the task.
   * @param pvParameters Pointer passed to the task function.
   * @param uxPriority Task priority value.
   * @param xCoreID    Core id to pin the task to (use `RealTimeCore`/`NonRealTimeCore`).
   *
   * @return The created task handle cast to `BaseType_t` on success,
   *         or an error code/value on failure.
   */
  BaseType_t createTask(TaskFunction_t pxTaskCode,
                        const char* const pcName,
                        const uint32_t ulStackDepth,
                        void* const pvParameters,
                        UBaseType_t uxPriority,
                        const BaseType_t xCoreID);

 public:
  constexpr static BaseType_t RealTimeCore{0};

#if BOARD_NUM_CORES > 1
  constexpr static BaseType_t NonRealTimeCore{1};
#else
  constexpr static BaseType_t NonRealTimeCore{0};
#endif

  /**
   * @brief Task shutdown hook to implement by subclasses.
   *
   * This must be called from the task function with the while loop.
   * The last line must be deleteHandle(handle);
   */
  virtual void shutdown() = 0;

  /**
   * @name Priority levels
   * Predefined task priorities for convenience.
   * @{
   */
  constexpr static UBaseType_t LowPriority{1};
  constexpr static UBaseType_t MediumPriority{2};
  constexpr static UBaseType_t HighPriority{3};
  constexpr static UBaseType_t RealTimePriority{4};
  /** @} */

  /**
   * @brief Virtual destructor.
   *
   * Ensures derived classes are cleaned up properly. Does not stop
   * the underlying RTOS task automatically — call `stop()` to request stop.
   */
  ~Task();

  /**
   * @brief Query whether the task is currently running.
   *
   * @return `true` if the underlying RTOS task handle is valid and the task
   *         is believed to be running; `false` otherwise.
   */
  [[nodiscard]] bool isRunning() const;

  /**
   * @brief Start/create the task.
   *
   * This will create the underlying RTOS task using the configured
   * stack depth, priority and core settings. `setup()` will be called
   * as part of the task's startup sequence.
   *
   * @return `true` if the task was successfully started; `false` on error.
   */
  [[nodiscard]] bool start();

  /**
   * @brief Request the task to stop and perform shutdown.
   *
   * This triggers the `shutdown()` hook and attempts to delete the
   * underlying RTOS task. Behavior is implementation defined for
   * subclasses but should return once the task is no longer running.
   *
   */
  void stop();

  /**
   * @brief Check if the task has been requested to stop.
   *
   * @return `true` if the task is requested to stop; `false` otherwise.
   */
  [[nodiscard]] bool isStopRequested() const;

  /**
   * @brief Get the configured stack depth for this task.
   *
   * @return Stack depth in bytes.
   */
  [[nodiscard]] uint32_t getStackDepth() const { return stackDepth; }

  /**
   * @brief Get the runtime priority of the underlying RTOS task.
   *
   * This calls the RTOS helper to fetch the current priority.
   *
   * @return Priority value as `UBaseType_t`.
   */
  [[nodiscard]] UBaseType_t getPriority() const {
    return uxTaskPriorityGet(handle);
  }
  /**
   * @brief Get the core id the calling code is running on.
   *
   * Note: this returns the core id of the caller, not necessarily the task's pinned core.
   *
   * @return Core identifier as `UBaseType_t`.
   */
  [[nodiscard]] UBaseType_t getCore() const { return xPortGetCoreID(); }
  /**
   * @brief Query the task's current stack high water mark.
   *
   * The high water mark indicates remaining stack space and can be used
   * to detect stack pressure.
   *
   * @return The high water mark as `UBaseType_t`.
   */
  [[nodiscard]] UBaseType_t getStackHighWaterMark() const {
    return uxTaskGetStackHighWaterMark(handle);
  }

  void logStackHighWaterMark(dbg::TOPIC topic);
};

}  // namespace task
