// Implementation file for Serializer.hpp templates

template <typename MapT, typename Key>
constexpr const char* lookup(const MapT& map, const Key& key) {
  const auto it = map.find(key);
  return (it != map.end()) ? it->second : "[?]";
}

template <typename Tuple, typename Func, std::size_t... Is>
void tuple_runtime_visit_impl(Tuple&& tuple, size_t index, Func&& func, std::index_sequence<Is...>) {
  // Internal implementation: expand the index sequence and call `func`
  // for the matching element index at runtime.
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
Serializer<Queues...>::Serializer(Queues&... qs)
    : queues_{qs...} {}

template <typename... Queues>
template <typename DebugMessageType>
void Serializer<Queues...>::print(const DebugMessageType& msg, std::size_t queue_index) const {
  const auto timestamp_str = timestamp2string(msg.timestamp);
  std::printf("[%zu]%s%s %s ",
              queue_index,
              lookup(LEVEL_LOOKUP, msg.level),
              lookup(TOPIC_LOOKUP, msg.topic),
              timestamp_str.c_str());

  const uint8_t* ptr = msg.buffer.data.data();
  std::size_t pos    = 0;

  while (pos < msg.buffer.size) {
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

        std::printf("%.*s", static_cast<int>(len), reinterpret_cast<const char*>(ptr + pos));
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

    if (pos < msg.buffer.size) {
      std::printf(" ");
    }
  }
  std::putchar('\n');
}

template <typename... Queues>
std::size_t Serializer<Queues...>::getQueueWithOldestMessage() const {

  int64_t earliest_timestamp = std::numeric_limits<int64_t>::max();
  std::size_t earliest_index = std::numeric_limits<std::size_t>::max();

  auto compare_timestamps = [&](const auto& queue, std::size_t index) {
    if (!queue.empty()) {
      const auto& front_msg = queue.peakFront();
      if (front_msg.timestamp < earliest_timestamp) {
        earliest_timestamp = front_msg.timestamp;
        earliest_index     = index;
      }
    }
  };

  std::size_t index = 0;
  std::apply(
    [&](const auto&... queues) { (compare_timestamps(queues, index++), ...); }, queues_);
  return earliest_index;
}

template <typename... Queues>
void Serializer<Queues...>::debugQueueLoads() {
  std::size_t index = 0;
  auto debugQueue   = [&](auto& q) {
    const auto load = static_cast<float>(q.size()) / static_cast<float>(q.capacity());
    if (load > 0.8f) {
      std::printf("%s%s Queue %zu load is high: %.2f%%\n",
                  lookup(LEVEL_LOOKUP, LEVEL::WARN),
                  lookup(TOPIC_LOOKUP, TOPIC::DEBUG),
                  index,
                  load * 100.0f);
    }
    const size_t dropped = q.getNumDroppedMessages();
    if (dropped > 0) {
      std::printf(
        "%s%s Queue %zu has dropped %zu messages due to full queue.\n",
        lookup(LEVEL_LOOKUP, LEVEL::WARN),
        lookup(TOPIC_LOOKUP, TOPIC::DEBUG),
        index,
        dropped);
      q.resetNumDroppedMessages();
    }
    ++index;
  };

  std::apply([&](auto&... q) { (debugQueue(q), ...); }, queues_);
}


template <typename... Queues>
template <typename DebugMessageType>
void Serializer<Queues...>::debugMessageBufferSizeUsage(const DebugMessageType& msg,
                                                        std::size_t queue_index) const {

  const auto used     = msg.buffer.size;
  const auto capacity = msg.buffer.capacity();
  const auto usage    = static_cast<float>(used) / static_cast<float>(capacity);
  if (usage > 0.95f) {
    std::printf(
      "%s%s Message buffer of queue %zu usage is high: %zu/%zu bytes "
      "(%.2f%%)\n",
      lookup(LEVEL_LOOKUP, LEVEL::WARN),
      lookup(TOPIC_LOOKUP, TOPIC::DEBUG),
      queue_index,
      used,
      capacity,
      usage * 100.0f);
  }
}


template <typename... Queues>
[[nodiscard]] bool Serializer<Queues...>::printNext() {

  const size_t earliest_index = getQueueWithOldestMessage();
  if (earliest_index == std::numeric_limits<std::size_t>::max()) {
    return false;
  }
  tuple_runtime_visit(queues_, earliest_index, [this, earliest_index](auto& queue) {
    using Queue = std::remove_reference_t<decltype(queue)>;
    using Msg   = typename Queue::ValueType;

    Msg msg{};
    if (!queue.pop(msg)) {
      return;
    }
    print(msg, earliest_index);
    debugMessageBufferSizeUsage(msg, earliest_index);
  });

  debugQueueLoads();
  return true;
}
