#pragma once

#include <Debugger/Constants.hpp>
#include <Debugger/Format.hpp>
#include <Debugger/Message.hpp>
#include <Debugger/Types.hpp>
#include <Types/StaticString.hpp>

#include <tuple>
#include <utility>

namespace dbg {


template <typename MapT, typename Key>
constexpr const char* lookup(const MapT& map, const Key& key) {
  const auto it = map.find(key);
  return (it != map.end()) ? it->second : "[?]";
}

constexpr typ::StaticString<19> timestamp2string(int64_t timestamp) {
  // format: MM-dd/HH:mm:SS:ms

  int64_t ms  = timestamp % 1000;
  timestamp  /= 1000;
  int64_t s   = timestamp % 60;
  timestamp  /= 60;
  int64_t m   = timestamp % 60;
  timestamp  /= 60;
  int64_t h   = timestamp % 24;
  timestamp  /= 24;
  int64_t d   = (timestamp % 31) + 1;
  timestamp  /= 31;
  int64_t M   = (timestamp % 12) + 1;

  typ::StaticString<19> result;
  result[0]  = '0' + (M / 10);
  result[1]  = '0' + (M % 10);
  result[2]  = '-';
  result[3]  = '0' + (d / 10);
  result[4]  = '0' + (d % 10);
  result[5]  = '/';
  result[6]  = '0' + (h / 10);
  result[7]  = '0' + (h % 10);
  result[8]  = ':';
  result[9]  = '0' + (m / 10);
  result[10] = '0' + (m % 10);
  result[11] = ':';
  result[12] = '0' + (s / 10);
  result[13] = '0' + (s % 10);
  result[14] = ':';
  result[15] = '0' + (ms / 100);
  result[16] = '0' + ((ms / 10) % 10);
  result[17] = '0' + (ms % 10);
  result[18] = '\0';

  return result;
}


template <typename Tuple, typename Func, std::size_t... Is>
void tuple_runtime_visit_impl(Tuple&& tuple, size_t index, Func&& func, std::index_sequence<Is...>) {
  ((index == Is ? (func(std::get<Is>(tuple)), true) : false) || ...);
}

template <typename Tuple, typename Func>
void tuple_runtime_visit(Tuple&& tuple, size_t index, Func&& func) {
  constexpr size_t N = std::tuple_size_v<std::remove_reference_t<Tuple>>;

  tuple_runtime_visit_impl(std::forward<Tuple>(tuple),
                           index,
                           std::forward<Func>(func),
                           std::make_index_sequence<N>{});
}


template <typename... Queues>
class Serializer {

  std::tuple<Queues&...> queues_;

  constexpr static std::size_t N = sizeof...(Queues);

