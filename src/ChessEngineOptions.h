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

using RandomOption = BoolOption<struct RandomOptionTag>;
using NullMoveOption = BoolOption<struct NullMoveOptionTag>;
using FutilityOption = BoolOption<struct FutilityOptionTag>;
using LazyEvalOption = BoolOption<struct LazyEvalOptionTag>;
using QuiescenceOption = BoolOption<struct QuiescenceOptionTag>;
using AspirationOption = BoolOption<struct AspirationOptionTag>;
using ExtraDepthOption = BoolOption<struct ExtraDepthOptionTag>;
using MaxDepthOption = IntOption<Depth, NEGAMAX_MIN_DEPTH, NEGAMAX_MAX_DEPTH>;
using TimeLimitOption = IntOption<Milliseconds, MIN_TIME_LIMIT, MAX_TIME_LIMIT>;

struct Options {
    RandomOption random = RandomOption::Off;
    NullMoveOption nullMove = NullMoveOption::On;
    FutilityOption futility = FutilityOption::On;
    LazyEvalOption lazyEval = LazyEvalOption::On;
    QuiescenceOption quiescence = QuiescenceOption::On;
    AspirationOption aspiration = AspirationOption::On;
    ExtraDepthOption extraDepth = ExtraDepthOption::On;
    MaxDepthOption maxDepth = MaxDepthOption::max();
    TimeLimitOption timeLimit = TimeLimitOption::max();

    constexpr Options() = default;
    constexpr Options(RandomOption random, NullMoveOption nullMove, FutilityOption futility, LazyEvalOption lazyEval,
                      QuiescenceOption quiescence, AspirationOption aspiration, ExtraDepthOption extraDepth,
                      MaxDepthOption maxDepth, TimeLimitOption timeLimit) :
        random(random),
        nullMove(nullMove),
        futility(futility),
        lazyEval(lazyEval),
        quiescence(quiescence),
        aspiration(aspiration),
        extraDepth(extraDepth),
        maxDepth(maxDepth),
        timeLimit(timeLimit) {}
};
}
