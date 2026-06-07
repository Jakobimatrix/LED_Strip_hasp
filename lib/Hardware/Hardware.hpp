#pragma once

#include <cstddef>


// BOARD_HEADER should be a quoted header filename, e.g. "SEED_XIAO_ESP32_C6.hpp"
#ifndef BOARD_HEADER
#error \
  "BOARD_HEADER must be defined as a quoted header filename (e.g. -DBOARD_HEADER=\"SEED_XIAO_ESP32_C6.hpp\")"
#endif

#include BOARD_HEADER
