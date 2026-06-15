#pragma once

#include "board/BoardTypes.h"

namespace Board::Config {

using UiOrientation = Board::UiOrientation;

constexpr const char *BOARD_ID = "waveshare_esp32s3_touch_amoled_1_8";
constexpr const char *BOARD_LABEL = "Waveshare ESP32-S3-Touch-AMOLED-1.8";
constexpr const char *OTA_ASSET_NAME = "rsvp-nano-esp32-s3-touch-amoled-1.8-ota.bin";

constexpr bool ENABLE_TOP_EDGE_MENU_SWIPE = true;
constexpr bool ENABLE_BOTTOM_EDGE_QUICK_SETTINGS_SWIPE = true;
constexpr bool READER_SINGLE_TAP_PAUSES_WHILE_LOCKED = false;
constexpr bool TOUCH_READER_PLAYBACK_ENABLED = false;
constexpr bool ENABLE_RESTRUCTURED_MENU = true;

constexpr int PANEL_NATIVE_WIDTH = 368;
constexpr int PANEL_NATIVE_HEIGHT = 448;
constexpr int DISPLAY_WIDTH = 448;
constexpr int DISPLAY_HEIGHT = 368;
constexpr int READER_CHROME_MARGIN_X = 40;
constexpr int READER_CHROME_MARGIN_TOP = 24;
constexpr int READER_CHROME_MARGIN_BOTTOM = 24;
constexpr int READER_BATTERY_MARGIN_X = 64;
constexpr int READER_BATTERY_MARGIN_TOP = 32;

constexpr bool UI_ROTATED_180 = true;
constexpr UiOrientation DEFAULT_UI_ORIENTATION =
    UI_ROTATED_180 ? UiOrientation::LandscapeFlipped : UiOrientation::Landscape;
constexpr UiOrientation ROTATED_UI_ORIENTATION =
    UI_ROTATED_180 ? UiOrientation::Landscape : UiOrientation::LandscapeFlipped;

}  // namespace Board::Config