  template <typename DebugMessageType>
  void print(const DebugMessageType& msg) const {
    const auto timestamp_str = timestamp2string(msg.timestamp);
    std::printf("%s%s %s",
                lookup(LEVEL_LOOKUP, msg.level),
                lookup(TOPIC_LOOKUP, msg.topic),
                timestamp_str.data());

    const uint8_t* ptr = msg.buffer.data.data();
    std::size_t pos    = 0;

    typ::StaticString<DebugMessageType::MSG_BUFFER_SIZE> fmt_str;
    const auto& fmt = Format<Msg::format>{};

    for (std::size_t i = 0; i < fmt.count; ++i) {
      const auto& tok = fmt.tokens[i];

      if (tok.type == TokenType::Text) {
        std::printf("%.*s", tok.length, msg_format_data<Msg>() + tok.offset);
      } else {
        const auto type = static_cast<ArgType>(ptr[pos++]);

        switch (type) {

          case ArgType::Bool: {
            bool v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%s", v ? "true" : "false");
            break;
          }

          case ArgType::Char: {
            char v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%c", v);
            break;
          }

          case ArgType::SignedChar: {
            signed char v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%hhd", v);
            break;
          }

          case ArgType::UnsignedChar: {
            unsigned char v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%hhu", v);
            break;
          }

          case ArgType::Int16: {
            short v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%hd", v);
            break;
          }

          case ArgType::UInt16: {
            unsigned short v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%hu", v);
            break;
          }

          case ArgType::Int32: {
            int32_t v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%" PRId32, v);
            break;
          }

          case ArgType::UInt32: {
            uint32_t v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%" PRIu32, v);
            break;
          }

          case ArgType::Int64: {
            int64_t v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%" PRId64, v);
            break;
          }

          case ArgType::UInt64: {
            uint64_t v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%" PRIu64, v);
            break;
          }

          case ArgType::Float32: {
            float v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%f", static_cast<double>(v));
            break;
          }

          case ArgType::Float64: {
            double v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%f", v);
            break;
          }

          case ArgType::CString: {
            uint16_t len;
            std::memcpy(&len, ptr + pos, sizeof(len));
            pos += sizeof(len);

            std::printf(
              "%.*s", static_cast<int>(len), reinterpret_cast<const char*>(ptr + pos));

            pos += len;
            break;
          }

          case ArgType::Level: {
            LEVEL v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%s", lookup(LEVEL_LOOKUP, v));
            break;
          }

          case ArgType::Topic: {
            TOPIC v;
            std::memcpy(&v, ptr + pos, sizeof(v));
            pos += sizeof(v);

            std::printf("%s", lookup(TOPIC_LOOKUP, v));
            break;
          }

          default:
            break;
        }
      }
    }

    std::putchar('\n');
  }

  std::size_t getQueueWithOldestMessage() const {
    int64_t earliest_timestamp = std::numeric_limits<int64_t>::max();
    std::size_t earliest_index = std::numeric_limits<std::size_t>::max();

    void compare_timestamps = [this, &earliest_timestamp, &earliest_index](
                                const auto& queue, std::size_t index) {
      if (!queue.empty()) {
        const auto& front_msg = queue.peakFront();
        if (front_msg.timestamp < earliest_timestamp) {
          earliest_timestamp = front_msg.timestamp;
          earliest_index     = index;
        }
      }
    };
    return earliest_index;
  }

 public:
  template <typename... Queues>
  explicit Serializer(Queues&... qs)
      : queues_{qs...} {}

  void debugQueueLoads() const {
    void debugQueue = [](const auto& q) {
      const auto load = static_cast<float>(q.size()) / static_cast<float>(q.capacity());
      if (load > 0.8f) {
        std::printf("%s%s Queue %zu load is high: %.2f%%\n",
                    lookup(LEVEL_LOOKUP, LEVEL::WARN),
                    lookup(TOPIC_LOOKUP, TOPIC::DEBUG),
                    i,
                    load * 100.0f);
      }
    };

    std::apply([this](auto&... q) { (debugQueue(q), ...); }, queues_);
  }

  template <typename DebugMessageType>
  void debugMessageBufferSizeUsage(const DebugMessageType& msg) const {
    const auto used     = msg.buffer.size;
    const auto capacity = msg.buffer.capacity();
    const auto usage = static_cast<float>(used) / static_cast<float>(capacity);
    if (usage > 0.95f) {
      std::printf("%s%s Message buffer usage is high: %zu/%zu bytes (%.2f%%)\n",
                  lookup(LEVEL_LOOKUP, LEVEL::WARN),
                  lookup(TOPIC_LOOKUP, TOPIC::DEBUG),
                  used,
                  capacity,
                  usage * 100.0f);
    }
  }


  [[nodiscard]] bool printNext() {
    const size_t earliest_index = getQueueWithOldestMessage();
    if (earliest_index == std::numeric_limits<std::size_t>::max()) {
      return false;
    }
    tuple_runtime_visit(queues_, earliest_index, [](auto& queue) {
      using Queue = std::remove_reference_t<decltype(queue)>;
      using Msg   = typename Queue::ValueType;

      Msg msg{};
      queue.pop(msg);
      print(msg);
      debugMessageBufferSizeUsage(msg);
    });

    debugQueueLoads();
    return true;
  }
};

}  // namespace dbg
