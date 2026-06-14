#include "board/BoardButtons.h"

#include <Wire.h>

#include "drivers/gpio/Tca9554.h"

namespace {

bool gExpanderPowerButtonStableHeld = false;
bool gExpanderPowerButtonRawHeld = false;
uint32_t gExpanderPowerButtonRawChangedMs = 0;
uint32_t gExpanderPowerButtonLastPollMs = 0;
bool gExpanderPowerButtonSeen = false;

constexpr uint32_t kExpanderPowerButtonPollIntervalMs = 25;
constexpr uint32_t kExpanderPowerButtonDebounceMs = 70;

}  // namespace

namespace Board::Buttons {

bool readVirtualBootHeld() { return false; }

bool readVirtualPowerHeld() {
  const uint32_t nowMs = millis();
  if (gExpanderPowerButtonSeen &&
      nowMs - gExpanderPowerButtonLastPollMs < kExpanderPowerButtonPollIntervalMs) {
    return gExpanderPowerButtonStableHeld;
  }
  gExpanderPowerButtonLastPollMs = nowMs;

  bool held = false;
  if (!BoardDrivers::Tca9554::readInputPin(
          Wire1, static_cast<uint8_t>(Board::Config::TCA9554_ADDRESS),
          Board::Config::TCA9554_PIN_PWR_BUTTON, held,
          Board::Config::TCA9554_RELEASE_BUS_BEFORE_READ)) {
    return gExpanderPowerButtonStableHeld;
  }

  if (!gExpanderPowerButtonSeen) {
    gExpanderPowerButtonSeen = true;
    gExpanderPowerButtonRawHeld = held;
    gExpanderPowerButtonStableHeld = held;
    gExpanderPowerButtonRawChangedMs = nowMs;
    return gExpanderPowerButtonStableHeld;
  }

  if (held != gExpanderPowerButtonRawHeld) {
    gExpanderPowerButtonRawHeld = held;
    gExpanderPowerButtonRawChangedMs = nowMs;
    return gExpanderPowerButtonStableHeld;
  }

  if (held != gExpanderPowerButtonStableHeld &&
      nowMs - gExpanderPowerButtonRawChangedMs >= kExpanderPowerButtonDebounceMs) {
    gExpanderPowerButtonStableHeld = held;
  }

  return gExpanderPowerButtonStableHeld;
}

bool consumeVirtualPowerShortPress() { return false; }
bool consumeVirtualPowerLongPress() { return false; }
bool usesPowerEvents() { return Config::APP_POWER_BUTTON_USES_PMU_EVENTS; }
uint32_t powerEventIgnoreMs() { return Config::PMU_BOOT_BUTTON_IGNORE_MS; }

}  // namespace Board::Buttons
