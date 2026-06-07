#pragma once

#include <Arduino.h>

#include <Enums/ExpandEnums.hpp>
#include <Debugger/Constants.hpp>
#include <Debugger/Message.hpp>
#include <Debugger/Logger.hpp>
#include <Task/Task.hpp>
#include <Types/SpscQueue.hpp>


namespace glob {

// Defines
constexpr UBaseType_t WIFI_TASK_PRIORITY{task::Task::MediumPriority};
constexpr UBaseType_t MQTT_TASK_PRIORITY{task::Task::MediumPriority};
constexpr UBaseType_t DEBUG_TASK_PRIORITY{task::Task::LowPriority};
constexpr UBaseType_t LED_TASK_PRIORITY{task::Task::RealTimePriority};


// QUEUES for Logger
template <std::size_t Capacity, std::size_t MsgBufferSize>
using DebugQueue = typ::SpscQueue<Capacity, dbg::DebugMessage<MsgBufferSize>>;

extern DebugQueue<16, 128> debugMqttQueue;
extern DebugQueue<16, 128> debugWiFiQueue;
extern DebugQueue<32, 128> debugLedQueue;
extern DebugQueue<8, 62> debugTaskQueue;

// LOGGERS
extern dbg::Logger<decltype(debugWiFiQueue)> dbgWiFiLogger;
extern dbg::Logger<decltype(debugMqttQueue)> dbgMqttLogger;
extern dbg::Logger<decltype(debugLedQueue)> dbgLedLogger;
extern dbg::Logger<decltype(debugTaskQueue)> dbgTaskLogger;

// Global log level and topic mask
extern dbg::LEVEL LogLevel;
extern dbg::TOPIC LogTopicMask;

}  // namespace glob
