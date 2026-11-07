#include "ChessEngineGame.h"
#include "ArduinoChessEngine.h"
#include "ChessEngineLogger.h"

namespace ChessEngine {
#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

Game::Game() :
    isInputBusy(),
    buff() {
}

void Game::begin() {
    Serial.println("Begin");
    ArduinoChessEngine.begin(DEFAULT_FEN);
    PieceColor side = PieceColor::White;

    while (true) {
        printRound();
        if (ArduinoChessEngine.isEnd()) {
            break;
        }
        if (ArduinoChessEngine.getTurn() == side) {
            const String input = readInput();
            if (input == "end" || input == "exit") {
                Serial.println("End");
                break;
            } else if (input.indexOf("begin") == 0) {
                const auto fen = input.length() > 5 ? input.substring(6) : DEFAULT_FEN;
                ArduinoChessEngine.begin(fen);
                Serial.println("Begin");
            } else if (input.indexOf("undo") == 0) {
                size_t undo = input.length() > 4 ? input.substring(5).toInt() : 2;
                ArduinoChessEngine.undo(undo) ? CHESS_ENGINE_PRINTF("Undo: %d\n", undo) :
                                                CHESS_ENGINE_PRINTF("Illegal undo: %d\n", undo);
            } else if (input.indexOf("time") == 0) {
                Seconds time = input.length() > 4 ? input.substring(5).toInt() : 60;
                ArduinoChessEngine.setTimeLimit(convertSecondsToMilliseconds(time));
                CHESS_ENGINE_PRINTF("Time: %s\n", convertMillisecondsToStr(ArduinoChessEngine.getTimeLimit()).c_str());
            } else if (input.indexOf("side") == 0) {
                if (input.length() == 6) {
                    side = convertCharToPieceColor(input.charAt(5));
                }
                CHESS_ENGINE_PRINTF("Side: %c\n", convertPieceColorToChar(side));
            } else if (input.length() == 2) {
                CHESS_ENGINE_PRINTF("Destinations: ");
                for (const Move& move : ArduinoChessEngine.createMoves(input)) {
                    CHESS_ENGINE_PRINTF(convertPositionToStr(move.positions.dst).c_str());
                    CHESS_ENGINE_PRINTF(" ");
                }
                CHESS_ENGINE_PRINTF("\n");
            } else if (input.length() == 4 || input.length() == 5) {
                const auto move = ArduinoChessEngine.make(input);
                move.isNone() ? CHESS_ENGINE_PRINTF("Illegal move\n") :
                                CHESS_ENGINE_PRINTF("Moved: %s\n", convertMoveToUci(move).c_str());
            } else {
                CHESS_ENGINE_PRINTF("Invalid input\n");
            }
        } else {
            CHESS_ENGINE_PRINTF("Solving..\n");
            const auto move = ArduinoChessEngine.make(ArduinoChessEngine.solve());
            if (move.isNone() && !ArduinoChessEngine.isEnd()) {
                CHESS_ENGINE_PRINTF("Error!\n");
            }
            CHESS_ENGINE_PRINTF("Moved: %s\n", convertMoveToUci(move).c_str());
        }
    }
}

void Game::pull() {
    if (checkInput() && flushInput() == "stop") {
        ArduinoChessEngine.stop();
        CHESS_ENGINE_PRINTF("Stopped\n");
    }
}

void Game::printRound() {
    const Board& board = ArduinoChessEngine.getBoard();
    const Position check = ArduinoChessEngine.getCheck();
    const Move lastMove = ArduinoChessEngine.getLastMove();

    CHESS_ENGINE_PRINTF("%s\n", ArduinoChessEngine.createFen().c_str());
    for (Position row = 0; row < BOARD_SQUARES_NUM; row++) {
        bool isSpaceNeeded = true;
        for (Position col = 0; col < BOARD_SQUARES_NUM; col++) {
            const Position pos = row * BOARD_SQUARES_NUM + col;
            const char piece = board[pos].toChar();
            if (pos == check) {
                if (isSpaceNeeded) {
                    CHESS_ENGINE_PRINTF("(");
                }
                CHESS_ENGINE_PRINTF("%c", piece);
                CHESS_ENGINE_PRINTF(")");
                isSpaceNeeded = false;
            } else if (!lastMove.isNone() && (pos == lastMove.positions.src || pos == lastMove.positions.dst)) {
                if (isSpaceNeeded) {
                    CHESS_ENGINE_PRINTF("[");
                }
                CHESS_ENGINE_PRINTF("%c", piece);
                CHESS_ENGINE_PRINTF("]");
                isSpaceNeeded = false;
            } else {
                if (isSpaceNeeded) {
                    CHESS_ENGINE_PRINTF(" ");
                }
                CHESS_ENGINE_PRINTF("%c", piece);
                isSpaceNeeded = true;
            }
        }
        if (isSpaceNeeded) {
            CHESS_ENGINE_PRINTF(" ");
        }
        CHESS_ENGINE_PRINTF("%d\n", BOARD_SQUARES_NUM - row);
    }
    CHESS_ENGINE_PRINTF(" a b c d e f g h\n");
    printRoundStatus(ArduinoChessEngine.getRoundStatus());
}

void Game::printRoundStatus(RoundStatus status) {
    switch (status) {
        case RoundStatus::Check:
            CHESS_ENGINE_PRINTF("Check\n");
            break;
        case RoundStatus::Checkmate:
            CHESS_ENGINE_PRINTF("Checkmate\n");
            break;
        case RoundStatus::Stalemate:
            CHESS_ENGINE_PRINTF("Stalemate\n");
            break;
        case RoundStatus::DeadPosition:
            CHESS_ENGINE_PRINTF("Dead Position\n");
            break;
        case RoundStatus::FiftyMove:
            CHESS_ENGINE_PRINTF("Fifty Move\n");
            break;
        case RoundStatus::ThreefoldRepetition:
            CHESS_ENGINE_PRINTF("Threefold Repetition\n");
            break;
        default:
            break;
    }
}

bool Game::checkInput() {
    if (isInputBusy) {
        return false;
    }
    while (Serial.available()) {
        const char ch = static_cast<char>(Serial.read());
        if (ch == '\n' || ch == '\r') {
            return true;
        }
        buff += ch;
    }
    return false;
}

String Game::flushInput() {
    String input = buff;
    buff = String();
    return input;
}

String Game::readInput() {
    String input{};
    isInputBusy = true;
    while (!input.length()) {
        input = Serial.readString();
    }
    isInputBusy = false;
    input.trim();
    return input;
}
}
