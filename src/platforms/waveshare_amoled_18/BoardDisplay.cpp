#include "board/BoardDisplay.h"

#include "drivers/display/sh8601/sh8601.h"

namespace Board::Display {

bool begin() {
  sh8601Init();
  return true;
}

void enablePowerIfAvailable() {}

void holdBacklightOffForDeepSleep() {}

void setBacklight(bool on) { sh8601SetDisplayOn(on); }

void flashBacklight(uint8_t count, uint32_t onMs, uint32_t offMs) {
  for (uint8_t i = 0; i < count; ++i) {
    setBacklight(true);
    delay(onMs);
    setBacklight(false);
    delay(offMs);
  }
}

void setBrightness(uint8_t percent) { sh8601SetBrightnessPercent(percent); }

void sleep() { sh8601Sleep(); }

void wake() { sh8601Wake(); }

bool pushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                const uint16_t *data) {
  sh8601PushColors(x, y, width, height, data);
  return true;
}

}  // namespace Board::Display
