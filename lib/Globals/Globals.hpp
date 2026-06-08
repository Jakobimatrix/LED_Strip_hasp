/**
 * @file Globals.hpp
 * @brief Global task priorities, logger queues and logger instances.
 *
 * This header exposes a small set of project-wide global symbols used by
 * various subsystems (WiFi, MQTT, LED handling and task scheduling). It
 * defines task priority constants, a typed single-producer/single-consumer
 * queue alias for debug messages and the external declarations for the
 * logger queues and logger objects used across the firmware.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <Arduino.h>

#include <Enums/ExpandEnums.hpp>
#include <Debugger/Constants.hpp>
#include <Debugger/Message.hpp>
#include <Debugger/Logger.hpp>
#include <Task/Task.hpp>
#include <Types/SpscQueue.hpp>


namespace glob {

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
constexpr UBaseType_t WIFI_TASK_PRIORITY{task::Task::MediumPriority};

/** @brief Priority for MQTT-related task(s). */
constexpr UBaseType_t MQTT_TASK_PRIORITY{task::Task::MediumPriority};

/** @brief Priority for debug/logging task(s). */
constexpr UBaseType_t DEBUG_TASK_PRIORITY{task::Task::LowPriority};

/** @brief Priority for LED driving tasks (real-time). */
constexpr UBaseType_t LED_TASK_PRIORITY{task::Task::RealTimePriority};
/*@}*/


/**
 * @brief Alias for a single-producer/single-consumer debug message queue.
 *
 * The queue stores `dbg::DebugMessage` entries with a compile-time
 * configurable message buffer size. The underlying queue implementation
 * is the project's `typ::SpscQueue`.
 *
 * @tparam Capacity Maximum number of messages the queue can hold.
 * @tparam MsgBufferSize Size in bytes of the internal per-message buffer
 *         used by `dbg::DebugMessage`.
 */
template <std::size_t Capacity, std::size_t MsgBufferSize>
using DebugQueue = typ::SpscQueue<Capacity, dbg::DebugMessage<MsgBufferSize>>;

/**
 * @name Logger Queues
 * @brief External queue instances used by subsystem loggers.
 *
 * These queues are defined in a single compilation unit and declared here
 * for global access. Each queue is dedicated to a subsystem to avoid
 * interleaving messages and to allow independent sizing.
 */
/*@{*/
/** @brief Queue for MQTT subsystem debug messages.*/
extern DebugQueue<16, 128> debugMqttQueue;

/** @brief Queue for WiFi subsystem debug messages.*/
extern DebugQueue<16, 128> debugWiFiQueue;

/** @brief Queue for LED subsystem debug messages.*/
extern DebugQueue<32, 128> debugLedQueue;

/** @brief Queue for generic task/debug messages.*/
extern DebugQueue<8, 62> debugTaskQueue;
/*@}*/


/**
 * @name Logger Instances
 * @brief Logger objects that publish to the corresponding debug queues.
 *
 * Each `dbg::Logger` instance is parameterized with the concrete queue
 * type it uses. The logger objects provide formatted logging and topic
 * filtering as implemented in `Debugger::Logger`.
 */
/*@{*/
/** @brief Logger instance for WiFi messages (writes to `debugWiFiQueue`). */
extern dbg::Logger<decltype(debugWiFiQueue)> dbgWiFiLogger;

/** @brief Logger instance for MQTT messages (writes to `debugMqttQueue`). */
extern dbg::Logger<decltype(debugMqttQueue)> dbgMqttLogger;

/** @brief Logger instance for LED subsystem messages (writes to `debugLedQueue`). */
extern dbg::Logger<decltype(debugLedQueue)> dbgLedLogger;

/** @brief Logger instance for general task/debug messages (writes to `debugTaskQueue`). */
extern dbg::Logger<decltype(debugTaskQueue)> dbgTaskLogger;
/*@}*/


}  // namespace glob
