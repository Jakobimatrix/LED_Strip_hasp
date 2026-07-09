#include <Debugger/Logger.hpp>

#include <mutex>

namespace glob {

dbg::LEVEL LogLevel = dbg::LEVEL::INFO | dbg::LEVEL::WARN | dbg::LEVEL::ERROR;
dbg::TOPIC LogTopicMask = dbg::TOPIC::MQTT | dbg::TOPIC::BOOT | dbg::TOPIC::DEBUG |
                          dbg::TOPIC::WIFI | dbg::TOPIC::WIFIPROV |
                          dbg::TOPIC::LED | dbg::TOPIC::PERF | dbg::TOPIC::TASK;

std::mutex sharedQueueMutex;

}  // namespace glob