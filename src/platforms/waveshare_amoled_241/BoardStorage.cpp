#include "board/BoardStorage.h"

#include "platforms/waveshare_amoled_241/WaveshareAmoled241.h"

namespace Board::Storage {

bool setSdMmcPins() {
  return SD_MMC.setPins(WaveshareAmoled241::Storage::kSdClockPin,
                        WaveshareAmoled241::Storage::kSdCommandPin,
                        WaveshareAmoled241::Storage::kSdData0Pin);
}

void configureSdMmcSlot(sdmmc_slot_config_t &slotConfig) {
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
  slotConfig.clk = WaveshareAmoled241::Storage::kSdClockPin;
  slotConfig.cmd = WaveshareAmoled241::Storage::kSdCommandPin;
  slotConfig.d0 = WaveshareAmoled241::Storage::kSdData0Pin;
  slotConfig.d1 = WaveshareAmoled241::Storage::kSdData1Pin;
  slotConfig.d2 = WaveshareAmoled241::Storage::kSdData2Pin;
  slotConfig.d3 = WaveshareAmoled241::Storage::kSdData3Pin;
#else
  (void)slotConfig;
#endif
}

}  // namespace Board::Storage
