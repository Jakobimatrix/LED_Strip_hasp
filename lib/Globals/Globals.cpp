#include <Globals.hpp>

namespace glob {

Preferences prefs;

DebugQueue<16, 128> debugMqttQueue{};
DebugQueue<16, 128> debugWiFiQueue{};
DebugQueue<32, 128> debugLedQueue{};
DebugQueue<8, 62> debugTaskQueue{};

dbg::Logger<decltype(debugWiFiQueue)> dbgWiFiLogger{debugWiFiQueue};
dbg::Logger<decltype(debugMqttQueue)> dbgMqttLogger{debugMqttQueue};
dbg::Logger<decltype(debugLedQueue)> dbgLedLogger{debugLedQueue};
dbg::Logger<decltype(debugTaskQueue)> dbgTaskLogger{debugTaskQueue};

}  // namespace glob
