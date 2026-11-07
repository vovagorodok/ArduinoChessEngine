#pragma once
#include "ChessEngineMoves.h"

namespace ChessEngine {
using HalfMove = uint16_t;
using FullMove = uint16_t;

struct CastlingAbilities {
    bool WhiteKing: 1;
    bool WhiteQueen: 1;
    bool BlackKing: 1;
    bool BlackQueen: 1;
};

struct MaterialWeights {
    Weight white;
    Weight black;
};

struct PstWeights {
    Weight early;
    Weight late;

    PstWeights operator+(PstWeights other) const {
        return {static_cast<Weight>(early + other.early), static_cast<Weight>(late + other.late)};
    }
    PstWeights operator-(PstWeights other) const {
        return {static_cast<Weight>(early - other.early), static_cast<Weight>(late - other.late)};
    }
    PstWeights& operator+=(PstWeights other) {
        early += other.early;
        late += other.late;
        return *this;
    }
    PstWeights& operator-=(PstWeights other) {
        early -= other.early;
        late -= other.late;
        return *this;
    }
};

struct RoundWeights {
    MaterialWeights material;
    PstWeights pst;
};

enum class RoundStatus : uint8_t {
    Normal,
    Check,
    Checkmate,
    Stalemate,
    DeadPosition,
    FiftyMove,
    ThreefoldRepetition,
    Stopped,
    Invalid
};

enum class RoundPhase : uint8_t {
    Early,
    Late
};

struct Round {
    PieceColor turn;
    CastlingAbilities castling;
    Position enPassant;
    HalfMove halfMove;
    FullMove fullMove;
    Moves moves;
    Move move;
    Move best;
    RoundWeights weights;
    RoundStatus status;
    RoundPhase phase;

    bool isWhiteTurn() const { return turn == PieceColor::White; }
    bool isBlackTurn() const { return turn == PieceColor::Black; }

    bool isNormal() const { return status == RoundStatus::Normal; }
    bool isCheck() const { return status == RoundStatus::Check; }
    bool isCheckmate() const { return status == RoundStatus::Checkmate; }
    bool isStalemate() const { return status == RoundStatus::Stalemate; }
    bool isDeadPosition() const { return status == RoundStatus::DeadPosition; }
    bool isFiftyMove() const { return status == RoundStatus::FiftyMove; }
    bool isThreefoldRepetition() const { return status == RoundStatus::ThreefoldRepetition; }
    bool isStopped() const { return status == RoundStatus::Stopped; }
    bool isInvalid() const { return status == RoundStatus::Invalid; }
    bool isDraw() const { return isStalemate() || isDeadPosition() || isFiftyMove() || isThreefoldRepetition(); }
    bool isEnd() const { return isCheckmate() || isDraw() || isInvalid(); }
};
}
