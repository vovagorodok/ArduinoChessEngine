#include <Arduino.h>
#include <ArduinoChessEngine.h>

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ;

    ArduinoChessEngine.begin("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    ArduinoChessEngine.make("a2a3");
    ArduinoChessEngine.make(ArduinoChessEngine.solve());
    Serial.println(ArduinoChessEngine.getLastMove().toUci());
}

void loop() {
}
