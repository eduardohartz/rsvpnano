#include "board/BoardPower.h"

#include <Wire.h>

#include "drivers/power/axp2101/Axp2101.h"
#include "drivers/gpio/tca9554/Tca9554.h"
#include "platforms/waveshare_amoled_18/WaveshareAmoled18.h"

namespace {

struct PowerContext {
  bool tca9554Sequenced = false;
};

PowerContext gPower;

constexpr BoardDrivers::Axp2101::Config kAxp2101Config = {
    WaveshareAmoled18::Axp2101Wiring::kReleaseBusBeforeRead,
    WaveshareAmoled18::Axp2101Wiring::kEnablePowerKeyIrqs,
    WaveshareAmoled18::Axp2101Wiring::kRequiresPowerKeyConfig,
    WaveshareAmoled18::Axp2101Wiring::kPowerKeyOnTimeValue,
    WaveshareAmoled18::Axp2101Wiring::kPowerKeyOffTimeValue,
};

void configureIoExpander(bool forceDisplaySequence = false) {
  BoardDrivers::Tca9554::PortState state = {};
  if (!BoardDrivers::Tca9554::readPortState(
          Wire1, WaveshareAmoled18::Tca9554Wiring::kAddress, state,
          WaveshareAmoled18::Tca9554Wiring::kReleaseBusBeforeRead)) {
    Serial.println("[board] TCA9554 not detected");
    return;
  }

  constexpr uint8_t displayMask = WaveshareAmoled18::Tca9554Wiring::kDisplayMask;
  const bool runDisplaySequence = forceDisplaySequence || !gPower.tca9554Sequenced;

  if (runDisplaySequence) {
    state.output &= WaveshareAmoled18::Tca9554Wiring::kDisplayClearMask;
  } else {
    state.output |= displayMask;
  }
  state.output |= WaveshareAmoled18::Tca9554Wiring::kSdEnableMask;
  state.config &= WaveshareAmoled18::Tca9554Wiring::kOutputClearMask;
  state.config |= WaveshareAmoled18::Tca9554Wiring::kInputMask;

  if (!BoardDrivers::Tca9554::writePortState(
          Wire1, WaveshareAmoled18::Tca9554Wiring::kAddress, state)) {
    Serial.println("[board] TCA9554 output setup failed");
    return;
  }
  if (!runDisplaySequence) {
    return;
  }

  if (forceDisplaySequence) {
    Serial.println("[board] TCA9554 display/touch wake sequence");
  }
  delay(20);
  state.output |= displayMask;
  if (!BoardDrivers::Tca9554::writeOutput(Wire1, WaveshareAmoled18::Tca9554Wiring::kAddress,
                                          state.output)) {
    Serial.println("[board] TCA9554 display release failed");
    return;
  }
  delay(50);
  gPower.tca9554Sequenced = true;
}

}  // namespace

namespace Board::Power {

void begin() {
  configureIoExpander();
  BoardDrivers::Axp2101::begin(kAxp2101Config);
}

void prepareDeepSleepPowerHold() {}

bool enableAudioPowerIfAvailable() { return true; }

bool readBatteryStatus(BatteryStatus &status) {
  return BoardDrivers::Axp2101::readBatteryStatus(status);
}

DiagnosticSnapshot diagnosticSnapshot() { return BoardDrivers::Axp2101::diagnosticSnapshot(); }

bool externalPowerPresent() { return BoardDrivers::Axp2101::externalPowerPresent(); }

bool releaseBatteryPowerHold() { return BoardDrivers::Axp2101::releasePower(); }

bool supportsSoftwarePowerOff() { return true; }

bool powerOffUsesControllerWake() { return WaveshareAmoled18::Power::kRequestPmuShutdownOnPowerOff; }

bool shouldRequestShutdownOnPowerOff() {
  return WaveshareAmoled18::Power::kRequestPmuShutdownOnPowerOff;
}

bool shouldReleaseBatteryPowerBeforeDeepSleep() {
  return WaveshareAmoled18::Power::kReleaseBatteryHoldBeforeDeepSleep;
}

}  // namespace Board::Power
