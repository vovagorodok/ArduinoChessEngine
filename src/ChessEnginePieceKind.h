#pragma once
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
enum class PieceKind : int8_t {
    Empty = 0,
    Pawn = 1,
    Knight = 2,
    Bishop = 3,
    Rook = 4,
    Queen = 5,
    King = 6
};

inline char convertPieceKindToChar(PieceKind k) {
    switch (k) {
        case PieceKind::Pawn:
            return 'p';
        case PieceKind::Knight:
            return 'n';
        case PieceKind::Bishop:
            return 'b';
        case PieceKind::Rook:
            return 'r';
        case PieceKind::Queen:
            return 'q';
        case PieceKind::King:
            return 'k';
        default:
            return '_';
    }
}

inline PieceKind convertCharToPieceKind(char ch) {
    switch (ch) {
        case 'p':
            return PieceKind::Pawn;
        case 'n':
            return PieceKind::Knight;
        case 'b':
            return PieceKind::Bishop;
        case 'r':
            return PieceKind::Rook;
        case 'q':
            return PieceKind::Queen;
        case 'k':
            return PieceKind::King;
        default:
            return PieceKind::Empty;
    }
}

constexpr bool operator<(PieceKind a, PieceKind b) {
    return static_cast<int8_t>(a) < static_cast<int8_t>(b);
}
constexpr bool operator>(PieceKind a, PieceKind b) {
    return static_cast<int8_t>(a) > static_cast<int8_t>(b);
}
constexpr bool operator<=(PieceKind a, PieceKind b) {
    return static_cast<int8_t>(a) <= static_cast<int8_t>(b);
}
constexpr bool operator>=(PieceKind a, PieceKind b) {
    return static_cast<int8_t>(a) >= static_cast<int8_t>(b);
}
}
