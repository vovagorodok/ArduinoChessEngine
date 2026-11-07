#pragma once
#include "ChessEngineTime.h"

namespace ChessEngine {
class Timer {
 public:
    void begin() { startTime = millis(); }
    Milliseconds value() { return millis() - startTime; }

 private:
    Milliseconds startTime;
};
}
