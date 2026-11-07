#pragma once
#include "ChessEngineRound.h"
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
class Game {
 public:
    Game();

    void begin();
    void pull();

 private:
    void printRound();
    void printRoundStatus(RoundStatus status);
    bool checkInput();
    String flushInput();
    String readInput();

    bool isInputBusy;
    String buff;
};
}

using ChessEngineGame = ChessEngine::Game;
