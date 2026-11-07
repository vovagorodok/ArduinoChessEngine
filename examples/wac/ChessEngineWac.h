#pragma once
#include "ChessEngineRound.h"
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
constexpr size_t MAX_EPD_BEST_MOVES = 5;
using EpdBestMoves = Vector<Move, MAX_EPD_BEST_MOVES>;

struct EpdResultCounts {
    size_t matched;
    size_t missing;
    size_t extra;
};

class Wac {
 public:
    Wac();

    void begin();
    void begin(size_t num);
    void begin(const String& epd);

 private:
    bool apply(const String& epd);
    bool isExpectedMove(const Move& move) const;
    EpdResultCounts evaluate() const;
    EpdResultCounts solve(const String& epd);

    EpdBestMoves expectedMoves;
    Moves solvedMoves;
};
}

using ChessEngineWac = ChessEngine::Wac;
