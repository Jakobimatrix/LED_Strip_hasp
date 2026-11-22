#include <Globals.hpp>

namespace glob {

DebugQueue<16, 128> debugMqttQueue{};
DebugQueue<16, 128> debugWiFiQueue{};
DebugQueue<16, 128> debugWiFiProvQueue{};
DebugQueue<32, 128> debugLedQueue{};
DebugQueue<4, 128> debugMeasurementsQueue{};
DebugQueue<16, 128> sharedQueue{};

dbg::Logger<decltype(debugWiFiQueue)> dbgWiFiLogger{debugWiFiQueue};
dbg::Logger<decltype(debugWiFiProvQueue)> dbgWiFiProvLogger{debugWiFiProvQueue};
dbg::Logger<decltype(debugMqttQueue)> dbgMqttLogger{debugMqttQueue};
dbg::Logger<decltype(debugLedQueue)> dbgLedLogger{debugLedQueue};
dbg::Logger<decltype(debugMeasurementsQueue)> dbgMeasurementsLogger{debugMeasurementsQueue};
dbg::Logger<decltype(sharedQueue), true> dbgTaskLogger{sharedQueue};
}  // namespace glob
