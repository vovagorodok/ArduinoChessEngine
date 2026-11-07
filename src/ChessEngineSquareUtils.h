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

inline bool isSquareNumberChar(char ch) {
    return ch >= '1' && ch <= '8';
}

inline uint8_t convertCharToSquareNumber(char ch) {
    return ch - '0';
}

inline char convertSquareNumberToChar(uint8_t number) {
    return '0' + number;
}
}
