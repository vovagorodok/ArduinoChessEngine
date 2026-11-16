#pragma once
#include "ChessEnginePiece.h"
#include "ChessEnginePosition.h"
#include "ChessEngineConsts.h"
#include <array>

namespace ChessEngine {
using Board = std::array<Piece, BOARD_SIZE>;

struct Kings {
    Position white;
    Position black;
    Position check;
};

struct CastlingRooks {
    Position whiteKing;
    Position whiteQueen;
    Position blackKing;
    Position blackQueen;
};
}
