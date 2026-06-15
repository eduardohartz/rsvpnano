#pragma once

#include <Arduino.h>

namespace Board::System {

    void begin();
    void lightSleepUntilBootButton();
    void holdBacklightOffForDeepSleep();
    void resetWakePeripherals();
    void deepSleepUntilConfiguredWake();
    const char* wakeLabel(bool useRecoverableSoftOff, bool externalPowerPresent);
    void logStartupDiagnostics();

} // namespace Board::System
