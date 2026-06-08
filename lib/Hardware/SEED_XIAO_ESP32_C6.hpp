/**
 * @file SEED_XIAO_ESP32_C6.hpp
 * @brief Board-specific definitions for the Seeed XIAO ESP32-C6 target.
 *
 * This header provides minimal board-level constants and configuration
 * required by the project. It is intended to be included via the
 * central [lib/Hardware/Hardware.hpp](lib/Hardware/Hardware.hpp#L1) selection
 * mechanism where `BOARD_HEADER` names this file.
 *
 * The file purposely contains only small, stable definitions (macros)
 * and must not introduce inline functions or globals that change the
 * board's runtime behavior.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once

#include <cstddef>

#define BOARD_NUM_CORES  1
#define ON_BOARD_LED_PIN 5
