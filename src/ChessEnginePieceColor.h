#pragma once
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
enum class PieceColor : int8_t {
    Black = 0,
    White = 1
};

inline bool isPieceColorChar(char ch) {
    return ch == 'w' || ch == 'b';
}

inline PieceColor convertCharToPieceColor(char ch) {
    return ch == 'w' ? PieceColor::White : PieceColor::Black;
}

inline char convertPieceColorToChar(PieceColor color) {
    return color == PieceColor::White ? 'w' : 'b';
}

constexpr PieceColor operator!(PieceColor color) {
    return color == PieceColor::White ? PieceColor::Black : PieceColor::White;
}
}
