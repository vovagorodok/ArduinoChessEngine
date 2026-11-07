#include <Arduino.h>
#include <ArduinoChessEngine.h>
#include "ChessEngineUci.h"

ChessEngineUci uci{};

#ifndef ARDUINO_ARCH_ESP32
extern "C" {
int _write(int fd, char* ptr, int len) {
    (void)fd;
    return Serial.write(ptr, len);
}
}
#endif

#ifdef CHESS_ENGINE_MULTI_TASK
#ifdef ARDUINO_ARCH_ESP32
using Milliseconds = ChessEngineMilliseconds;
constexpr Milliseconds WDT_UPDATE_TIME = CONFIG_ESP_TASK_WDT_TIMEOUT_S * 1000 / 2;

void runTask(void*) {
    Milliseconds lastWdtUpdate = 0;
    ChessEngineTask& task = ArduinoChessEngine.setMultiTask();
    do {
        task.pull();
        const Milliseconds now = millis();

        if (now - lastWdtUpdate >= WDT_UPDATE_TIME) {
            lastWdtUpdate = now;
            delay(1);
        }
    } while (true);
}
#endif
#endif

void setup() {
    Serial.begin(115200);
    while (!Serial)
        ;

#ifdef CHESS_ENGINE_MULTI_TASK
#ifdef ARDUINO_ARCH_ESP32
    xTaskCreate(runTask, "RunTask", 10000, NULL, 1, NULL);
#endif
#endif
}

void loop() {
    uci.begin();
}
