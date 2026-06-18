#pragma once

#include <Arduino.h>
#include <FS.h>
#include <driver/sdmmc_host.h>

namespace Board::Storage {

    enum class CardType : uint8_t {
        None = 0,
        Mmc = 1,
        Sd = 2,
        Sdhc = 3,
    };

    fs::FS& filesystem();
    bool mount(const char* mountPoint, int frequencyKhz);
    void end();
    uint64_t cardSize();
    CardType cardType();
    bool supportsFrequencySelection();
    bool setSdMmcPins();
    void configureSdMmcSlot(sdmmc_slot_config_t& slotConfig);

} // namespace Board::Storage
