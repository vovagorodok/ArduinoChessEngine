#pragma once
#include "ChessEngineRound.h"
#include "ChessEngineConsts.h"
#include <array>

namespace ChessEngine {
class Search {
 public:
    Round& operator[](Depth depth) { return search[depth]; }
    Round& getRound(Depth depth) { return search[depth]; }
    const Round& getRound(Depth depth) const { return search[depth]; }

    Round& getNextRound(Depth depth) { return search[depth + 1]; }
    Round& getPrevRound(Depth depth) { return search[depth - 1]; }

    static bool isPrevRoundExist(Depth depth) { return depth > ROOT_DEPTH; }
    static bool isNextRoundExist(Depth depth) { return depth < MAX_DEPTH; }

 private:
    std::array<Round, MAX_DEPTHS> search;
};
}
