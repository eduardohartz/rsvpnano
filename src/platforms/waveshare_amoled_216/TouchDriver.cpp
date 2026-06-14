#include "drivers/touch/TouchDriver.h"

#include "drivers/touch/cst92xx/cst92xx.h"

namespace TouchDriver {

TwoWire &wire() { return Wire; }

bool configure(uint8_t) { return true; }

bool usesInterruptGate() { return false; }

size_t packetLength() { return Cst92xxTouch::packetLength(); }

bool readPacket(uint8_t address, uint8_t *buffer, size_t len) {
  return Cst92xxTouch::readPacket(wire(), address, buffer, len);
}

bool decodePacket(const uint8_t *data, size_t len, Sample &sample) {
  return Cst92xxTouch::decodePacket(data, len, sample);
}

}  // namespace TouchDriver
