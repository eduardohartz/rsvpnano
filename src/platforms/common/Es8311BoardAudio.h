#pragma once

#include <Arduino.h>
#include <algorithm>
#include <esp_log.h>

#include "board/BoardPower.h"
#include "drivers/audio/es8311/Es8311.h"

namespace {

constexpr char kAudioTag[] = "audio";
constexpr uint32_t kAudioStartupDelayMs = 15;
constexpr uint32_t kBeepFrequencyHz = 1320;
constexpr int16_t kBeepAmplitude = 12000;
constexpr uint32_t kEnvelopeAttackMs = 6;
constexpr uint32_t kEnvelopeReleaseMs = 12;
constexpr uint32_t kSampleRateHz = 16000;
constexpr uint32_t kBeepDurationMs = 120;
constexpr uint32_t kMaxToneDurationMs = 120;
constexpr uint32_t kWriteTimeoutMs = 250;
constexpr size_t kToneBufferFrames =
    (static_cast<size_t>(kSampleRateHz) * kMaxToneDurationMs) / 1000U;
constexpr size_t kToneBufferSamples = kToneBufferFrames * 2U;

int16_t gToneBuffer[kToneBufferSamples] = {};

// Renders a square tone with attack/release envelope; returns the sample count.
size_t fillToneBuffer(uint32_t frequencyHz, uint32_t durationMs, int16_t amplitude) {
  if (frequencyHz == 0 || durationMs == 0) {
    return 0;
  }
  if (durationMs > kMaxToneDurationMs) {
    durationMs = kMaxToneDurationMs;
  }

  const size_t frames = (static_cast<size_t>(kSampleRateHz) * durationMs) / 1000U;
  size_t attackFrames = (static_cast<size_t>(kSampleRateHz) * kEnvelopeAttackMs) / 1000U;
  size_t releaseFrames = (static_cast<size_t>(kSampleRateHz) * kEnvelopeReleaseMs) / 1000U;
  attackFrames = std::min(attackFrames, frames / 4U);
  releaseFrames = std::min(releaseFrames, frames / 4U);
  const uint32_t halfPeriodSamples = std::max(1U, kSampleRateHz / (frequencyHz * 2U));

  for (size_t frame = 0; frame < frames; ++frame) {
    int32_t sample = ((frame / halfPeriodSamples) % 2U == 0U) ? amplitude : -amplitude;

    if (attackFrames > 0 && frame < attackFrames) {
      sample = (sample * static_cast<int32_t>(frame)) / static_cast<int32_t>(attackFrames);
    } else if (releaseFrames > 0 && frame >= (frames - releaseFrames)) {
      const size_t remaining = frames - frame;
      sample = (sample * static_cast<int32_t>(remaining)) / static_cast<int32_t>(releaseFrames);
    }

    const size_t index = frame * 2U;
    gToneBuffer[index] = static_cast<int16_t>(sample);
    gToneBuffer[index + 1U] = static_cast<int16_t>(sample);
  }

  return frames * 2U;
}

bool enableAudioRail() { return Board::Power::enableAudioPowerIfAvailable(); }

}  // namespace

namespace BoardPlatform::Es8311BoardAudio {

bool begin(BoardDrivers::Es8311::Context &context) {
  if (BoardDrivers::Es8311::available(context)) {
    return true;
  }

  if (!enableAudioRail()) {
    ESP_LOGW(kAudioTag, "Audio rail unavailable");
    return false;
  }

  delay(kAudioStartupDelayMs);
  return BoardDrivers::Es8311::begin(context);
}

bool tone(BoardDrivers::Es8311::Context &context, uint32_t frequencyHz, uint32_t durationMs,
          int16_t amplitude) {
  const size_t sampleCount = fillToneBuffer(frequencyHz, durationMs, amplitude);
  if (sampleCount == 0) {
    return false;
  }

  if (!enableAudioRail() || !BoardDrivers::Es8311::prepareOutput(context)) {
    return false;
  }

  if (BoardDrivers::Es8311::writeSamples(context, gToneBuffer, sampleCount, kWriteTimeoutMs)) {
    return true;
  }

  ESP_LOGW(kAudioTag, "Retrying speaker tone after recovering output path");
  if (!enableAudioRail() || !BoardDrivers::Es8311::recoverOutputPath(context)) {
    return false;
  }

  return BoardDrivers::Es8311::writeSamples(context, gToneBuffer, sampleCount, kWriteTimeoutMs);
}

bool beep(BoardDrivers::Es8311::Context &context) {
  return tone(context, kBeepFrequencyHz, kBeepDurationMs, kBeepAmplitude);
}

bool available(const BoardDrivers::Es8311::Context &context) {
  return BoardDrivers::Es8311::available(context);
}

}  // namespace BoardPlatform::Es8311BoardAudio
