#pragma once
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
using Minutes = unsigned long;
using Seconds = unsigned long;
using Milliseconds = unsigned long;

constexpr Minutes MINUTES_IN_HOUR = 60;
constexpr Seconds SECONDS_IN_MINUTE = 60;
constexpr Seconds SECONDS_IN_HOUR = SECONDS_IN_MINUTE * MINUTES_IN_HOUR;
constexpr Milliseconds MILLISECONDS_IN_SECOND = 1000;
constexpr Milliseconds MILLISECONDS_IN_MINUTE = MILLISECONDS_IN_SECOND * SECONDS_IN_MINUTE;

inline Seconds convertMillisecondsToSeconds(Milliseconds milliseconds) {
    return milliseconds / MILLISECONDS_IN_SECOND;
}

inline Milliseconds convertSecondsToMilliseconds(Seconds seconds) {
    return seconds * MILLISECONDS_IN_SECOND;
}

inline String convertSecondsToStr(Seconds seconds) {
    static char buf[14];
    const auto hr = seconds / SECONDS_IN_HOUR;
    const auto min = (seconds % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE;
    const auto sec = seconds % SECONDS_IN_MINUTE;
    sprintf(buf, "%02lu:%02lu:%02lu", hr, min, sec);
    return String(buf);
}

inline String convertMillisecondsToStr(Milliseconds milliseconds) {
    return convertSecondsToStr(convertMillisecondsToSeconds(milliseconds));
}
}
