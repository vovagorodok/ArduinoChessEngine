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
            NullMoveOption::Off,
            FutilityOption::Off,
            LazyEvalOption::Off,
            AspirationOption::Off,
            RandomOption::On,
            MaxDepthOption::min(),
            TimeLimitOption::min()
        },
        /* 1 */ Options{
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            AspirationOption::On,
            RandomOption::Off,
            MaxDepthOption::min(),
            TimeLimitOption::min()
        },
        /* 2 */ Options{
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            AspirationOption::On,
            RandomOption::Off,
            MaxDepthOption::min() + 1 * (MaxDepthOption::max() - MaxDepthOption::min()) / 6,
            TimeLimitOption::min() + 1 * (TimeLimitOption::max() - TimeLimitOption::min()) / 6,
        },
        /* 3 */ Options{
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            AspirationOption::On,
            RandomOption::Off,
            MaxDepthOption::min() + 2 * (MaxDepthOption::max() - MaxDepthOption::min()) / 6,
            TimeLimitOption::min() + 2 * (TimeLimitOption::max() - TimeLimitOption::min()) / 6,
        },
        /* 4 */ Options{
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            AspirationOption::On,
            RandomOption::Off,
            MaxDepthOption::min() + 3 * (MaxDepthOption::max() - MaxDepthOption::min()) / 6,
            TimeLimitOption::min() + 3 * (TimeLimitOption::max() - TimeLimitOption::min()) / 6,
        },
        /* 5 */ Options{
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            AspirationOption::On,
            RandomOption::Off,
            MaxDepthOption::min() + 4 * (MaxDepthOption::max() - MaxDepthOption::min()) / 6,
            TimeLimitOption::min() + 4 * (TimeLimitOption::max() - TimeLimitOption::min()) / 6,
        },
        /* 6 */ Options{
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            AspirationOption::On,
            RandomOption::Off,
            MaxDepthOption::min() + 5 * (MaxDepthOption::max() - MaxDepthOption::min()) / 6,
            TimeLimitOption::min() + 5 * (TimeLimitOption::max() - TimeLimitOption::min()) / 6,
        },
        /* 7 */ Options{
            NullMoveOption::On,
            FutilityOption::On,
            LazyEvalOption::On,
            AspirationOption::On,
            RandomOption::Off,
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
