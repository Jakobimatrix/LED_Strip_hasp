#pragma once

#include <Types/SpscQueue.hpp>
#include <Debugger/Message.hpp>

namespace dbg {


template <std::size_t Capacity, std::size_t MsgBufferSize>
using DebugQueue = SpscQueue<Capacity, DebugMessage<MsgBufferSize>>;

static DebugQueue<16, 128> debugMqttQueue;
static DebugQueue<16, 128> debugWiFiQueue;
static DebugQueue<32, 128> debugLedQueue;
static DebugQueue<8, 62> debugTaskQueue;

}  // namespace dbg
