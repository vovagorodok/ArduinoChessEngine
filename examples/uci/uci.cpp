#include <ArduinoChessEngine.h>
#include "ChessEngineUci.h"

#ifndef ARDUINO
ChessEngineUci uci{};

int main() {
    uci.begin();

    return 0;
}
#endif
