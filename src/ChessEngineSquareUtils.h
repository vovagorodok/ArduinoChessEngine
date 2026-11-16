#pragma once
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
inline bool isSquareFileChar(char ch) {
    return ch >= 'a' && ch <= 'h';
}

inline uint8_t convertCharToSquareFile(char ch) {
    return ch - 'a';
}

inline char convertSquareFileToChar(uint8_t file) {
    return 'a' + file;
}

inline bool isSquareRankChar(char ch) {
    return ch >= '1' && ch <= '8';
}

inline uint8_t convertCharToSquareRank(char ch) {
    return ch - '0';
}

inline char convertSquareRankToChar(uint8_t rank) {
    return '0' + rank;
}
}
