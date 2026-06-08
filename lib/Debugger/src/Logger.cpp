#include <Debugger/Logger.hpp>

namespace glob {

dbg::LEVEL LogLevel = dbg::LEVEL::INFO | dbg::LEVEL::WARN | dbg::LEVEL::ERROR;
dbg::TOPIC LogTopicMask = dbg::TOPIC::MQTT | dbg::TOPIC::BOOT |
                          dbg::TOPIC::DEBUG | dbg::TOPIC::WIFI | dbg::TOPIC::LED |
                          dbg::TOPIC::SCHEDULE | dbg::TOPIC::PERF;

}  // namespace glob