#pragma once
#include "ChessEnginePiece.h"
#include "ChessEnginePosition.h"
#include "ChessEngineConsts.h"
#include <array>

namespace ChessEngine {
using Board = std::array<Piece, BOARD_SIZE>;

struct KingPositions {
    Position white;
    Position black;
    Position check;
};
}
