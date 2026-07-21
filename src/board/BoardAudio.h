#pragma once

#include <Arduino.h>

namespace Board::Audio {

    bool begin();
    bool beep();
    // Plays a short square tone (duration capped at 120 ms). Returns false when
    // the board has no speaker or the codec is unavailable.
    bool tone(uint32_t frequencyHz, uint32_t durationMs, int16_t amplitude);
    bool available();

} // namespace Board::Audio
