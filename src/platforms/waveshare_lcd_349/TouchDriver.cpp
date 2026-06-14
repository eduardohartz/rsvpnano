#include "drivers/touch/TouchDriver.h"

#include "board/BoardConfig.h"
#include "drivers/touch/axs15231b_touch/axs15231b_touch.h"

namespace TouchDriver {

TwoWire &wire() { return Wire; }

bool configure(uint8_t) { return true; }

bool usesInterruptGate() { return false; }

size_t packetLength() { return Axs15231bTouch::packetLength(); }

bool readPacket(uint8_t address, uint8_t *buffer, size_t len) {
  return Axs15231bTouch::readPacket(wire(), address, buffer, len);
}

bool decodePacket(const uint8_t *data, size_t len, Sample &sample) {
  return Axs15231bTouch::decodePacket(data, len, sample);
}

}  // namespace TouchDriver
