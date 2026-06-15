#include "input/Input.h"

#include <algorithm>

#include "board/BoardImu.h"
#include "board/BoardInput.h"

namespace Input {
namespace {

struct ControlsState {
  bool initialized = false;
  ControlMask stableControls = InputNone;
  ControlMask candidateControls = InputNone;
  ControlMask activeControls = InputNone;
  bool releasedEvent = false;
  bool longPressEmitted = false;
  uint32_t candidateSinceMs = 0;
  uint32_t pressStartedMs = 0;
  uint32_t lastPressDurationMs = 0;
};

struct TouchState {
  Board::UiOrientation orientation = Board::UiOrientation::Portrait;
  bool initialized = false;
  bool active = false;
  uint8_t emptySamples = 0;
  uint8_t consecutiveReadFailures = 0;
  uint32_t startedAtMs = 0;
  uint32_t lastPollMs = 0;
  uint32_t backoffUntilMs = 0;
  uint32_t ignoreEventsUntilMs = 0;
  uint16_t startX = 0;
  uint16_t startY = 0;
  uint16_t lastX = 0;
  uint16_t lastY = 0;
};

struct TouchMotion {
  uint16_t dx = 0;
  uint16_t dy = 0;
  uint32_t durationMs = 0;
};

ControlsState gControls;
ControlTiming gControlTiming;
TouchSurface gTouchSurface;
TouchTiming gTouchTiming;
TouchState gTouch;

void resetControls(ControlMask controls, uint32_t nowMs) {
  gControls.initialized = true;
  gControls.stableControls = controls;
  gControls.candidateControls = controls;
  gControls.activeControls = controls;
  gControls.releasedEvent = false;
  gControls.longPressEmitted = false;
  gControls.candidateSinceMs = nowMs;
  gControls.pressStartedMs = controls == InputNone ? 0 : nowMs;
  gControls.lastPressDurationMs = 0;
}

void updateControls(ControlMask controls, uint32_t nowMs) {
  if (!gControls.initialized) {
    resetControls(controls, nowMs);
    return;
  }

  gControls.releasedEvent = false;

  {
    // Debounce the raw bitmask before changing the stable pressed controls.
    if (controls != gControls.candidateControls) {
      gControls.candidateControls = controls;
      gControls.candidateSinceMs = nowMs;
    }

    if (gControls.candidateControls == gControls.stableControls) {
      return;
    }

    if (nowMs - gControls.candidateSinceMs < gControlTiming.debounceMs) {
      return;
    }
  }

  const ControlMask previousControls = gControls.stableControls;
  gControls.stableControls = gControls.candidateControls;

  {
    // Track the gesture lifetime for whichever controls became active together.
    if (previousControls == InputNone && gControls.stableControls != InputNone) {
      gControls.activeControls = gControls.stableControls;
      gControls.pressStartedMs = nowMs;
      gControls.lastPressDurationMs = 0;
      gControls.longPressEmitted = false;
      return;
    }

    if (previousControls != InputNone && gControls.stableControls == InputNone) {
      gControls.releasedEvent = true;
      gControls.lastPressDurationMs = nowMs - gControls.pressStartedMs;
      return;
    }

    if (previousControls != gControls.stableControls) {
      gControls.activeControls = gControls.stableControls;
      gControls.pressStartedMs = nowMs;
      gControls.lastPressDurationMs = 0;
      gControls.longPressEmitted = false;
    }
  }
}

bool pollControlsEvent(ControlMask controls, uint32_t nowMs, Event &event) {
  updateControls(controls, nowMs);

  if (gControls.stableControls != InputNone && !gControls.longPressEmitted &&
      nowMs - gControls.pressStartedMs >= gControlTiming.longPressMs) {
    gControls.longPressEmitted = true;
    event = {gControls.stableControls, Gesture::LongPressed};
    return true;
  }

  if (gControls.releasedEvent && !gControls.longPressEmitted &&
      gControls.lastPressDurationMs <= gControlTiming.shortPressMaxMs) {
    event = {gControls.activeControls, Gesture::ShortPressed};
    gControls.activeControls = InputNone;
    return true;
  }

  if (gControls.releasedEvent) {
    gControls.activeControls = InputNone;
  }
  return false;
}

void resetTouchState() {
  gTouch.active = false;
  gTouch.emptySamples = 0;
  gTouch.startedAtMs = 0;
  gTouch.startX = 0;
  gTouch.startY = 0;
  gTouch.lastX = 0;
  gTouch.lastY = 0;
}

constexpr uint8_t orientationQuarterTurns(Board::UiOrientation orientation) {
  const uint8_t value = static_cast<uint8_t>(orientation);
  uint8_t flippedTurns = value & 1U;
  flippedTurns <<= 1U;
  const uint8_t landscapeTurn = (value & 2U) == 0U ? uint8_t{1} : uint8_t{0};
  return flippedTurns | landscapeTurn;
}

TouchContact mapTouchContact(TouchContact contact) {
  const uint8_t quarterTurns = orientationQuarterTurns(gTouch.orientation);
  constexpr uint16_t kMin = 0;
  const uint16_t maxX = std::max(gTouchSurface.width, uint16_t{1}) - 1;
  const uint16_t maxY = std::max(gTouchSurface.height, uint16_t{1}) - 1;

  uint16_t x = contact.x;
  uint16_t y = contact.y;

  switch (quarterTurns) {
    case 1:
      x = maxY - std::clamp(contact.y, kMin, maxY);
      y = contact.x;
      break;
    case 2:
      x = maxX - std::clamp(contact.x, kMin, maxX);
      y = maxY - std::clamp(contact.y, kMin, maxY);
      break;
    case 3:
      x = contact.y;
      y = maxX - std::clamp(contact.x, kMin, maxX);
      break;
    default:
      break;
  }

  return {true, std::clamp(x, kMin, maxX), std::clamp(y, kMin, maxY)};
}

TouchMotion touchMotion(uint32_t nowMs) {
  const auto delta = [](uint16_t left, uint16_t right) {
    uint16_t high = std::max(left, right);
    high -= std::min(left, right);
    return high;
  };

  return {delta(gTouch.lastX, gTouch.startX), delta(gTouch.lastY, gTouch.startY),
          nowMs - gTouch.startedAtMs};
}

bool matchesReleaseGesture(Gesture gesture, TouchMotion motion) {
  switch (gesture) {
    case Gesture::Tapped:
      return motion.durationMs <= gTouchTiming.tapMaxDurationMs &&
             motion.dx <= gTouchTiming.tapMoveTolerancePx &&
             motion.dy <= gTouchTiming.tapMoveTolerancePx;

    case Gesture::TopEdgeSwiped:
      return gTouch.startY <= gTouchTiming.edgeSizePx && gTouch.lastY > gTouch.startY &&
             gTouch.lastY - gTouch.startY >= gTouchTiming.swipeMinDistancePx;

    case Gesture::BottomEdgeSwiped:
      return gTouchSurface.height > gTouchTiming.edgeSizePx &&
             gTouch.startY >=
                 gTouchSurface.height - gTouchTiming.edgeSizePx &&
             gTouch.lastY < gTouch.startY &&
             gTouch.startY - gTouch.lastY >= gTouchTiming.swipeMinDistancePx;

    default:
      return false;
  }
}

Gesture touchReleaseGesture(uint32_t nowMs) {
  const TouchMotion motion = touchMotion(nowMs);
  if (matchesReleaseGesture(Gesture::Tapped, motion)) {
    return Gesture::Tapped;
  }
  if (matchesReleaseGesture(Gesture::TopEdgeSwiped, motion)) {
    return Gesture::TopEdgeSwiped;
  }
  if (matchesReleaseGesture(Gesture::BottomEdgeSwiped, motion)) {
    return Gesture::BottomEdgeSwiped;
  }
  return Gesture::TouchEnd;
}

bool releaseTouch(uint32_t nowMs, Event &event) {
  if (!gTouch.active) {
    return false;
  }

  ++gTouch.emptySamples;
  if (gTouch.emptySamples < gTouchTiming.releaseConfirmSamples) {
    return false;
  }

  const Gesture gesture = touchReleaseGesture(nowMs);
  gTouch.active = false;
  gTouch.emptySamples = 0;
  event = {InputTouch, gesture, gTouch.lastX, gTouch.lastY};
  return true;
}

bool pollTouchContact(const TouchContact &contact, uint32_t nowMs, Event &event) {
  if (!contact.touched) {
    return releaseTouch(nowMs, event);
  }

  gTouch.emptySamples = 0;

  const TouchContact mapped = mapTouchContact(contact);

  if (!gTouch.active) {
    gTouch.active = true;
    gTouch.startedAtMs = nowMs;
    gTouch.startX = mapped.x;
    gTouch.startY = mapped.y;
    gTouch.lastX = mapped.x;
    gTouch.lastY = mapped.y;
    event = {InputTouch, Gesture::TouchStart, mapped.x, mapped.y};
    return true;
  }

  gTouch.lastX = mapped.x;
  gTouch.lastY = mapped.y;
  event = {InputTouch, Gesture::TouchMove, mapped.x, mapped.y};
  return true;
}

void syncTouchOrientation() {
  const Board::UiOrientation orientation = Board::Imu::uiOrientation();
  if (gTouch.orientation == orientation) {
    return;
  }

  gTouch.orientation = orientation;
  resetTouchState();
}

bool beginTouch(uint32_t nowMs) {
  resetTouchState();
  gTouch.lastPollMs = 0;
  gTouch.backoffUntilMs = 0;
  gTouch.consecutiveReadFailures = 0;

  gTouch.initialized = Board::Input::beginTouch();
  if (gTouch.initialized) {
    gTouch.ignoreEventsUntilMs = nowMs + gTouchTiming.recoveryEventIgnoreMs;
  }
  return gTouch.initialized;
}

bool pollTouchEvent(uint32_t nowMs, Event &event) {
  syncTouchOrientation();

  {
    // Touch hardware can disappear during resets; retry without blocking button input.
    if (!gTouch.initialized) {
      if (nowMs >= gTouch.backoffUntilMs && !beginTouch(nowMs)) {
        gTouch.backoffUntilMs = nowMs + gTouchTiming.recoveryRetryMs;
      }
      return false;
    }
  }

  {
    // Keep failed reads from hammering the bus and keep normal polling bounded.
    if (nowMs < gTouch.backoffUntilMs ||
        nowMs - gTouch.lastPollMs < gTouchTiming.pollIntervalMs) {
      return false;
    }
    gTouch.lastPollMs = nowMs;
  }

  TouchContact contact;
  const bool contactRead = [&]() {
    if (!Board::Input::touchReady()) {
      contact = {};
      return true;
    }

    if (Board::Input::readTouch(contact)) {
      gTouch.consecutiveReadFailures = 0;
      return true;
    }

    gTouch.backoffUntilMs = nowMs + gTouchTiming.failureBackoffMs;
    ++gTouch.consecutiveReadFailures;

    if (gTouch.consecutiveReadFailures >= gTouchTiming.maxConsecutiveReadFailures) {
      gTouch.initialized = false;
      gTouch.backoffUntilMs = nowMs + gTouchTiming.recoveryRetryMs;
      resetTouchState();
    }
    return false;
  }();

  if (!contactRead) {
    return false;
  }

  if (nowMs < gTouch.ignoreEventsUntilMs) {
    resetTouchState();
    return false;
  }

  return pollTouchContact(contact, nowMs, event);
}

}  // namespace

bool begin() {
  if (!Board::Input::begin()) {
    return false;
  }

  const uint32_t nowMs = millis();
  gControlTiming = Board::Input::controlTiming();
  resetControls(Board::Input::currentControls(), nowMs);

  gTouchSurface = Board::Input::touchSurface();
  gTouchTiming = Board::Input::touchTiming();
  gTouch.orientation = Board::Imu::uiOrientation();
  beginTouch(nowMs);
  return true;
}

void end() {
  cancel();
  Board::Input::end();
}

void cancel() {
  const uint32_t nowMs = millis();
  resetControls(Board::Input::currentControls(), nowMs);
  resetTouchState();
  gTouch.initialized = false;
  Board::Input::cancel();
}

bool poll(Event &event, uint32_t nowMs) {
  event = {};

  const ControlMask controls = Board::Input::currentControls();
  if (pollControlsEvent(controls, nowMs, event)) {
    return true;
  }

  return pollTouchEvent(nowMs, event);
}

}  // namespace Input
