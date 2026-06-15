#pragma once

#include <Arduino.h>
#include <SD_MMC.h>
#include <driver/sdmmc_host.h>

namespace Board::Storage {

    bool setSdMmcPins();
    void configureSdMmcSlot(sdmmc_slot_config_t& slotConfig);

} // namespace Board::Storage
