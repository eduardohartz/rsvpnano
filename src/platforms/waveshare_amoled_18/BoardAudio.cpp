#include "platforms/common/BoardEs8311Audio.h"

#include <Wire.h>

#include "board/BoardAudio.h"
#include "board/BoardConfig.h"
#include "drivers/audio/es8311/Es8311.h"

namespace {

BoardDrivers::Es8311::Config es8311Config() {
  BoardDrivers::Es8311::Config config;
  config.wire = &Wire;
  config.address = Board::Config::ES8311_ADDRESS;
  config.mclkPin = Board::Config::PIN_AUDIO_MCLK;
  config.bclkPin = Board::Config::PIN_AUDIO_BCLK;
  config.wsPin = Board::Config::PIN_AUDIO_WS;
  config.dataOutPin = Board::Config::PIN_AUDIO_DOUT;
  return config;
}

}  // namespace

namespace Board::Audio {

bool begin() { return BoardPlatform::Es8311Audio::begin(es8311Config()); }

bool beep() { return BoardPlatform::Es8311Audio::beep(); }

bool available() { return BoardPlatform::Es8311Audio::available(); }

}  // namespace Board::Audio
