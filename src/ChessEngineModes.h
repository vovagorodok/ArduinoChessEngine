#pragma once

namespace ChessEngine {
template <typename Tag>
class BoolMode {
 public:
    enum Value {
        Inactive,
        Active
    };

    constexpr BoolMode(Value value) :
        value(value) {}
    constexpr operator bool() const { return value == Active; }

 private:
    Value value;
};

using NullMoveMode = BoolMode<struct NullMoveModeTag>;
using LazyEvalMode = BoolMode<struct LazyEvalModeTag>;

enum class BoardEvalMode {
    Material,
    Positional
};

struct Modes {
    NullMoveMode nullMove = NullMoveMode::Inactive;
    LazyEvalMode lazyEval = LazyEvalMode::Inactive;
    BoardEvalMode boardEval = BoardEvalMode::Positional;
};
}
