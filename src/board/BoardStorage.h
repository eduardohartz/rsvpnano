#pragma once

#include <Arduino.h>
#include <FS.h>
#include <driver/sdmmc_host.h>

namespace Board::Storage {

    fs::FS& filesystem();
    bool mount(const char* mountPoint, int frequencyKhz);
    void end();
    uint64_t cardSize();
    uint8_t cardType();
    bool supportsFrequencySelection();
    bool setSdMmcPins();
    void configureSdMmcSlot(sdmmc_slot_config_t& slotConfig);

} // namespace Board::Storage
