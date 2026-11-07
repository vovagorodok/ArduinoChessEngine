#pragma once
#include "ChessEngineMove.h"
#include "ChessEngineVector.h"
#include "ChessEngineArchConsts.h"

namespace ChessEngine {
#define MAX_HISTORY CHESS_ENGINE_MAX_HISTORY
using History = Vector<Move, MAX_HISTORY>;
}
