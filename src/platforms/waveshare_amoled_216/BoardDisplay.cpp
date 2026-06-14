#include "board/BoardDisplay.h"

#include "drivers/display/co5300/co5300.h"

namespace Board::Display {

bool begin() {
  co5300Init();
  return true;
}

void enablePowerIfAvailable() {}

void holdBacklightOffForDeepSleep() {}

void setBacklight(bool on) { co5300SetDisplayOn(on); }

void flashBacklight(uint8_t count, uint32_t onMs, uint32_t offMs) {
  for (uint8_t i = 0; i < count; ++i) {
    setBacklight(true);
    delay(onMs);
    setBacklight(false);
    delay(offMs);
  }
}

void setBrightness(uint8_t percent) { co5300SetBrightnessPercent(percent); }

void sleep() { co5300Sleep(); }

void wake() { co5300Wake(); }

bool pushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                const uint16_t *data) {
  co5300PushColors(x, y, width, height, data);
  return true;
}

}  // namespace Board::Display
