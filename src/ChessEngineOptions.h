#pragma once
#include "ChessEngineConsts.h"

namespace ChessEngine {
template <typename Tag>
class BoolOption {
 public:
    enum Value {
        Off,
        On
    };

    constexpr BoolOption(Value value) :
        value(value) {}
    constexpr operator bool() const { return value == On; }

 private:
    Value value;
};

template <typename T, T Min, T Max>
class IntOption {
 public:
    constexpr IntOption(T value = Max) :
        value(value >= Min && value <= Max ? value : Max) {}
    constexpr operator T() const { return value; }
    static constexpr T min() { return Min; }
    static constexpr T max() { return Max; }

 private:
    T value;
};

using NullMoveOption = BoolOption<struct NullMoveOptionTag>;
using FutilityOption = BoolOption<struct FutilityOptionTag>;
using LazyEvalOption = BoolOption<struct LazyEvalOptionTag>;
using AspirationOption = BoolOption<struct AspirationOptionTag>;
using RandomOption = BoolOption<struct RandomOptionTag>;
using MaxDepthOption = IntOption<Depth, NEGAMAX_MIN_DEPTH, NEGAMAX_MAX_DEPTH>;
using TimeLimitOption = IntOption<Milliseconds, MIN_TIME_LIMIT, MAX_TIME_LIMIT>;

struct Options {
    NullMoveOption nullMove = NullMoveOption::On;
    FutilityOption futility = FutilityOption::On;
    LazyEvalOption lazyEval = LazyEvalOption::On;
    AspirationOption aspiration = AspirationOption::On;
    RandomOption random = RandomOption::Off;
    MaxDepthOption maxDepth = MaxDepthOption::max();
    TimeLimitOption timeLimit = TimeLimitOption::max();

    constexpr Options() = default;
    constexpr Options(NullMoveOption nullMove, FutilityOption futility, LazyEvalOption lazyEval,
                      AspirationOption aspiration, RandomOption random, MaxDepthOption maxDepth,
                      TimeLimitOption timeLimit) :
        nullMove(nullMove),
        futility(futility),
        lazyEval(lazyEval),
        aspiration(aspiration),
        random(random),
        maxDepth(maxDepth),
        timeLimit(timeLimit) {}
};
}
