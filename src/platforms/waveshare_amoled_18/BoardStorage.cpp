#include "board/BoardStorage.h"

#include "platforms/waveshare_amoled_18/WaveshareAmoled18.h"

namespace Board::Storage {

bool setSdMmcPins() {
  return SD_MMC.setPins(WaveshareAmoled18::Storage::kSdClockPin,
                        WaveshareAmoled18::Storage::kSdCommandPin,
                        WaveshareAmoled18::Storage::kSdData0Pin);
}

void configureSdMmcSlot(sdmmc_slot_config_t &slotConfig) {
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
  slotConfig.clk = WaveshareAmoled18::Storage::kSdClockPin;
  slotConfig.cmd = WaveshareAmoled18::Storage::kSdCommandPin;
  slotConfig.d0 = WaveshareAmoled18::Storage::kSdData0Pin;
  slotConfig.d1 = WaveshareAmoled18::Storage::kSdData1Pin;
  slotConfig.d2 = WaveshareAmoled18::Storage::kSdData2Pin;
  slotConfig.d3 = WaveshareAmoled18::Storage::kSdData3Pin;
#else
  (void)slotConfig;
#endif
}

}  // namespace Board::Storage
