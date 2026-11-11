#pragma once
#include "ChessEngineOptions.h"
#include <array>

namespace ChessEngine {
#define MAX_LEVELS 8

class Levels {
 public:
    // clang-format off
    Levels() :
        levels{
        /* 0 */ Options{
            RandomOption::On,
            NullMoveOption::Off,
            FutilityOption::Off,
            LazyEvalOption::Off,
            QuiescenceOption::Off,
            AspirationOption::Off,
            ExtraDepthOption::Off,
            MaxDepthOption::min(),
            TimeLimitOption::min()
        },
        /* 1 */ Options{
            RandomOption::Off,
            NullMoveOption::Off,
            FutilityOption::Off,
            LazyEvalOption::Off,
            QuiescenceOption::Off,
            AspirationOption::Off,
            ExtraDepthOption::Off,
            MaxDepthOption::min(),
            TimeLimitOption::min()
        },
        /* 2 */ Options{
            RandomOption::Off,
            NullMoveOption::Off,
            FutilityOption::Off,
            LazyEvalOption::Off,
            QuiescenceOption::On,
            AspirationOption::Off,
            ExtraDepthOption::Off,
            MaxDepthOption::min(),
            TimeLimitOption::min()
        },
        /* 3 */ Options{
            RandomOption::Off,
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            QuiescenceOption::On,
            AspirationOption::On,
            ExtraDepthOption::On,
            MaxDepthOption::min(),
            TimeLimitOption::min()
        },
        /* 4 */ Options{
            RandomOption::Off,
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            QuiescenceOption::On,
            AspirationOption::On,
            ExtraDepthOption::On,
            MaxDepthOption::min() + 1 * (MaxDepthOption::max() - MaxDepthOption::min()) / 4,
            TimeLimitOption::min() + 1 * (TimeLimitOption::max() - TimeLimitOption::min()) / 4,
        },
        /* 5 */ Options{
            RandomOption::Off,
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            QuiescenceOption::On,
            AspirationOption::On,
            ExtraDepthOption::On,
            MaxDepthOption::min() + 2 * (MaxDepthOption::max() - MaxDepthOption::min()) / 4,
            TimeLimitOption::min() + 2 * (TimeLimitOption::max() - TimeLimitOption::min()) / 4,
        },
        /* 6 */ Options{
            RandomOption::Off,
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            QuiescenceOption::On,
            AspirationOption::On,
            ExtraDepthOption::On,
            MaxDepthOption::min() + 3 * (MaxDepthOption::max() - MaxDepthOption::min()) / 4,
            TimeLimitOption::min() + 3 * (TimeLimitOption::max() - TimeLimitOption::min()) / 4,
        },
        /* 7 */ Options{
            RandomOption::Off,
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            QuiescenceOption::On,
            AspirationOption::On,
            ExtraDepthOption::On,
            MaxDepthOption::max(),
            TimeLimitOption::max()
        }
    } {}
    // clang-format on

    const Options& get(size_t level) const {
        if (level >= levels.size()) {
            return levels.front();
        }
        return levels[level];
    }

 private:
    const std::array<Options, MAX_LEVELS> levels;
};
}
