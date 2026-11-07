#pragma once
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
enum class PieceType : int8_t {
    Empty = 0,
    WhitePawn = 1,
    WhiteKnight = 2,
    WhiteBishop = 3,
    WhiteRook = 4,
    WhiteQueen = 5,
    WhiteKing = 6,
    BlackPawn = -1,
    BlackKnight = -2,
    BlackBishop = -3,
    BlackRook = -4,
    BlackQueen = -5,
    BlackKing = -6
};
}
