#include "board/BoardSystem.h"
#include "board/BoardTouch.h"

#include <algorithm>

#include "drivers/touch/TouchDriver.h"

namespace Board::Touch {

namespace {

constexpr uint8_t kAddress = Config::TOUCH_I2C_ADDRESS;
constexpr uint32_t kPollIntervalMs = Config::TOUCH_POLL_INTERVAL_MS;
constexpr uint32_t kFailureBackoffMs = Config::TOUCH_FAILURE_BACKOFF_MS;
constexpr uint32_t kRecoveryRetryMs = Config::TOUCH_RECOVERY_RETRY_MS;
constexpr uint8_t kReleaseConfirmSamples = 2;

bool gInitialized = false;
uint32_t gLastPollMs = 0;
uint32_t gBackoffUntilMs = 0;
uint32_t gIgnoreEventsUntilMs = 0;
uint32_t gLastSampleMs = 0;
uint8_t gConsecutiveReadFailures = 0;
uint8_t gEmptySamples = 0;
bool gActive = false;
Config::UiOrientation gUiOrientation = Config::DEFAULT_UI_ORIENTATION;
uint16_t gLastX = 0;
uint16_t gLastY = 0;

uint16_t clampDisplayX(uint16_t x) {
  return std::min<uint16_t>(x, static_cast<uint16_t>(Config::DISPLAY_WIDTH - 1));
}

uint16_t clampDisplayY(uint16_t y) {
  return std::min<uint16_t>(y, static_cast<uint16_t>(Config::DISPLAY_HEIGHT - 1));
}

bool touchInterruptActive() {
  return Config::PIN_TOUCH_IRQ < 0 || !digitalRead(Config::PIN_TOUCH_IRQ);
}

}  // namespace

bool begin() {
  gLastPollMs = 0;
  gBackoffUntilMs = 0;
  gIgnoreEventsUntilMs = 0;
  gLastSampleMs = 0;
  gConsecutiveReadFailures = 0;
  gEmptySamples = 0;
  gActive = false;
  gLastX = 0;
  gLastY = 0;

  System::resetTouchController();

  TwoWire &wire = TouchDriver::wire();
  wire.beginTransmission(kAddress);
  const uint8_t error = wire.endTransmission();
  gInitialized = (error == 0);

  if (!gInitialized) {
    Serial.printf("[touch] Controller not detected at 0x%02X\n", kAddress);
  } else {
    Serial.printf("[touch] Initialized controller=0x%02X\n", kAddress);
    if (Config::TOUCH_RECOVERY_EVENT_IGNORE_MS > 0) {
      gIgnoreEventsUntilMs = millis() + Config::TOUCH_RECOVERY_EVENT_IGNORE_MS;
      Serial.printf("[touch] Ignoring events for %lu ms after init\n",
                    static_cast<unsigned long>(Config::TOUCH_RECOVERY_EVENT_IGNORE_MS));
    }
    if (Config::TOUCH_REQUIRES_MONITOR_MODE) {
      if (TouchDriver::configure(kAddress)) {
        Serial.printf("[touch] Applied monitor mode reg=0x%02X value=0x%02X\n",
                      Config::TOUCH_MONITOR_MODE_REGISTER, Config::TOUCH_MONITOR_MODE_VALUE);
      } else {
        Serial.println("[touch] Failed to apply board-specific monitor mode");
      }
    }
  }

  return gInitialized;
}

void end() {
  cancel();
  gInitialized = false;
}

void cancel() {
  gActive = false;
  gLastPollMs = 0;
  gBackoffUntilMs = 0;
  gIgnoreEventsUntilMs = 0;
  gLastSampleMs = 0;
  gConsecutiveReadFailures = 0;
  gEmptySamples = 0;
}

bool readEvent(Event &event) {
  event = Event{};

  const uint32_t now = millis();

  if (!gInitialized) {
    if (now < gBackoffUntilMs) {
      return false;
    }

    Serial.println("[touch] Attempting controller recovery");
    if (!begin()) {
      gBackoffUntilMs = now + kRecoveryRetryMs;
    }
    return false;
  }

  if (now < gBackoffUntilMs) {
    return false;
  }

  if (now - gLastPollMs < kPollIntervalMs) {
    return false;
  }
  gLastPollMs = now;

  auto releaseTouchIfNeeded = [&]() -> bool {
    if (!gActive) {
      return false;
    }

    ++gEmptySamples;
    if (gEmptySamples < kReleaseConfirmSamples) {
      return false;
    }

    gActive = false;
    gEmptySamples = 0;
    event.touched = false;
    event.x = gLastX;
    event.y = gLastY;
    event.phase = Phase::End;
    return true;
  };

  const size_t packetLength = TouchDriver::packetLength();
  if (TouchDriver::usesInterruptGate() && !touchInterruptActive()) {
    return releaseTouchIfNeeded();
  }

  uint8_t data[15] = {0};
  if (!TouchDriver::readPacket(kAddress, data, packetLength)) {
    gBackoffUntilMs = now + kFailureBackoffMs;
    if (++gConsecutiveReadFailures >= 5) {
      gInitialized = false;
      gActive = false;
      gEmptySamples = 0;
      gBackoffUntilMs = now + kRecoveryRetryMs;
      Serial.println("[touch] Read failed repeatedly, scheduling controller recovery");
    }
    return false;
  }

  gConsecutiveReadFailures = 0;

  TouchDriver::Sample sample;
  if (!TouchDriver::decodePacket(data, packetLength, sample)) {
    gBackoffUntilMs = now + kFailureBackoffMs;
    return false;
  }
  if (!sample.touched) {
    return releaseTouchIfNeeded();
  }

  if (now < gIgnoreEventsUntilMs) {
    gActive = false;
    gEmptySamples = 0;
    return false;
  }

  gBackoffUntilMs = 0;
  gConsecutiveReadFailures = 0;
  gEmptySamples = 0;
  gLastSampleMs = now;

  event.touched = true;
  event.gesture = 0;
  event.phase = gActive ? Phase::Move : Phase::Start;

  const uint16_t physicalX = sample.physicalX;
  const uint16_t physicalY = sample.physicalY;

  switch (gUiOrientation) {
    case Config::UiOrientation::Portrait:
      event.x = physicalX;
      event.y = physicalY;
      break;
    case Config::UiOrientation::PortraitFlipped:
      event.x = static_cast<uint16_t>(Config::PANEL_NATIVE_WIDTH - 1 - physicalX);
      event.y = static_cast<uint16_t>(Config::PANEL_NATIVE_HEIGHT - 1 - physicalY);
      break;
    case Config::UiOrientation::Landscape:
      event.x = clampDisplayX(static_cast<uint16_t>(Config::PANEL_NATIVE_HEIGHT - 1 - physicalY));
      event.y = clampDisplayY(physicalX);
      break;
    case Config::UiOrientation::LandscapeFlipped:
    default:
      event.x = clampDisplayX(physicalY);
      event.y = clampDisplayY(static_cast<uint16_t>(Config::PANEL_NATIVE_WIDTH - 1 - physicalX));
      break;
  }
  gActive = true;
  gLastX = event.x;
  gLastY = event.y;

  return true;
}

void setUiOrientation(Config::UiOrientation orientation) {
  if (gUiOrientation == orientation) {
    return;
  }

  gUiOrientation = orientation;
  cancel();
}

void setUiRotated180(bool rotated180) {
  setUiOrientation(rotated180 ? Config::ROTATED_UI_ORIENTATION : Config::DEFAULT_UI_ORIENTATION);
}

}  // namespace Board::Touch
