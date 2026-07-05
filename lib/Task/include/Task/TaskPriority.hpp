
/**
 * @file TaskPriority.hpp
 * @brief Task priority constants for subsystem tasks.

 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once
#include <Task/Task.hpp>

namespace task {
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
 * @name Task Priority Constants
 * @brief Recommended FreeRTOS priority levels for subsystem tasks.
 *
 * These constants map the subsystem names to `task::Task` priority levels
 * defined by the project's task utilities. They provide a single place to
 * adjust task priorities used when creating FreeRTOS tasks.
 */
/*@{*/
/** @brief Priority for WiFi-related task(s). */
constexpr UBaseType_t WIFI_TASK_PRIORITY{task::MediumPriority};

/** @brief Priority for MQTT-related task(s). */
constexpr UBaseType_t MQTT_TASK_PRIORITY{task::MediumPriority};

/** @brief Priority for debug/logging task(s). */
constexpr UBaseType_t DEBUG_TASK_PRIORITY{task::LowPriority};

/** @brief Priority for LED driving tasks (real-time). */
constexpr UBaseType_t LED_TASK_PRIORITY{task::RealTimePriority};
/*@}*/

}  // namespace task
