/**
 * @file Hardware.hpp
 * @brief Central header for selecting and including the target board header.
 *
 * This header validates the build-time macro `BOARD_HEADER` and then
 * includes that header. The selected board header (for example
 * "SEED_XIAO_ESP32_C6.hpp") is expected to provide board-specific
 * definitions, pin mappings and any required hardware abstractions used
 * by the project.
 *
 * @date 08.06.2026
 * @author Jakob Wandel
 * @version 1.0
 */
#pragma once


// Standard-size type definitions and size constants
#include <cstddef>


/**
 * @def BOARD_HEADER
 * @brief Build-time macro naming the board header to include.
 *
 * `BOARD_HEADER` must be defined as a quoted header filename. For
 * example: `-DBOARD_HEADER="SEED_XIAO_ESP32_C6.hpp"` passed to the
 * compiler. The target header will be included below via `#include
 * BOARD_HEADER`.
 *
 * @note This macro must expand to a quoted string literal, not an
 *       angle-bracket include. The included file should exist in the
 *       project's include path.
 */
// BOARD_HEADER should be a quoted header filename, e.g. "SEED_XIAO_ESP32_C6.hpp"
#ifndef BOARD_HEADER
#error \
  "BOARD_HEADER must be defined as a quoted header filename (e.g. -DBOARD_HEADER=\"SEED_XIAO_ESP32_C6.hpp\")"
#endif


/**
 * @brief Include the selected board header.
 *
 * The chosen board header is expected to provide the concrete board
 * definitions used throughout the codebase. No additional symbols are
 * defined here — this file acts only as the canonical place to select
 * and include the board-specific header.
 */
#include BOARD_HEADER
