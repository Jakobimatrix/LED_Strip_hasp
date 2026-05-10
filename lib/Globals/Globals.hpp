#pragma once

#include <Arduino.h>

#include <Task/Task.hpp>
#include <Debugger/Constants.hpp>
#include <Debugger/Queues.hpp>
#include <Debugger/Logger.hpp>

#include <Enums/ExpandEnums.hpp>


namespace glob {

constexpr UBaseType_t WIFI_TASK_PRIORITY{task::Task::MediumPriority};
constexpr UBaseType_t MQTT_TASK_PRIORITY{task::Task::MediumPriority};
constexpr UBaseType_t DEBUG_TASK_PRIORITY{task::Task::LowPriority};
constexpr UBaseType_t LED_TASK_PRIORITY{task::Task::RealTimePriority};

dbg::Logger dbgLogger{dbg::debugTaskQueue};
dbg::Logger dbgWiFiLogger{dbg::debugWiFiQueue};
dbg::Logger dbgMqttLogger{dbg::debugMqttQueue};
dbg::Logger dbgLedLogger{dbg::debugLedQueue};
dbg::Logger dbgTaskLogger{dbg::debugTaskQueue};

LEVEL globalLogLevel = LEVEL::INFO | LEVEL::WARN | LEVEL::ERROR;
TOPIC globalLogTopicMask = TOPIC::MQTT | TOPIC::BOOT | TOPIC::DEBUG | TOPIC::WIFI |
                           TOPIC::LED | TOPIC::SCHEDULE | TOPIC::PERF;

}  // namespace glob
