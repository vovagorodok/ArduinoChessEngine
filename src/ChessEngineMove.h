#pragma once
#include "ChessEngineTypes.h"
#include "ChessEngineConsts.h"
#include "ChessEnginePieces.h"
#include "ChessEnginePositions.h"
#include "ChessEngineMoveType.h"

namespace ChessEngine {
struct Move {
    Pieces pieces;
    Positions positions;
    MoveType type;
    bool isCheck;
    Weight weight;

    constexpr Move() :
        pieces(),
        positions(),
        type(),
        isCheck(),
        weight() {}
    constexpr Move(Pieces pieces, Positions positions, MoveType type) :
        pieces(pieces),
        positions(positions),
        type(type),
        isCheck(),
        weight() {}
    constexpr Move(Piece srcPiece, Piece dstPiece, Position srcPos, Position dstPos, MoveType type) :
        pieces{srcPiece, dstPiece},
        positions{srcPos, dstPos},
        type(type),
        isCheck(),
        weight() {}
    static constexpr Move none() { return Move{}; }

    static inline Move fromUci(const String& uci) {
        if (uci.length() < 4) {
            return Move::none();
        }
        Move move{};
        move.positions = convertStrToPositions(uci);
        move.type = MoveType::Normal;
        if (uci.length() > 4) {
            move.type = convertPieceKindToMoveType(convertCharToPieceKind(uci[4]));
            if (move.isNormal()) {
                return Move::none();
            }
        }
        return move;
    }

    inline String toUci() {
        String str = convertPositionsToStr(positions);
        if (isPromotion()) {
            str += convertPieceKindToChar(convertMoveTypeToPieceKind(type));
        }
        return str;
    }

    bool isNone() const { return positions.src.isNone(); }
    bool isSame(const Move& other) const { return type == other.type && positions == other.positions; }
    bool isNormal() const { return type == MoveType::Normal; }
    bool isEnPassant() const { return type == MoveType::EnPassant; }
    bool isCastling() const {
        switch (type) {
            case MoveType::CastleKing:
            case MoveType::CastleQueen:
                return true;
            default:
                return false;
        }
    }
    bool isPromotion() const {
        switch (type) {
            case MoveType::PromoKnight:
            case MoveType::PromoBishop:
            case MoveType::PromoRook:
            case MoveType::PromoQueen:
                return true;
            default:
                return false;
        }
    }
    bool isBeat() const { return !pieces.dst.isEmpty(); }
};

static_assert(sizeof(Move) <= sizeof(uint64_t));

inline Move convertUciToMove(const String& uci) {
    return Move::fromUci(uci);
}

inline String convertMoveToUci(Move move) {
    return move.toUci();
}
}
