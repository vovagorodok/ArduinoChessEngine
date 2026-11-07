#include <ArduinoChessEngine.h>
#include "ChessEngineWac.h"

#ifndef ARDUINO
ChessEngineWac wac{};

int main() {
    wac.begin();

    return 0;
}
#endif
