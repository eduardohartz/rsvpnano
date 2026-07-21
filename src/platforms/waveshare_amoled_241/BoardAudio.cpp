#include "board/BoardAudio.h"

namespace Board::Audio {

bool begin() { return false; }

bool beep() { return false; }

bool tone(uint32_t, uint32_t, int16_t) { return false; }

bool available() { return false; }

}  // namespace Board::Audio
