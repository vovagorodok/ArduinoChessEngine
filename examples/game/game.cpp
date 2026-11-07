#include <ArduinoChessEngine.h>
#include "ChessEngineGame.h"

#ifndef ARDUINO
ChessEngineGame game{};

int main() {
    game.begin();

    return 0;
}
#endif