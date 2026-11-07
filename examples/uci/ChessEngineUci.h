#pragma once
#include "ChessEngineSupervisor.h"
#include "ChessEngineLevels.h"
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
class Uci : public CyclicPull {
 public:
    Uci();

    void begin();
    void pull() override;

 private:
    void handle(const String& input);
    void handleUci();
    void handleSetOption(const String& input);
    void handlePosition(const String& input);
    void handleGo(const String& input);
    void applyMoves(String moves);
    bool checkInput();
    String flushInput();
    String readInput();

    Levels levels;
    bool isInputBusy;
    String buff;
};
}

using ChessEngineUci = ChessEngine::Uci;
