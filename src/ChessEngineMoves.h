#pragma once
#include "ChessEngineMove.h"
#include "ChessEngineVector.h"
#include "ChessEngineArchConsts.h"

namespace ChessEngine {
#define MAX_MOVES CHESS_ENGINE_MAX_MOVES
using Moves = Vector<Move, MAX_MOVES>;
}
