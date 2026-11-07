#include "ChessEngineTask.h"
#include "ChessEngine.h"

namespace ChessEngine {
#ifdef CHESS_ENGINE_MULTI_TASK
Task::Task(Engine& engine) :
    moves(),
    isActive(false),
    isRunning(false),
    depth(ROOT_DEPTH),
    engine(engine) {
}

void Task::pull() {
    if (!isRunning) {
        return;
    }
    engine.addTaskMoves(depth);
    isRunning = false;
}
#endif
}
