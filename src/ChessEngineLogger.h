#pragma once
#include "ChessEngineDefinesArduino.h"

// clang-format off: Used IndentPPDirectives with BeforeHash
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
    #define CHESS_ENGINE_PRINTF Serial.printf
#else
    #define CHESS_ENGINE_PRINTF printf
#endif

#define CHESS_ENGINE_PRINTF_NOOP do {} while (0)

#ifdef CHESS_ENGINE_LOGS
    #ifdef CHESS_ENGINE_LOGS_WITHOUT_PREFIX
        #define CHESS_ENGINE_LOG(tag, fmt, ...) \
            CHESS_ENGINE_PRINTF(fmt "\n", ##__VA_ARGS__)
        #define CHESS_ENGINE_LOG_BEGIN(tag, fmt, ...) \
            CHESS_ENGINE_PRINTF(fmt, ##__VA_ARGS__)
        #define CHESS_ENGINE_LOG_ADD(tag, fmt, ...) \
            CHESS_ENGINE_PRINTF(fmt, ##__VA_ARGS__)
        #define CHESS_ENGINE_LOG_END(tag, fmt, ...) \
            CHESS_ENGINE_PRINTF(fmt "\n", ##__VA_ARGS__)
    #else
        #define CHESS_ENGINE_LOG(tag, fmt, ...) \
            CHESS_ENGINE_PRINTF("DBG: ChessEngine: " tag ": " fmt "\n", ##__VA_ARGS__)
        #define CHESS_ENGINE_LOG_BEGIN(tag, fmt, ...) \
            CHESS_ENGINE_PRINTF("DBG: ChessEngine: " tag ": " fmt, ##__VA_ARGS__)
        #define CHESS_ENGINE_LOG_ADD(tag, fmt, ...) \
            CHESS_ENGINE_PRINTF(fmt, ##__VA_ARGS__)
        #define CHESS_ENGINE_LOG_END(tag, fmt, ...) \
            CHESS_ENGINE_PRINTF(fmt "\n", ##__VA_ARGS__)
    #endif
#else
    #define CHESS_ENGINE_LOG(tag, fmt, ...) \
        CHESS_ENGINE_PRINTF_NOOP
    #define CHESS_ENGINE_LOG_BEGIN(tag, fmt, ...) \
        CHESS_ENGINE_PRINTF_NOOP
    #define CHESS_ENGINE_LOG_ADD(tag, fmt, ...) \
        CHESS_ENGINE_PRINTF_NOOP
    #define CHESS_ENGINE_LOG_END(tag, fmt, ...) \
        CHESS_ENGINE_PRINTF_NOOP
#endif
// clang-format on
