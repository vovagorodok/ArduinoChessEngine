#pragma once
#include "ChessEngineSquareUtils.h"
#include "ChessEngineConsts.h"
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
class Position {
 public:
    using ValueType = uint8_t;
    static constexpr ValueType NONE_VALUE = 255;

    constexpr Position() :
        value(NONE_VALUE) {}
    constexpr Position(ValueType v) :
        value(v) {}

    static constexpr Position none() { return Position(NONE_VALUE); }
    constexpr bool isNone() const { return value == NONE_VALUE; }
    constexpr operator ValueType() const { return value; }

    static Position fromString(const String& str) {
        if (str.length() < 2) {
            return Position::none();
        }
        const char file = str[0];
        const char rank = str[1];
        if (!isSquareFileChar(file) || !isSquareRankChar(rank)) {
            return Position::none();
        }
        return convertCharToSquareFile(file) + BOARD_SQUARES_NUM * (BOARD_SQUARES_NUM - convertCharToSquareRank(rank));
    }

    String toString() const {
        if (isNone()) {
            return "__";
        }
        return String(convertSquareFileToChar(value % BOARD_SQUARES_NUM)) +
               convertSquareRankToChar(BOARD_SQUARES_NUM - value / BOARD_SQUARES_NUM);
    }

    ValueType toSquareFile() const { return value % BOARD_SQUARES_NUM; }
    ValueType toSquareRank() const { return value / BOARD_SQUARES_NUM; }

    Position& operator++() {
        value++;
        return *this;
    }
    Position operator++(int) {
        Position prev = *this;
        value++;
        return prev;
    }
    Position& operator--() {
        value--;
        return *this;
    }
    Position operator--(int) {
        Position prev = *this;
        value--;
        return prev;
    }
    Position& operator+=(int offset) {
        value += offset;
        return *this;
    }

 private:
    ValueType value;
};

inline Position convertStringToPosition(const String& str) {
    return Position::fromString(str);
}

inline String convertPositionToString(Position pos) {
    return pos.toString();
}
}
