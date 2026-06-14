#include "board/BoardDisplay.h"

#include <Wire.h>
#include <driver/gpio.h>

#include "drivers/gpio/Tca9554.h"
#include "drivers/display/axs15231b/axs15231b.h"

namespace {

bool gBacklightEnableConfigured = false;

bool configureOutputPin(uint8_t pin, bool high) {
  return BoardDrivers::Tca9554::configureOutputPin(
      Wire1, static_cast<uint8_t>(Board::Config::TCA9554_ADDRESS), pin, high,
      Board::Config::TCA9554_RELEASE_BUS_BEFORE_READ);
}

}  // namespace

namespace Board::Display {

bool begin() {
  axs15231bInit();
  return true;
}

void enablePowerIfAvailable() {
  if (gBacklightEnableConfigured) {
    return;
  }

  if (!configureOutputPin(Config::TCA9554_PIN_BACKLIGHT_ENABLE, true)) {
    Serial.println("[board] TCA9554 backlight enable not configured");
    return;
  }

  gBacklightEnableConfigured = true;
  Serial.println("[board] Backlight enable configured");
}

void holdBacklightOffForDeepSleep() {
  if (!Config::HAS_LCD_BACKLIGHT || Config::PIN_LCD_BACKLIGHT < 0) {
    return;
  }

  const gpio_num_t backlightPin = static_cast<gpio_num_t>(Config::PIN_LCD_BACKLIGHT);
  analogWrite(Config::PIN_LCD_BACKLIGHT, 255);
  pinMode(Config::PIN_LCD_BACKLIGHT, OUTPUT);
  digitalWrite(Config::PIN_LCD_BACKLIGHT, HIGH);
  gpio_set_direction(backlightPin, GPIO_MODE_OUTPUT);
  gpio_set_level(backlightPin, 1);
  gpio_hold_en(backlightPin);
  gpio_deep_sleep_hold_en();
}

void setBacklight(bool on) { axs15231bSetBacklight(on); }

void flashBacklight(uint8_t count, uint32_t onMs, uint32_t offMs) {
  for (uint8_t i = 0; i < count; ++i) {
    setBacklight(true);
    delay(onMs);
    setBacklight(false);
    delay(offMs);
  }
}

void setBrightness(uint8_t percent) { axs15231bSetBrightnessPercent(percent); }

void sleep() { axs15231bSleep(); }

void wake() { axs15231bWake(); }

bool pushColors(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                const uint16_t *data) {
  axs15231bPushColors(x, y, width, height, data);
  return true;
}

}  // namespace Board::Display
