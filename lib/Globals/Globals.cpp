#include <Globals.hpp>

namespace glob {

DebugQueue<16, 128> debugMqttQueue{};
DebugQueue<16, 128> debugWiFiQueue{};
DebugQueue<32, 128> debugLedQueue{};
DebugQueue<8, 62> debugTaskQueue{};

dbg::Logger<decltype(debugWiFiQueue)> dbgWiFiLogger{debugWiFiQueue};
dbg::Logger<decltype(debugMqttQueue)> dbgMqttLogger{debugMqttQueue};
dbg::Logger<decltype(debugLedQueue)> dbgLedLogger{debugLedQueue};
dbg::Logger<decltype(debugTaskQueue)> dbgTaskLogger{debugTaskQueue};

dbg::LEVEL LogLevel = dbg::LEVEL::INFO | dbg::LEVEL::WARN | dbg::LEVEL::ERROR;
dbg::TOPIC LogTopicMask = dbg::TOPIC::MQTT | dbg::TOPIC::BOOT |
                            dbg::TOPIC::DEBUG | dbg::TOPIC::WIFI | dbg::TOPIC::LED |
                            dbg::TOPIC::SCHEDULE | dbg::TOPIC::PERF;

}  // namespace glob
