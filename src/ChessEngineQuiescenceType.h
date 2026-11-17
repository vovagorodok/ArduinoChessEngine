#pragma once
#include "ChessEngineDefinesArduino.h"

namespace ChessEngine {
enum class QuiescenceType : uint8_t {
    Skip,
    MaybeOpponentInCheck,
    Tactical
};
}
