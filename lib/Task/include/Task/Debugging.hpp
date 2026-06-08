/**
 * @file Debugging.hpp
 * @brief Task wrapper and entry point for the project's debugger task.
 *
 * This header declares the `debuggerTask` entry function and the
 * `TaskDebugging` wrapper class which integrates the debugger as a
 * `Task` derived object. The wrapper exposes a compile-time stack
 * configuration and overrides the `setup()` / `shutdown()` lifecycle
 * hooks used by the project's task framework.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Task/Task.hpp>

/**
 * @namespace task
 * @brief Task subsystem: task wrappers and helpers used by the firmware.
 */
namespace task {



/**
 * @brief RTOS-compatible entry function for the debugger task.
 *
 * This function is intended to be used as the task entry when creating
 * the debugger task with the project's RTOS/task abstraction. The
 * implementation is responsible for running the debug loop and
 * returning only when the task should terminate.
 *
 * @param pvParameters Pointer to user-defined parameters passed to the task.
 *                     May be `nullptr` if unused.
 */
void debuggerTask(void* pvParameters);

/**
 * @class TaskDebugging
 * @brief Task wrapper that manages the debugger task lifecycle.
 *
 * `TaskDebugging` derives from `Task` and provides the necessary
 * overrides to initialize, run and tear down the debugger task used
 * by the firmware. It exposes a compile-time `STACK_DEPTH` that is
 * used when creating the underlying RTOS task.
 */
class TaskDebugging : public Task {

  /**
   * @brief Stack depth to allocate for the debugger task.
   */
  constexpr static uint32_t STACK_DEPTH{2048};

  /**
   * @brief Perform task-specific initialization.
   *
   * @return `true` if initialization succeeded and the task is ready
   *         to run; `false` on failure.
   */
  [[nodiscard]] bool setup() override;

  /**
   * @brief Cleanly shut down the debugger task and release resources.
   */
  void shutdown() override;
};

}  // namespace task
