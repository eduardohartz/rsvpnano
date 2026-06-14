#include "board/BoardDisplay.h"

#include "drivers/display/rm690b0/rm690b0.h"

namespace Board::Display {

bool begin() {
  rm690b0Init();
  return true;
}

void enablePowerIfAvailable() {}

void holdBacklightOffForDeepSleep() {}

void setBacklight(bool on) { rm690b0SetDisplayOn(on); }

void flashBacklight(uint8_t count, uint32_t onMs, uint32_t offMs) {
  for (uint8_t i = 0; i < count; ++i) {
    setBacklight(true);
    delay(onMs);
    setBacklight(false);
    delay(offMs);
  }
}

void setBrightness(uint8_t percent) { rm690b0SetBrightnessPercent(percent); }

void sleep() { rm690b0Sleep(); }

void wake() { rm690b0Wake(); }

bool pushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                const uint16_t *data) {
  rm690b0PushColors(x, y, width, height, data);
  return true;
}

}  // namespace Board::Display
