#pragma once
#include "ChessEnginePosition.h"

namespace ChessEngine {
struct Positions {
    Position src;
    Position dst;
};

inline String convertPositionsToString(Positions positions) {
    return convertPositionToString(positions.src) + convertPositionToString(positions.dst);
}

inline Positions convertStringToPositions(const String& str) {
    if (str.length() < 4) {
        return {};
    }
    return {convertStringToPosition(str.substring(0, 2)), convertStringToPosition(str.substring(2, 4))};
}

constexpr bool operator==(Positions a, Positions b) {
    return a.src == b.src && a.dst == b.dst;
}
constexpr bool operator!=(Positions a, Positions b) {
    return !(a == b);
}
}
