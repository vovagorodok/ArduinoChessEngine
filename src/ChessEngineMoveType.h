#pragma once
#include "ChessEnginePieceKind.h"

namespace ChessEngine {
enum class MoveType : uint8_t {
    Normal,
    EnPassant,
    CastleKing,
    CastleQueen,
    PromoKnight,
    PromoBishop,
    PromoRook,
    PromoQueen
};

inline PieceKind convertMoveTypeToPieceKind(MoveType type) {
    switch (type) {
        case MoveType::PromoKnight:
            return PieceKind::Knight;
        case MoveType::PromoBishop:
            return PieceKind::Bishop;
        case MoveType::PromoRook:
            return PieceKind::Rook;
        case MoveType::PromoQueen:
            return PieceKind::Queen;
        default:
            return PieceKind::Empty;
    }
}

inline MoveType convertPieceKindToMoveType(PieceKind kind) {
    switch (kind) {
        case PieceKind::Knight:
            return MoveType::PromoKnight;
        case PieceKind::Bishop:
            return MoveType::PromoBishop;
        case PieceKind::Rook:
            return MoveType::PromoRook;
        case PieceKind::Queen:
            return MoveType::PromoQueen;
        default:
            return MoveType::Normal;
    }
}

constexpr bool operator<(MoveType a, MoveType b) {
    return static_cast<uint8_t>(a) < static_cast<uint8_t>(b);
}
constexpr bool operator>(MoveType a, MoveType b) {
    return static_cast<uint8_t>(a) > static_cast<uint8_t>(b);
}
constexpr bool operator<=(MoveType a, MoveType b) {
    return static_cast<uint8_t>(a) <= static_cast<uint8_t>(b);
}
constexpr bool operator>=(MoveType a, MoveType b) {
    return static_cast<uint8_t>(a) >= static_cast<uint8_t>(b);
}
}
