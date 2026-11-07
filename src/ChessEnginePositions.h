#pragma once
#include "ChessEnginePosition.h"

namespace ChessEngine {
struct Positions {
    Position src;
    Position dst;
};

inline String convertPositionsToStr(Positions positions) {
    return convertPositionToStr(positions.src) + convertPositionToStr(positions.dst);
}

inline Positions convertStrToPositions(const String& str) {
    if (str.length() < 4) {
        return {};
    }
    return {convertStrToPosition(str.substring(0, 2)), convertStrToPosition(str.substring(2, 4))};
}

constexpr bool operator==(Positions a, Positions b) {
    return a.src == b.src && a.dst == b.dst;
}
constexpr bool operator!=(Positions a, Positions b) {
    return !(a == b);
}
}
