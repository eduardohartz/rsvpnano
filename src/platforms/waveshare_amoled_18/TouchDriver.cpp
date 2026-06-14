#include "drivers/touch/TouchDriver.h"

#include "board/BoardConfig.h"
#include "drivers/touch/ft6336/ft6336.h"

namespace TouchDriver {

TwoWire &wire() { return Wire; }

bool configure(uint8_t address) {
  TwoWire &touchWire = wire();
  touchWire.beginTransmission(address);
  touchWire.write(Board::Config::TOUCH_MONITOR_MODE_REGISTER);
  touchWire.write(Board::Config::TOUCH_MONITOR_MODE_VALUE);
  return touchWire.endTransmission(true) == 0;
}

bool usesInterruptGate() { return Board::Config::PIN_TOUCH_IRQ >= 0; }

size_t packetLength() { return Ft6336Touch::packetLength(); }

bool readPacket(uint8_t address, uint8_t *buffer, size_t len) {
  return Ft6336Touch::readPacket(wire(), address, buffer, len);
}

bool decodePacket(const uint8_t *data, size_t len, Sample &sample) {
  return Ft6336Touch::decodePacket(data, len, sample);
}

}  // namespace TouchDriver
