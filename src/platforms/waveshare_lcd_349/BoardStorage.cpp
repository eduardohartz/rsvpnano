#include "board/BoardStorage.h"

#include "platforms/waveshare_lcd_349/WaveshareLcd349.h"

namespace Board::Storage {

bool setSdMmcPins() {
  return SD_MMC.setPins(WaveshareLcd349::Storage::kSdClockPin,
                        WaveshareLcd349::Storage::kSdCommandPin,
                        WaveshareLcd349::Storage::kSdData0Pin);
}

void configureSdMmcSlot(sdmmc_slot_config_t &slotConfig) {
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
  slotConfig.clk = WaveshareLcd349::Storage::kSdClockPin;
  slotConfig.cmd = WaveshareLcd349::Storage::kSdCommandPin;
  slotConfig.d0 = WaveshareLcd349::Storage::kSdData0Pin;
  slotConfig.d1 = WaveshareLcd349::Storage::kSdData1Pin;
  slotConfig.d2 = WaveshareLcd349::Storage::kSdData2Pin;
  slotConfig.d3 = WaveshareLcd349::Storage::kSdData3Pin;
#else
  (void)slotConfig;
#endif
}

}  // namespace Board::Storage
