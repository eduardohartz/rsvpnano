#pragma once

#include <Arduino.h>

#include "board/BoardTypes.h"

namespace Board::Power {

    using BatteryStatus = Board::BatteryStatus;
    using DiagnosticSnapshot = Board::PowerDiagnosticSnapshot;

    void begin();
    void prepareDeepSleepPowerHold();
    void resetWakePeripherals();
    bool enableAudioPowerIfAvailable();
    bool readBatteryStatus(BatteryStatus& status);
    DiagnosticSnapshot diagnosticSnapshot();
    bool externalPowerPresent();
    bool releaseBatteryPowerHold();
    bool supportsSoftwarePowerOff();
    bool powerOffUsesControllerWake();
    bool shouldRequestShutdownOnPowerOff();
    bool shouldReleaseBatteryPowerBeforeDeepSleep();
    bool usesRecoverableSoftOff();
    bool softOffWakeUsesPowerButton();
    bool softOffWakeUsesBootButton();
    uint32_t softOffWakeConfirmMs();

} // namespace Board::Power
