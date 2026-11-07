#include "ChessEngineUci.h"
#include "ArduinoChessEngine.h"
#include "ChessEngineLogger.h"
#include "ChessEngineMove.h"
#include "ChessEngineConsts.h"

namespace ChessEngine {
#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
constexpr Milliseconds GO_TIME_DIVISOR = 30;

Uci::Uci() :
    levels(),
    isInputBusy(),
    buff() {
}

void Uci::begin() {
    ArduinoChessEngine.begin(DEFAULT_FEN);
    ArduinoChessEngine.setCyclicPull(*this, 200);

    while (true) {
        const String input = readInput();
        if (!input.length()) {
            continue;
        }
        if (input == "quit") {
            ArduinoChessEngine.stop();
            break;
        }
        handle(input);
    }
}

void Uci::pull() {
    if (checkInput() && flushInput() == "stop") {
        ArduinoChessEngine.stop();
    }
}

void Uci::handle(const String& input) {
    if (input == "uci") {
        handleUci();
    } else if (input == "isready") {
        CHESS_ENGINE_PRINTF("readyok\n");
    } else if (input == "ucinewgame") {
        ArduinoChessEngine.begin(DEFAULT_FEN);
    } else if (input.indexOf("setoption") == 0) {
        handleSetOption(input);
    } else if (input.indexOf("position") == 0) {
        handlePosition(input);
    } else if (input == "go" || input.indexOf("go ") == 0) {
        handleGo(input);
    } else if (input == "stop") {
        ArduinoChessEngine.stop();
    }
}

void Uci::handleUci() {
    CHESS_ENGINE_PRINTF("id name ArduinoChessEngine\n");
    CHESS_ENGINE_PRINTF("id author vovagorodok, dkwach\n");
    CHESS_ENGINE_PRINTF("option name Level type spin default 7 min 0 max 7\n");
    CHESS_ENGINE_PRINTF("uciok\n");
}

void Uci::handleSetOption(const String& input) {
    if (input.indexOf("setoption name Level value ") != 0) {
        return;
    }
    const size_t level = static_cast<size_t>(input.substring(27).toInt());
    ArduinoChessEngine.setOptions(levels.get(level));
}

void Uci::handlePosition(const String& input) {
    if (input.indexOf("position startpos") == 0) {
        ArduinoChessEngine.begin(DEFAULT_FEN);
        const int movesPos = input.indexOf(" moves ");
        if (movesPos >= 0) {
            applyMoves(input.substring(movesPos + 7));
        }
        return;
    }
    if (input.indexOf("position fen ") != 0) {
        return;
    }
    const String rest = input.substring(13);
    const int movesPos = rest.indexOf(" moves ");
    const String fen = movesPos < 0 ? rest : rest.substring(0, movesPos);
    ArduinoChessEngine.begin(fen);
    if (movesPos >= 0) {
        applyMoves(rest.substring(movesPos + 7));
    }
}

void Uci::handleGo(const String& input) {
    const Options saved = ArduinoChessEngine.getOptions();
    Options options = saved;
    Milliseconds wtime = 0;
    Milliseconds btime = 0;
    Milliseconds movetime = 0;
    bool hasDepth = false;

    String rest = input.length() > 2 ? input.substring(3) : String();
    rest.trim();
    while (rest.length()) {
        const int paramEnd = rest.indexOf(' ');
        const String param = paramEnd < 0 ? rest : rest.substring(0, paramEnd);
        rest = paramEnd < 0 ? String() : rest.substring(paramEnd + 1);
        rest.trim();

        if (param == "infinite") {
            options.timeLimit = TimeLimitOption::max();
            continue;
        }
        if (param == "movetime" || param == "wtime" || param == "btime" || param == "depth") {
            const int valueEnd = rest.indexOf(' ');
            const String value = valueEnd < 0 ? rest : rest.substring(0, valueEnd);
            rest = valueEnd < 0 ? String() : rest.substring(valueEnd + 1);
            rest.trim();
            const long number = value.toInt();
            if (param == "movetime") {
                movetime = static_cast<Milliseconds>(number);
            } else if (param == "wtime") {
                wtime = static_cast<Milliseconds>(number);
            } else if (param == "btime") {
                btime = static_cast<Milliseconds>(number);
            } else {
                options.maxDepth = static_cast<Depth>(number);
                hasDepth = true;
            }
        }
    }

    if (movetime) {
        options.timeLimit = movetime < MIN_TIME_LIMIT ? MIN_TIME_LIMIT : movetime;
    } else if (wtime || btime) {
        const Milliseconds remain = ArduinoChessEngine.isWhiteTurn() ? wtime : btime;
        const Milliseconds limit = remain / GO_TIME_DIVISOR;
        options.timeLimit = limit < MIN_TIME_LIMIT ? MIN_TIME_LIMIT : limit;
    }
    if (!hasDepth && (movetime || wtime || btime)) {
        options.maxDepth = MaxDepthOption::max();
    }

    ArduinoChessEngine.setOptions(options);
    const Move move = ArduinoChessEngine.solve();
    ArduinoChessEngine.setOptions(saved);

    if (move.isNone()) {
        CHESS_ENGINE_PRINTF("bestmove 0000\n");
    } else {
        CHESS_ENGINE_PRINTF("bestmove %s\n", convertMoveToUci(move).c_str());
    }
}

void Uci::applyMoves(String moves) {
    moves.trim();
    while (moves.length()) {
        const int moveEnd = moves.indexOf(' ');
        const String uci = moveEnd < 0 ? moves : moves.substring(0, moveEnd);
        moves = moveEnd < 0 ? String() : moves.substring(moveEnd + 1);
        ArduinoChessEngine.make(uci);
    }
}

bool Uci::checkInput() {
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

String Uci::flushInput() {
    String input = buff;
    buff = String();
    input.trim();
    return input;
}

String Uci::readInput() {
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
