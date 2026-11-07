#pragma once
#include "ChessEngineTypes.h"
#include "ChessEnginePieceType.h"
#include "ChessEnginePieceKind.h"
#include "ChessEnginePieceColor.h"

namespace ChessEngine {
class Piece {
    PieceType type;

 public:
    using ValueType = int8_t;

    constexpr Piece() :
        type(PieceType::Empty) {}
    constexpr Piece(PieceType t) :
        type(t) {}
    Piece(char ch) :
        type(PieceType::Empty) {
        PieceKind kind = convertCharToPieceKind(tolower(ch));
        if (kind != PieceKind::Empty) {
            *this = isupper(ch) ? white(kind) : black(kind);
        }
    }
    char toChar() const {
        char c = convertPieceKindToChar(getKind());
        return isWhite() ? toupper(c) : c;
    }

    static constexpr Piece empty() { return Piece(PieceType::Empty); }
    static constexpr Piece white(PieceKind kind) { return static_cast<PieceType>(static_cast<ValueType>(kind)); }
    static constexpr Piece black(PieceKind kind) { return static_cast<PieceType>(-static_cast<ValueType>(kind)); }

    constexpr PieceType getType() const { return type; }
    constexpr ValueType getValue() const { return static_cast<ValueType>(type); }
    constexpr PieceKind getKind() const { return static_cast<PieceKind>(abs(getValue())); }
    constexpr PieceColor getColor() const { return isWhite() ? PieceColor::White : PieceColor::Black; }
    constexpr Weight getKindWeight() const { return static_cast<Weight>(getKind()); }

    constexpr bool isEmpty() const { return type == PieceType::Empty; }
    constexpr bool isWhite() const { return getValue() > 0; }
    constexpr bool isBlack() const { return getValue() < 0; }
    constexpr bool canMoveTo(Piece dst) const { return dst.isEmpty() || isWhite() != dst.isWhite(); }

    constexpr bool operator==(Piece other) const { return type == other.type; }
    constexpr bool operator!=(Piece other) const { return type != other.type; }
    constexpr bool operator==(PieceType other) const { return type == other; }
    constexpr bool operator!=(PieceType other) const { return type != other; }
};
}
