#pragma once
#include "ChessEngineRound.h"

namespace ChessEngine {
#ifdef CHESS_ENGINE_MULTI_TASK
class Engine;

class Task {
 public:
    Task(Engine& engine);

    void pull();

 private:
    Moves moves;
    bool isActive;
    volatile bool isRunning;
    volatile Depth depth;

    friend Engine;
    Engine& engine;
};
#endif
}
