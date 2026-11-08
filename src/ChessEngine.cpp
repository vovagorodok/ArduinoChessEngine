#include "ChessEngine.h"
#include "ChessEngineLogger.h"
#include <utility>

namespace ChessEngine {
namespace {
#define TAG "Engine"

#ifndef CHESS_ENGINE_MAP_QUALIFIER
#define CHESS_ENGINE_MAP_QUALIFIER constexpr
#endif

constexpr uint8_t WHITE_PROMOTION_ROW = 8;
constexpr uint8_t BLACK_PROMOTION_ROW = 1;
constexpr uint8_t SINGLE_PAWN_MOVE = BOARD_SQUARES_NUM;
constexpr uint8_t DOUBLE_PAWN_MOVE = 2 * SINGLE_PAWN_MOVE;
constexpr HalfMove FIFTY_MOVE_THRESHOLD = 100;

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER PositionToNumMap COL_NUMS{{
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8,
    1, 2, 3, 4, 5, 6, 7, 8
}};
CHESS_ENGINE_MAP_QUALIFIER PositionToNumMap ROW_NUMS{{
    8, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1
}};
CHESS_ENGINE_MAP_QUALIFIER PositionToNumMap DIAG1_NUMS{{
    1, 2,  3,  4,  5,  6,  7,  8,
    2, 3,  4,  5,  6,  7,  8,  9,
    3, 4,  5,  6,  7,  8,  9, 10,
    4, 5,  6,  7,  8,  9, 10, 11,
    5, 6,  7,  8,  9, 10, 11, 12,
    6, 7,  8,  9, 10, 11, 12, 13,
    7, 8,  9, 10, 11, 12, 13, 14,
    8, 9, 10, 11, 12, 13, 14, 15
}};
CHESS_ENGINE_MAP_QUALIFIER PositionToNumMap DIAG2_NUMS{{
     8,  7,  6,  5,  4,  3, 2, 1,
     9,  8,  7,  6,  5,  4, 3, 2,
    10,  9,  8,  7,  6,  5, 4, 3,
    11, 10,  9,  8,  7,  6, 5, 4,
    12, 11, 10,  9,  8,  7, 6, 5,
    13, 12, 11, 10,  9,  8, 7, 6,
    14, 13, 12, 11, 10,  9, 8, 7,
    15, 14, 13, 12, 11, 10, 9, 8
}};
// clang-format on

inline bool isSameStraight(Position pos1, Position pos2) {
    return COL_NUMS[pos1] == COL_NUMS[pos2] || ROW_NUMS[pos1] == ROW_NUMS[pos2];
}

inline bool isSameDiagonal(Position pos1, Position pos2) {
    return DIAG1_NUMS[pos1] == DIAG1_NUMS[pos2] || DIAG2_NUMS[pos1] == DIAG2_NUMS[pos2];
}

inline bool isSameStraightOrDiagonal(Position pos1, Position pos2) {
    return isSameStraight(pos1, pos2) || isSameDiagonal(pos1, pos2);
}

inline bool isWhiteSquare(Position pos) {
    return (COL_NUMS[pos] + ROW_NUMS[pos]) % 2 == 1;
}

inline bool isBlackSquare(Position pos) {
    return (COL_NUMS[pos] + ROW_NUMS[pos]) % 2 == 0;
}

inline bool isWhitePawnPromotion(Position dst) {
    return ROW_NUMS[dst] == WHITE_PROMOTION_ROW;
}

inline bool isBlackPawnPromotion(Position dst) {
    return ROW_NUMS[dst] == BLACK_PROMOTION_ROW;
}

inline bool isWhitePawnNearPromotion(Position dst) {
    return ROW_NUMS[dst] > 5;
}

inline bool isBlackPawnNearPromotion(Position dst) {
    return ROW_NUMS[dst] < 4;
}

inline bool isWhitePawnDoubleMove(const Positions& positions) {
    return positions.dst == positions.src - DOUBLE_PAWN_MOVE;
}

inline bool isBlackPawnDoubleMove(const Positions& positions) {
    return positions.dst == positions.src + DOUBLE_PAWN_MOVE;
}

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER PieceToWeightMap MATERIAL_WEIGHTS{{
    /* Empty */    0,
    /* Pawn */   100,
    /* Knight */ 320,
    /* Bishop */ 330,
    /* Rook */   500,
    /* Queen */  900,
    /* King */     0,
}};
// clang-format on

// Piece-Square Tables: https://chessprogramming.org/Simplified_Evaluation_Function
// clang-format off
CHESS_ENGINE_MAP_QUALIFIER PiecePositionToWeightMap WHITE_PST_WEIGHTS{{
    {  /* Pawn late */
       0,  0,  0,  0,  0,  0,  0,  0,
      80, 80, 80, 80, 80, 80, 80, 80,
      50, 50, 50, 50, 50, 50, 50, 50,
      30, 30, 30, 30, 30, 30, 30, 30,
      20, 20, 20, 20, 20, 20, 20, 20,
      10, 10, 10, 10, 10, 10, 10, 10,
      10, 10, 10, 10, 10, 10, 10, 10,
       0,  0,  0,  0,  0,  0,  0,  0
    },
    {  /* Pawn */
        0,  0,   0,   0,   0,   0,  0,  0,
       50, 50,  50,  50,  50,  50, 50, 50,
       10, 10,  20,  30,  30,  20, 10, 10,
        5,  5,  10,  25,  25,  10,  5,  5,
        0,  0,   0,  20,  20,   0,  0,  0,
        5, -5, -10,   0,   0, -10, -5,  5,
        5, 10,  10, -20, -20,  10, 10,  5,
        0,  0,   0,   0,   0,   0,  0,  0
    },
    {  /* Knight */
      -50, -40, -30, -30, -30, -30, -40, -50,
      -40, -20,   0,   0,   0,   0, -20, -40,
      -30,   0,  10,  15,  15,  10,   0, -30,
      -30,   5,  15,  20,  20,  15,   5, -30,
      -30,   0,  15,  20,  20,  15,   0, -30,
      -30,   5,  10,  15,  15,  10,   5, -30,
      -40, -20,   0,   5,   5,   0, -20, -40,
      -50, -40, -30, -30, -30, -30, -40, -50
    },
    {  /* Bishop */
      -20, -10, -10, -10, -10, -10, -10, -20,
      -10,   0,   0,   0,   0,   0,   0, -10,
      -10,   0,   5,  10,  10,   5,   0, -10,
      -10,   5,   5,  10,  10,   5,   5, -10,
      -10,   0,  10,  10,  10,  10,   0, -10,
      -10,  10,  10,  10,  10,  10,  10, -10,
      -10,   5,   0,   0,   0,   0,   5, -10,
      -20, -10, -10, -10, -10, -10, -10, -20
    },
    {  /* Rook */
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
       -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    },
    {  /* Queen */
      -20, -10, -10,  -5,  -5, -10, -10, -20,
      -10,   0,   0,   0,   0,   0,   0, -10,
      -10,   0,   5,   5,   5,   5,   0, -10,
       -5,   0,   5,   5,   5,   5,   0,  -5,
        0,   0,   5,   5,   5,   5,   0,  -5,
      -10,   5,   5,   5,   5,   5,   0, -10,
      -10,   0,   5,   0,   0,   0,   0, -10,
      -20, -10, -10,  -5,  -5, -10, -10, -20
    },
    {  /* King */
      -30, -40, -40, -50, -50, -40, -40, -30,
      -30, -40, -40, -50, -50, -40, -40, -30,
      -30, -40, -40, -50, -50, -40, -40, -30,
      -30, -40, -40, -50, -50, -40, -40, -30,
      -20, -30, -30, -40, -40, -30, -30, -20,
      -10, -20, -20, -20, -20, -20, -20, -10,
       10,  10, -10, -10, -10, -10,  10,  10,
       10,  40,  30,   0,   0,   0,  50,  10
    },
    {  /* King late */
      -50, -40, -30, -20, -20, -30, -40, -50,
      -30, -20, -10,   0,   0, -10, -20, -30,
      -30, -10,  20,  30,  30,  20, -10, -30,
      -30, -10,  30,  40,  40,  30, -10, -30,
      -30, -10,  30,  40,  40,  30, -10, -30,
      -30, -10,  20,  30,  30,  20, -10, -30,
      -30, -30,   0,   0,   0,   0, -30, -30,
      -50, -30, -30, -30, -30, -30, -30, -50
    }
}};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER PiecePositionToWeightMap BLACK_PST_WEIGHTS{{
    {  /* Pawn late */
       0,  0,  0,  0,  0,  0,  0,  0,
      10, 10, 10, 10, 10, 10, 10, 10,
      10, 10, 10, 10, 10, 10, 10, 10,
      20, 20, 20, 20, 20, 20, 20, 20,
      30, 30, 30, 30, 30, 30, 30, 30,
      50, 50, 50, 50, 50, 50, 50, 50,
      80, 80, 80, 80, 80, 80, 80, 80,
       0,  0,  0,  0,  0,  0,  0,  0
    },
    {  /* Pawn */
        0,  0,   0,   0,   0,   0,  0,  0,
        5, 10,  10, -20, -20,  10, 10,  5,
        5, -5, -10,   0,   0, -10, -5,  5,
        0,  0,   0,  20,  20,   0,  0,  0,
        5,  5,  10,  25,  25,  10,  5,  5,
       10, 10,  20,  30,  30,  20, 10, 10,
       50, 50,  50,  50,  50,  50, 50, 50,
        0,  0,   0,   0,   0,   0,  0,  0
    },
    {  /* Knight */
      -50, -40, -30, -30, -30, -30, -40, -50,
      -40, -20,   0,   0,   0,   0, -20, -40,
      -30,   5,  10,  15,  15,  10,   5, -30,
      -30,   0,  15,  20,  20,  15,   0, -30,
      -30,   5,  15,  20,  20,  15,   5, -30,
      -30,   0,  10,  15,  15,  10,   0, -30,
      -40, -20,   0,   5,   5,   0, -20, -40,
      -50, -40, -30, -30, -30, -30, -40, -50
    },
    {  /* Bishop */
      -20, -10, -10, -10, -10, -10, -10, -20,
      -10,   5,   0,   0,   0,   0,   5, -10,
      -10,  10,  10,  10,  10,  10,  10, -10,
      -10,   0,  10,  10,  10,  10,   0, -10,
      -10,   5,   5,  10,  10,   5,   5, -10,
      -10,   0,   5,  10,  10,   5,   0, -10,
      -10,   0,   0,   0,   0,   0,   0, -10,
      -20, -10, -10, -10, -10, -10, -10, -20
    },
    {  /* Rook */
       0,  0,  0,  5,  5,  0,  0,  0,
      -5,  0,  0,  0,  0,  0,  0, -5,
      -5,  0,  0,  0,  0,  0,  0, -5,
      -5,  0,  0,  0,  0,  0,  0, -5,
      -5,  0,  0,  0,  0,  0,  0, -5,
      -5,  0,  0,  0,  0,  0,  0, -5,
       5, 10, 10, 10, 10, 10, 10,  5,
       0,  0,  0,  0,  0,  0,  0,  0
    },
    {  /* Queen */
      -20, -10, -10,  -5,  -5, -10, -10, -20,
      -10,   0,   5,   0,   0,   0,   0, -10,
      -10,   5,   5,   5,   5,   5,   0, -10,
        0,   0,   5,   5,   5,   5,   0,  -5,
       -5,   0,   5,   5,   5,   5,   0,  -5,
      -10,   0,   5,   5,   5,   5,   0, -10,
      -10,   0,   0,   0,   0,   0,   0, -10,
      -20, -10, -10,  -5,  -5, -10, -10, -20
    },
    {  /* King */
       10,  40,  30,   0,   0,   0,  50,  10,
       10,  10, -10, -10, -10, -10,  10,  10,
      -10, -20, -20, -20, -20, -20, -20, -10,
      -20, -30, -30, -40, -40, -30, -30, -20,
      -30, -40, -40, -50, -50, -40, -40, -30,
      -30, -40, -40, -50, -50, -40, -40, -30,
      -30, -40, -40, -50, -50, -40, -40, -30,
      -30, -40, -40, -50, -50, -40, -40, -30
    },
    {  /* King late */
      -50, -30, -30, -30, -30, -30, -30, -50,
      -30, -30,   0,   0,   0,   0, -30, -30,
      -30, -10,  20,  30,  30,  20, -10, -30,
      -30, -10,  30,  40,  40,  30, -10, -30,
      -30, -10,  30,  40,  40,  30, -10, -30,
      -30, -10,  20,  30,  30,  20, -10, -30,
      -30, -20, -10,   0,   0, -10, -20, -30,
      -50, -40, -30, -20, -20, -30, -40, -50
    }
}};
// clang-format on

// clang-format off
enum UciPosition : Position::ValueType {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
    none = Position::NONE_VALUE
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER CastlingMove WHITE_KING_CASTLING_MOVE{
    {PieceType::WhiteKing, PieceType::WhiteRook},
    {e1, g1},
    {h1, f1},
    {f1, g1},
    MoveType::CastleKing
};
CHESS_ENGINE_MAP_QUALIFIER CastlingMove WHITE_QUEEN_CASTLING_MOVE{
    {PieceType::WhiteKing, PieceType::WhiteRook},
    {e1, c1},
    {a1, d1},
    {d1, c1, b1},
    MoveType::CastleQueen
};
CHESS_ENGINE_MAP_QUALIFIER CastlingMove BLACK_KING_CASTLING_MOVE{
    {PieceType::BlackKing, PieceType::BlackRook},
    {e8, g8},
    {h8, f8},
    {f8, g8},
    MoveType::CastleKing
};
CHESS_ENGINE_MAP_QUALIFIER CastlingMove BLACK_QUEEN_CASTLING_MOVE{
    {PieceType::BlackKing, PieceType::BlackRook},
    {e8, c8},
    {a8, d8},
    {d8, c8, b8},
    MoveType::CastleQueen
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER DoubleMoveSrcToEnPassantMovesMap WHITE_PAWN_DOUBLE_MOVE_SRC_TO_EN_PASSANT_MOVES{
    a2,
    h2,
    {
      /* a2 */ {a3, a4, none, b4},
      /* b2 */ {b3, b4, a4, c4},
      /* c2 */ {c3, c4, b4, d4},
      /* d2 */ {d3, d4, c4, e4},
      /* e2 */ {e3, e4, d4, f4},
      /* f2 */ {f3, f4, e4, g4},
      /* g2 */ {g3, g4, f4, h4},
      /* h2 */ {h3, h4, g4, none},
    },
};
CHESS_ENGINE_MAP_QUALIFIER DoubleMoveSrcToEnPassantMovesMap BLACK_PAWN_DOUBLE_MOVE_SRC_TO_EN_PASSANT_MOVES{
    a7,
    h7,
    {
      /* a7 */ {a6, a5, none, b5},
      /* b7 */ {b6, b5, a5, c5},
      /* c7 */ {c6, c5, b5, d5},
      /* d7 */ {d6, d5, c5, e5},
      /* e7 */ {e6, e5, d5, f5},
      /* f7 */ {f6, f5, e5, g5},
      /* g7 */ {g6, g5, f5, h5},
      /* h7 */ {h6, h5, g5, none},
    },
};
CHESS_ENGINE_MAP_QUALIFIER EnPassantDstToEnPassantRemovesMap WHITE_PAWN_DST_TO_EN_PASSANT_REMOVES{
    a6,
    h6,
    {
      /* a6 */ {a5, none, b5},
      /* b6 */ {b5, a5, c5},
      /* c6 */ {c5, b5, d5},
      /* d6 */ {d5, c5, e5},
      /* e6 */ {e5, d5, f5},
      /* f6 */ {f5, e5, g5},
      /* g6 */ {g5, f5, h5},
      /* h6 */ {h5, g5, none},
    },
};
CHESS_ENGINE_MAP_QUALIFIER EnPassantDstToEnPassantRemovesMap BLACK_PAWN_DST_TO_EN_PASSANT_REMOVES{
    a3,
    h3,
    {
      /* a3 */ {a4, none, b4},
      /* b3 */ {b4, a4, c4},
      /* c3 */ {c4, b4, d4},
      /* d3 */ {d4, c4, e4},
      /* e3 */ {e4, d4, f4},
      /* f3 */ {f4, e4, g4},
      /* g3 */ {g4, f4, h4},
      /* h3 */ {h4, g4, none},
    },
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER SourceToDirectionsMap<17> BISHOP_DESTINATIONS{
    /* a8 */ {{b7, c6, d5, e4, f3, g2, h1}},
    /* b8 */ {{c7, d6, e5, f4, g3, h2}, {a7}},
    /* c8 */ {{d7, e6, f5, g4, h3}, {b7, a6}},
    /* d8 */ {{e7, f6, g5, h4}, {c7, b6, a5}},
    /* e8 */ {{f7, g6, h5}, {d7, c6, b5, a4}},
    /* f8 */ {{g7, h6}, {e7, d6, c5, b4, a3}},
    /* g8 */ {{h7}, {f7, e6, d5, c4, b3, a2}},
    /* h8 */ {{g7, f6, e5, d4, c3, b2, a1}},
    /* a7 */ {{b6, c5, d4, e3, f2, g1}, {b8}},
    /* b7 */ {{c6, d5, e4, f3, g2, h1}, {a6}, {a8}, {c8}},
    /* c7 */ {{d6, e5, f4, g3, h2}, {b6, a5}, {b8}, {d8}},
    /* d7 */ {{e6, f5, g4, h3}, {c6, b5, a4}, {c8}, {e8}},
    /* e7 */ {{f6, g5, h4}, {d6, c5, b4, a3}, {d8}, {f8}},
    /* f7 */ {{g6, h5}, {e6, d5, c4, b3, a2}, {e8}, {g8}},
    /* g7 */ {{h6}, {f6, e5, d4, c3, b2, a1}, {f8}, {h8}},
    /* h7 */ {{g6, f5, e4, d3, c2, b1}, {g8}},
    /* a6 */ {{b5, c4, d3, e2, f1}, {b7, c8}},
    /* b6 */ {{c5, d4, e3, f2, g1}, {a5}, {a7}, {c7, d8}},
    /* c6 */ {{d5, e4, f3, g2, h1}, {b5, a4}, {b7, a8}, {d7, e8}},
    /* d6 */ {{e5, f4, g3, h2}, {c5, b4, a3}, {c7, b8}, {e7, f8}},
    /* e6 */ {{f5, g4, h3}, {d5, c4, b3, a2}, {d7, c8}, {f7, g8}},
    /* f6 */ {{g5, h4}, {e5, d4, c3, b2, a1}, {e7, d8}, {g7, h8}},
    /* g6 */ {{h5}, {f5, e4, d3, c2, b1}, {f7, e8}, {h7}},
    /* h6 */ {{g5, f4, e3, d2, c1}, {g7, f8}},
    /* a5 */ {{b4, c3, d2, e1}, {b6, c7, d8}},
    /* b5 */ {{c4, d3, e2, f1}, {a4}, {a6}, {c6, d7, e8}},
    /* c5 */ {{d4, e3, f2, g1}, {b4, a3}, {b6, a7}, {d6, e7, f8}},
    /* d5 */ {{e4, f3, g2, h1}, {c4, b3, a2}, {c6, b7, a8}, {e6, f7, g8}},
    /* e5 */ {{f4, g3, h2}, {d4, c3, b2, a1}, {d6, c7, b8}, {f6, g7, h8}},
    /* f5 */ {{g4, h3}, {e4, d3, c2, b1}, {e6, d7, c8}, {g6, h7}},
    /* g5 */ {{h4}, {f4, e3, d2, c1}, {f6, e7, d8}, {h6}},
    /* h5 */ {{g4, f3, e2, d1}, {g6, f7, e8}},
    /* a4 */ {{b3, c2, d1}, {b5, c6, d7, e8}},
    /* b4 */ {{c3, d2, e1}, {a3}, {a5}, {c5, d6, e7, f8}},
    /* c4 */ {{d3, e2, f1}, {b3, a2}, {b5, a6}, {d5, e6, f7, g8}},
    /* d4 */ {{e3, f2, g1}, {c3, b2, a1}, {c5, b6, a7}, {e5, f6, g7, h8}},
    /* e4 */ {{f3, g2, h1}, {d3, c2, b1}, {d5, c6, b7, a8}, {f5, g6, h7}},
    /* f4 */ {{g3, h2}, {e3, d2, c1}, {e5, d6, c7, b8}, {g5, h6}},
    /* g4 */ {{h3}, {f3, e2, d1}, {f5, e6, d7, c8}, {h5}},
    /* h4 */ {{g3, f2, e1}, {g5, f6, e7, d8}},
    /* a3 */ {{b2, c1}, {b4, c5, d6, e7, f8}},
    /* b3 */ {{c2, d1}, {a2}, {a4}, {c4, d5, e6, f7, g8}},
    /* c3 */ {{d2, e1}, {b2, a1}, {b4, a5}, {d4, e5, f6, g7, h8}},
    /* d3 */ {{e2, f1}, {c2, b1}, {c4, b5, a6}, {e4, f5, g6, h7}},
    /* e3 */ {{f2, g1}, {d2, c1}, {d4, c5, b6, a7}, {f4, g5, h6}},
    /* f3 */ {{g2, h1}, {e2, d1}, {e4, d5, c6, b7, a8}, {g4, h5}},
    /* g3 */ {{h2}, {f2, e1}, {f4, e5, d6, c7, b8}, {h4}},
    /* h3 */ {{g2, f1}, {g4, f5, e6, d7, c8}},
    /* a2 */ {{b1}, {b3, c4, d5, e6, f7, g8}},
    /* b2 */ {{c1}, {a1}, {a3}, {c3, d4, e5, f6, g7, h8}},
    /* c2 */ {{d1}, {b1}, {b3, a4}, {d3, e4, f5, g6, h7}},
    /* d2 */ {{e1}, {c1}, {c3, b4, a5}, {e3, f4, g5, h6}},
    /* e2 */ {{f1}, {d1}, {d3, c4, b5, a6}, {f3, g4, h5}},
    /* f2 */ {{g1}, {e1}, {e3, d4, c5, b6, a7}, {g3, h4}},
    /* g2 */ {{h1}, {f1}, {f3, e4, d5, c6, b7, a8}, {h3}},
    /* h2 */ {{g1}, {g3, f4, e5, d6, c7, b8}},
    /* a1 */ {{b2, c3, d4, e5, f6, g7, h8}},
    /* b1 */ {{a2}, {c2, d3, e4, f5, g6, h7}},
    /* c1 */ {{b2, a3}, {d2, e3, f4, g5, h6}},
    /* d1 */ {{c2, b3, a4}, {e2, f3, g4, h5}},
    /* e1 */ {{d2, c3, b4, a5}, {f2, g3, h4}},
    /* f1 */ {{e2, d3, c4, b5, a6}, {g2, h3}},
    /* g1 */ {{f2, e3, d4, c5, b6, a7}, {h2}},
    /* h1 */ {{g2, f3, e4, d5, c6, b7, a8}}
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER SourceToDirectionsMap<18> ROOK_DESTINATIONS{
    /* a8 */ {{b8, c8, d8, e8, f8, g8, h8}, {a7, a6, a5, a4, a3, a2, a1}},
    /* b8 */ {{c8, d8, e8, f8, g8, h8}, {b7, b6, b5, b4, b3, b2, b1}, {a8}},
    /* c8 */ {{d8, e8, f8, g8, h8}, {c7, c6, c5, c4, c3, c2, c1}, {b8, a8}},
    /* d8 */ {{e8, f8, g8, h8}, {d7, d6, d5, d4, d3, d2, d1}, {c8, b8, a8}},
    /* e8 */ {{f8, g8, h8}, {e7, e6, e5, e4, e3, e2, e1}, {d8, c8, b8, a8}},
    /* f8 */ {{g8, h8}, {f7, f6, f5, f4, f3, f2, f1}, {e8, d8, c8, b8, a8}},
    /* g8 */ {{h8}, {g7, g6, g5, g4, g3, g2, g1}, {f8, e8, d8, c8, b8, a8}},
    /* h8 */ {{h7, h6, h5, h4, h3, h2, h1}, {g8, f8, e8, d8, c8, b8, a8}},
    /* a7 */ {{b7, c7, d7, e7, f7, g7, h7}, {a6, a5, a4, a3, a2, a1}, {a8}},
    /* b7 */ {{c7, d7, e7, f7, g7, h7}, {b6, b5, b4, b3, b2, b1}, {a7}, {b8}},
    /* c7 */ {{d7, e7, f7, g7, h7}, {c6, c5, c4, c3, c2, c1}, {b7, a7}, {c8}},
    /* d7 */ {{e7, f7, g7, h7}, {d6, d5, d4, d3, d2, d1}, {c7, b7, a7}, {d8}},
    /* e7 */ {{f7, g7, h7}, {e6, e5, e4, e3, e2, e1}, {d7, c7, b7, a7}, {e8}},
    /* f7 */ {{g7, h7}, {f6, f5, f4, f3, f2, f1}, {e7, d7, c7, b7, a7}, {f8}},
    /* g7 */ {{h7}, {g6, g5, g4, g3, g2, g1}, {f7, e7, d7, c7, b7, a7}, {g8}},
    /* h7 */ {{h6, h5, h4, h3, h2, h1}, {g7, f7, e7, d7, c7, b7, a7}, {h8}},
    /* a6 */ {{b6, c6, d6, e6, f6, g6, h6}, {a5, a4, a3, a2, a1}, {a7, a8}},
    /* b6 */ {{c6, d6, e6, f6, g6, h6}, {b5, b4, b3, b2, b1}, {a6}, {b7, b8}},
    /* c6 */ {{d6, e6, f6, g6, h6}, {c5, c4, c3, c2, c1}, {b6, a6}, {c7, c8}},
    /* d6 */ {{e6, f6, g6, h6}, {d5, d4, d3, d2, d1}, {c6, b6, a6}, {d7, d8}},
    /* e6 */ {{f6, g6, h6}, {e5, e4, e3, e2, e1}, {d6, c6, b6, a6}, {e7, e8}},
    /* f6 */ {{g6, h6}, {f5, f4, f3, f2, f1}, {e6, d6, c6, b6, a6}, {f7, f8}},
    /* g6 */ {{h6}, {g5, g4, g3, g2, g1}, {f6, e6, d6, c6, b6, a6}, {g7, g8}},
    /* h6 */ {{h5, h4, h3, h2, h1}, {g6, f6, e6, d6, c6, b6, a6}, {h7, h8}},
    /* a5 */ {{b5, c5, d5, e5, f5, g5, h5}, {a4, a3, a2, a1}, {a6, a7, a8}},
    /* b5 */ {{c5, d5, e5, f5, g5, h5}, {b4, b3, b2, b1}, {a5}, {b6, b7, b8}},
    /* c5 */ {{d5, e5, f5, g5, h5}, {c4, c3, c2, c1}, {b5, a5}, {c6, c7, c8}},
    /* d5 */ {{e5, f5, g5, h5}, {d4, d3, d2, d1}, {c5, b5, a5}, {d6, d7, d8}},
    /* e5 */ {{f5, g5, h5}, {e4, e3, e2, e1}, {d5, c5, b5, a5}, {e6, e7, e8}},
    /* f5 */ {{g5, h5}, {f4, f3, f2, f1}, {e5, d5, c5, b5, a5}, {f6, f7, f8}},
    /* g5 */ {{h5}, {g4, g3, g2, g1}, {f5, e5, d5, c5, b5, a5}, {g6, g7, g8}},
    /* h5 */ {{h4, h3, h2, h1}, {g5, f5, e5, d5, c5, b5, a5}, {h6, h7, h8}},
    /* a4 */ {{b4, c4, d4, e4, f4, g4, h4}, {a3, a2, a1}, {a5, a6, a7, a8}},
    /* b4 */ {{c4, d4, e4, f4, g4, h4}, {b3, b2, b1}, {a4}, {b5, b6, b7, b8}},
    /* c4 */ {{d4, e4, f4, g4, h4}, {c3, c2, c1}, {b4, a4}, {c5, c6, c7, c8}},
    /* d4 */ {{e4, f4, g4, h4}, {d3, d2, d1}, {c4, b4, a4}, {d5, d6, d7, d8}},
    /* e4 */ {{f4, g4, h4}, {e3, e2, e1}, {d4, c4, b4, a4}, {e5, e6, e7, e8}},
    /* f4 */ {{g4, h4}, {f3, f2, f1}, {e4, d4, c4, b4, a4}, {f5, f6, f7, f8}},
    /* g4 */ {{h4}, {g3, g2, g1}, {f4, e4, d4, c4, b4, a4}, {g5, g6, g7, g8}},
    /* h4 */ {{h3, h2, h1}, {g4, f4, e4, d4, c4, b4, a4}, {h5, h6, h7, h8}},
    /* a3 */ {{b3, c3, d3, e3, f3, g3, h3}, {a2, a1}, {a4, a5, a6, a7, a8}},
    /* b3 */ {{c3, d3, e3, f3, g3, h3}, {b2, b1}, {a3}, {b4, b5, b6, b7, b8}},
    /* c3 */ {{d3, e3, f3, g3, h3}, {c2, c1}, {b3, a3}, {c4, c5, c6, c7, c8}},
    /* d3 */ {{e3, f3, g3, h3}, {d2, d1}, {c3, b3, a3}, {d4, d5, d6, d7, d8}},
    /* e3 */ {{f3, g3, h3}, {e2, e1}, {d3, c3, b3, a3}, {e4, e5, e6, e7, e8}},
    /* f3 */ {{g3, h3}, {f2, f1}, {e3, d3, c3, b3, a3}, {f4, f5, f6, f7, f8}},
    /* g3 */ {{h3}, {g2, g1}, {f3, e3, d3, c3, b3, a3}, {g4, g5, g6, g7, g8}},
    /* h3 */ {{h2, h1}, {g3, f3, e3, d3, c3, b3, a3}, {h4, h5, h6, h7, h8}},
    /* a2 */ {{b2, c2, d2, e2, f2, g2, h2}, {a1}, {a3, a4, a5, a6, a7, a8}},
    /* b2 */ {{c2, d2, e2, f2, g2, h2}, {b1}, {a2}, {b3, b4, b5, b6, b7, b8}},
    /* c2 */ {{d2, e2, f2, g2, h2}, {c1}, {b2, a2}, {c3, c4, c5, c6, c7, c8}},
    /* d2 */ {{e2, f2, g2, h2}, {d1}, {c2, b2, a2}, {d3, d4, d5, d6, d7, d8}},
    /* e2 */ {{f2, g2, h2}, {e1}, {d2, c2, b2, a2}, {e3, e4, e5, e6, e7, e8}},
    /* f2 */ {{g2, h2}, {f1}, {e2, d2, c2, b2, a2}, {f3, f4, f5, f6, f7, f8}},
    /* g2 */ {{h2}, {g1}, {f2, e2, d2, c2, b2, a2}, {g3, g4, g5, g6, g7, g8}},
    /* h2 */ {{h1}, {g2, f2, e2, d2, c2, b2, a2}, {h3, h4, h5, h6, h7, h8}},
    /* a1 */ {{b1, c1, d1, e1, f1, g1, h1}, {a2, a3, a4, a5, a6, a7, a8}},
    /* b1 */ {{c1, d1, e1, f1, g1, h1}, {a1}, {b2, b3, b4, b5, b6, b7, b8}},
    /* c1 */ {{d1, e1, f1, g1, h1}, {b1, a1}, {c2, c3, c4, c5, c6, c7, c8}},
    /* d1 */ {{e1, f1, g1, h1}, {c1, b1, a1}, {d2, d3, d4, d5, d6, d7, d8}},
    /* e1 */ {{f1, g1, h1}, {d1, c1, b1, a1}, {e2, e3, e4, e5, e6, e7, e8}},
    /* f1 */ {{g1, h1}, {e1, d1, c1, b1, a1}, {f2, f3, f4, f5, f6, f7, f8}},
    /* g1 */ {{h1}, {f1, e1, d1, c1, b1, a1}, {g2, g3, g4, g5, g6, g7, g8}},
    /* h1 */ {{g1, f1, e1, d1, c1, b1, a1}, {h2, h3, h4, h5, h6, h7, h8}}
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER SourceToDestinationsMap<MAX_DESTINATIONS> KNIGHT_DESTINATIONS{
    /* a8 */ {c7, b6},
    /* b8 */ {d7, c6, a6},
    /* c8 */ {e7, d6, b6, a7},
    /* d8 */ {f7, e6, c6, b7},
    /* e8 */ {g7, f6, d6, c7},
    /* f8 */ {h7, g6, e6, d7},
    /* g8 */ {h6, f6, e7},
    /* h8 */ {g6, f7},
    /* a7 */ {c8, c6, b5},
    /* b7 */ {d8, d6, c5, a5},
    /* c7 */ {e8, e6, d5, b5, a6, a8},
    /* d7 */ {f8, f6, e5, c5, b6, b8},
    /* e7 */ {g8, g6, f5, d5, c6, c8},
    /* f7 */ {h8, h6, g5, e5, d6, d8},
    /* g7 */ {h5, f5, e6, e8},
    /* h7 */ {g5, f6, f8},
    /* a6 */ {b8, c7, c5, b4},
    /* b6 */ {c8, d7, d5, c4, a4, a8},
    /* c6 */ {d8, e7, e5, d4, b4, a5, a7, b8},
    /* d6 */ {e8, f7, f5, e4, c4, b5, b7, c8},
    /* e6 */ {f8, g7, g5, f4, d4, c5, c7, d8},
    /* f6 */ {g8, h7, h5, g4, e4, d5, d7, e8},
    /* g6 */ {h8, h4, f4, e5, e7, f8},
    /* h6 */ {g4, f5, f7, g8},
    /* a5 */ {b7, c6, c4, b3},
    /* b5 */ {c7, d6, d4, c3, a3, a7},
    /* c5 */ {d7, e6, e4, d3, b3, a4, a6, b7},
    /* d5 */ {e7, f6, f4, e3, c3, b4, b6, c7},
    /* e5 */ {f7, g6, g4, f3, d3, c4, c6, d7},
    /* f5 */ {g7, h6, h4, g3, e3, d4, d6, e7},
    /* g5 */ {h7, h3, f3, e4, e6, f7},
    /* h5 */ {g3, f4, f6, g7},
    /* a4 */ {b6, c5, c3, b2},
    /* b4 */ {c6, d5, d3, c2, a2, a6},
    /* c4 */ {d6, e5, e3, d2, b2, a3, a5, b6},
    /* d4 */ {e6, f5, f3, e2, c2, b3, b5, c6},
    /* e4 */ {f6, g5, g3, f2, d2, c3, c5, d6},
    /* f4 */ {g6, h5, h3, g2, e2, d3, d5, e6},
    /* g4 */ {h6, h2, f2, e3, e5, f6},
    /* h4 */ {g2, f3, f5, g6},
    /* a3 */ {b5, c4, c2, b1},
    /* b3 */ {c5, d4, d2, c1, a1, a5},
    /* c3 */ {d5, e4, e2, d1, b1, a2, a4, b5},
    /* d3 */ {e5, f4, f2, e1, c1, b2, b4, c5},
    /* e3 */ {f5, g4, g2, f1, d1, c2, c4, d5},
    /* f3 */ {g5, h4, h2, g1, e1, d2, d4, e5},
    /* g3 */ {h5, h1, f1, e2, e4, f5},
    /* h3 */ {g1, f2, f4, g5},
    /* a2 */ {b4, c3, c1},
    /* b2 */ {c4, d3, d1, a4},
    /* c2 */ {d4, e3, e1, a1, a3, b4},
    /* d2 */ {e4, f3, f1, b1, b3, c4},
    /* e2 */ {f4, g3, g1, c1, c3, d4},
    /* f2 */ {g4, h3, h1, d1, d3, e4},
    /* g2 */ {h4, e1, e3, f4},
    /* h2 */ {f1, f3, g4},
    /* a1 */ {b3, c2},
    /* b1 */ {a3, c3, d2},
    /* c1 */ {d3, e2, a2, b3},
    /* d1 */ {e3, f2, b2, c3},
    /* e1 */ {f3, g2, c2, d3},
    /* f1 */ {g3, h2, d2, e3},
    /* g1 */ {h3, e2, f3},
    /* h1 */ {f2, g3}
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER SourceToDestinationsMap<MAX_DESTINATIONS> KING_DESTINATIONS{
    /* a8 */ {b8, b7, a7},
    /* b8 */ {c8, c7, b7, a7, a8},
    /* c8 */ {d8, d7, c7, b7, b8},
    /* d8 */ {e8, e7, d7, c7, c8},
    /* e8 */ {f8, f7, e7, d7, d8},
    /* f8 */ {g8, g7, f7, e7, e8},
    /* g8 */ {h8, h7, g7, f7, f8},
    /* h8 */ {h7, g7, g8},
    /* a7 */ {b8, b7, b6, a6, a8},
    /* b7 */ {c8, c7, c6, b6, a6, a7, a8, b8},
    /* c7 */ {d8, d7, d6, c6, b6, b7, b8, c8},
    /* d7 */ {e8, e7, e6, d6, c6, c7, c8, d8},
    /* e7 */ {f8, f7, f6, e6, d6, d7, d8, e8},
    /* f7 */ {g8, g7, g6, f6, e6, e7, e8, f8},
    /* g7 */ {h8, h7, h6, g6, f6, f7, f8, g8},
    /* h7 */ {h6, g6, g7, g8, h8},
    /* a6 */ {b7, b6, b5, a5, a7},
    /* b6 */ {c7, c6, c5, b5, a5, a6, a7, b7},
    /* c6 */ {d7, d6, d5, c5, b5, b6, b7, c7},
    /* d6 */ {e7, e6, e5, d5, c5, c6, c7, d7},
    /* e6 */ {f7, f6, f5, e5, d5, d6, d7, e7},
    /* f6 */ {g7, g6, g5, f5, e5, e6, e7, f7},
    /* g6 */ {h7, h6, h5, g5, f5, f6, f7, g7},
    /* h6 */ {h5, g5, g6, g7, h7},
    /* a5 */ {b6, b5, b4, a4, a6},
    /* b5 */ {c6, c5, c4, b4, a4, a5, a6, b6},
    /* c5 */ {d6, d5, d4, c4, b4, b5, b6, c6},
    /* d5 */ {e6, e5, e4, d4, c4, c5, c6, d6},
    /* e5 */ {f6, f5, f4, e4, d4, d5, d6, e6},
    /* f5 */ {g6, g5, g4, f4, e4, e5, e6, f6},
    /* g5 */ {h6, h5, h4, g4, f4, f5, f6, g6},
    /* h5 */ {h4, g4, g5, g6, h6},
    /* a4 */ {b5, b4, b3, a3, a5},
    /* b4 */ {c5, c4, c3, b3, a3, a4, a5, b5},
    /* c4 */ {d5, d4, d3, c3, b3, b4, b5, c5},
    /* d4 */ {e5, e4, e3, d3, c3, c4, c5, d5},
    /* e4 */ {f5, f4, f3, e3, d3, d4, d5, e5},
    /* f4 */ {g5, g4, g3, f3, e3, e4, e5, f5},
    /* g4 */ {h5, h4, h3, g3, f3, f4, f5, g5},
    /* h4 */ {h3, g3, g4, g5, h5},
    /* a3 */ {b4, b3, b2, a2, a4},
    /* b3 */ {c4, c3, c2, b2, a2, a3, a4, b4},
    /* c3 */ {d4, d3, d2, c2, b2, b3, b4, c4},
    /* d3 */ {e4, e3, e2, d2, c2, c3, c4, d4},
    /* e3 */ {f4, f3, f2, e2, d2, d3, d4, e4},
    /* f3 */ {g4, g3, g2, f2, e2, e3, e4, f4},
    /* g3 */ {h4, h3, h2, g2, f2, f3, f4, g4},
    /* h3 */ {h2, g2, g3, g4, h4},
    /* a2 */ {b3, b2, b1, a1, a3},
    /* b2 */ {c3, c2, c1, b1, a1, a2, a3, b3},
    /* c2 */ {d3, d2, d1, c1, b1, b2, b3, c3},
    /* d2 */ {e3, e2, e1, d1, c1, c2, c3, d3},
    /* e2 */ {f3, f2, f1, e1, d1, d2, d3, e3},
    /* f2 */ {g3, g2, g1, f1, e1, e2, e3, f3},
    /* g2 */ {h3, h2, h1, g1, f1, f2, f3, g3},
    /* h2 */ {h1, g1, g2, g3, h3},
    /* a1 */ {b2, b1, a2},
    /* b1 */ {c2, c1, a1, a2, b2},
    /* c1 */ {d2, d1, b1, b2, c2},
    /* d1 */ {e2, e1, c1, c2, d2},
    /* e1 */ {f2, f1, d1, d2, e2},
    /* f1 */ {g2, g1, e1, e2, f2},
    /* g1 */ {h2, h1, f1, f2, g2},
    /* h1 */ {g1, g2, h2}
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER SourceToDestinationsMap<2> WHITE_PAWN_MOVE_DESTINATIONS{
    /* a8 */ {},
    /* b8 */ {},
    /* c8 */ {},
    /* d8 */ {},
    /* e8 */ {},
    /* f8 */ {},
    /* g8 */ {},
    /* h8 */ {},
    /* a7 */ {a8},
    /* b7 */ {b8},
    /* c7 */ {c8},
    /* d7 */ {d8},
    /* e7 */ {e8},
    /* f7 */ {f8},
    /* g7 */ {g8},
    /* h7 */ {h8},
    /* a6 */ {a7},
    /* b6 */ {b7},
    /* c6 */ {c7},
    /* d6 */ {d7},
    /* e6 */ {e7},
    /* f6 */ {f7},
    /* g6 */ {g7},
    /* h6 */ {h7},
    /* a5 */ {a6},
    /* b5 */ {b6},
    /* c5 */ {c6},
    /* d5 */ {d6},
    /* e5 */ {e6},
    /* f5 */ {f6},
    /* g5 */ {g6},
    /* h5 */ {h6},
    /* a4 */ {a5},
    /* b4 */ {b5},
    /* c4 */ {c5},
    /* d4 */ {d5},
    /* e4 */ {e5},
    /* f4 */ {f5},
    /* g4 */ {g5},
    /* h4 */ {h5},
    /* a3 */ {a4},
    /* b3 */ {b4},
    /* c3 */ {c4},
    /* d3 */ {d4},
    /* e3 */ {e4},
    /* f3 */ {f4},
    /* g3 */ {g4},
    /* h3 */ {h4},
    /* a2 */ {a3, a4},
    /* b2 */ {b3, b4},
    /* c2 */ {c3, c4},
    /* d2 */ {d3, d4},
    /* e2 */ {e3, e4},
    /* f2 */ {f3, f4},
    /* g2 */ {g3, g4},
    /* h2 */ {h3, h4},
    /* a1 */ {a2},
    /* b1 */ {b2},
    /* c1 */ {c2},
    /* d1 */ {d2},
    /* e1 */ {e2},
    /* f1 */ {f2},
    /* g1 */ {g2},
    /* h1 */ {h2}
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER SourceToDestinationsMap<2> WHITE_PAWN_BEAT_DESTINATIONS{
    /* a8 */ {},
    /* b8 */ {},
    /* c8 */ {},
    /* d8 */ {},
    /* e8 */ {},
    /* f8 */ {},
    /* g8 */ {},
    /* h8 */ {},
    /* a7 */ {b8},
    /* b7 */ {a8, c8},
    /* c7 */ {b8, d8},
    /* d7 */ {c8, e8},
    /* e7 */ {d8, f8},
    /* f7 */ {e8, g8},
    /* g7 */ {f8, h8},
    /* h7 */ {g8},
    /* a6 */ {b7},
    /* b6 */ {a7, c7},
    /* c6 */ {b7, d7},
    /* d6 */ {c7, e7},
    /* e6 */ {d7, f7},
    /* f6 */ {e7, g7},
    /* g6 */ {f7, h7},
    /* h6 */ {g7},
    /* a5 */ {b6},
    /* b5 */ {a6, c6},
    /* c5 */ {b6, d6},
    /* d5 */ {c6, e6},
    /* e5 */ {d6, f6},
    /* f5 */ {e6, g6},
    /* g5 */ {f6, h6},
    /* h5 */ {g6},
    /* a4 */ {b5},
    /* b4 */ {a5, c5},
    /* c4 */ {b5, d5},
    /* d4 */ {c5, e5},
    /* e4 */ {d5, f5},
    /* f4 */ {e5, g5},
    /* g4 */ {f5, h5},
    /* h4 */ {g5},
    /* a3 */ {b4},
    /* b3 */ {a4, c4},
    /* c3 */ {b4, d4},
    /* d3 */ {c4, e4},
    /* e3 */ {d4, f4},
    /* f3 */ {e4, g4},
    /* g3 */ {f4, h4},
    /* h3 */ {g4},
    /* a2 */ {b3},
    /* b2 */ {a3, c3},
    /* c2 */ {b3, d3},
    /* d2 */ {c3, e3},
    /* e2 */ {d3, f3},
    /* f2 */ {e3, g3},
    /* g2 */ {f3, h3},
    /* h2 */ {g3},
    /* a1 */ {b2},
    /* b1 */ {a2, c2},
    /* c1 */ {b2, d2},
    /* d1 */ {c2, e2},
    /* e1 */ {d2, f2},
    /* f1 */ {e2, g2},
    /* g1 */ {f2, h2},
    /* h1 */ {g2}
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER SourceToDestinationsMap<2> BLACK_PAWN_MOVE_DESTINATIONS{
    /* a8 */ {a7},
    /* b8 */ {b7},
    /* c8 */ {c7},
    /* d8 */ {d7},
    /* e8 */ {e7},
    /* f8 */ {f7},
    /* g8 */ {g7},
    /* h8 */ {h7},
    /* a7 */ {a6, a5},
    /* b7 */ {b6, b5},
    /* c7 */ {c6, c5},
    /* d7 */ {d6, d5},
    /* e7 */ {e6, e5},
    /* f7 */ {f6, f5},
    /* g7 */ {g6, g5},
    /* h7 */ {h6, h5},
    /* a6 */ {a5},
    /* b6 */ {b5},
    /* c6 */ {c5},
    /* d6 */ {d5},
    /* e6 */ {e5},
    /* f6 */ {f5},
    /* g6 */ {g5},
    /* h6 */ {h5},
    /* a5 */ {a4},
    /* b5 */ {b4},
    /* c5 */ {c4},
    /* d5 */ {d4},
    /* e5 */ {e4},
    /* f5 */ {f4},
    /* g5 */ {g4},
    /* h5 */ {h4},
    /* a4 */ {a3},
    /* b4 */ {b3},
    /* c4 */ {c3},
    /* d4 */ {d3},
    /* e4 */ {e3},
    /* f4 */ {f3},
    /* g4 */ {g3},
    /* h4 */ {h3},
    /* a3 */ {a2},
    /* b3 */ {b2},
    /* c3 */ {c2},
    /* d3 */ {d2},
    /* e3 */ {e2},
    /* f3 */ {f2},
    /* g3 */ {g2},
    /* h3 */ {h2},
    /* a2 */ {a1},
    /* b2 */ {b1},
    /* c2 */ {c1},
    /* d2 */ {d1},
    /* e2 */ {e1},
    /* f2 */ {f1},
    /* g2 */ {g1},
    /* h2 */ {h1},
    /* a1 */ {},
    /* b1 */ {},
    /* c1 */ {},
    /* d1 */ {},
    /* e1 */ {},
    /* f1 */ {},
    /* g1 */ {},
    /* h1 */ {}
};
// clang-format on

// clang-format off
CHESS_ENGINE_MAP_QUALIFIER SourceToDestinationsMap<2> BLACK_PAWN_BEAT_DESTINATIONS{
    /* a8 */ {b7},
    /* b8 */ {a7, c7},
    /* c8 */ {b7, d7},
    /* d8 */ {c7, e7},
    /* e8 */ {d7, f7},
    /* f8 */ {e7, g7},
    /* g8 */ {f7, h7},
    /* h8 */ {g7},
    /* a7 */ {b6},
    /* b7 */ {a6, c6},
    /* c7 */ {b6, d6},
    /* d7 */ {c6, e6},
    /* e7 */ {d6, f6},
    /* f7 */ {e6, g6},
    /* g7 */ {f6, h6},
    /* h7 */ {g6},
    /* a6 */ {b5},
    /* b6 */ {a5, c5},
    /* c6 */ {b5, d5},
    /* d6 */ {c5, e5},
    /* e6 */ {d5, f5},
    /* f6 */ {e5, g5},
    /* g6 */ {f5, h5},
    /* h6 */ {g5},
    /* a5 */ {b4},
    /* b5 */ {a4, c4},
    /* c5 */ {b4, d4},
    /* d5 */ {c4, e4},
    /* e5 */ {d4, f4},
    /* f5 */ {e4, g4},
    /* g5 */ {f4, h4},
    /* h5 */ {g4},
    /* a4 */ {b3},
    /* b4 */ {a3, c3},
    /* c4 */ {b3, d3},
    /* d4 */ {c3, e3},
    /* e4 */ {d3, f3},
    /* f4 */ {e3, g3},
    /* g4 */ {f3, h3},
    /* h4 */ {g3},
    /* a3 */ {b2},
    /* b3 */ {a2, c2},
    /* c3 */ {b2, d2},
    /* d3 */ {c2, e2},
    /* e3 */ {d2, f2},
    /* f3 */ {e2, g2},
    /* g3 */ {f2, h2},
    /* h3 */ {g2},
    /* a2 */ {b1},
    /* b2 */ {a1, c1},
    /* c2 */ {b1, d1},
    /* d2 */ {c1, e1},
    /* e2 */ {d1, f1},
    /* f2 */ {e1, g1},
    /* g2 */ {f1, h1},
    /* h2 */ {g1},
    /* a1 */ {},
    /* b1 */ {},
    /* c1 */ {},
    /* d1 */ {},
    /* e1 */ {},
    /* f1 */ {},
    /* g1 */ {},
    /* h1 */ {}
};
// clang-format on

bool isNumberChar(char ch) {
    return ch >= '0' && ch <= '9';
}

uint8_t convertCharToNumber(char ch) {
    return ch - '0';
}

uint16_t convertCharToMultiNumber(char ch, uint16_t number) {
    const uint16_t value = convertCharToNumber(ch);
    return number ? number * 10 + value : value;
}

String convertMultiNumberToStr(uint16_t number) {
    return String(number);
}

bool isMateWeight(Weight weight) {
    return weight > MATE_SOLVED_WEIGHT;
}

bool isHalfMove(const Move& move) {
    return move.isBeat() || move.pieces.src.getKind() == PieceKind::Pawn;
}

Move selectRandomMove(const Moves& moves) {
    return moves.empty() ? Move::none() : moves[millis() % moves.size()];
}
}

Engine::Engine() :
#ifdef CHESS_ENGINE_MULTI_TASK
    task(*this),
#endif
    search(),
    rootRound(search[ROOT_DEPTH]),
    rootNextRound(search[ROOT_NEXT_DEPTH]),
    board(),
    kingPositions(),
    moves(),
    bestMoves(),
    modes(),
    options(),
    maxReachedDepth(),
    supervisor(options),
    history(),
    oldestRound(),
    oldestBoard(),
    lastMove() {
}

bool Engine::begin(const String& fen) {
    if (!applyFen(fen)) {
        rootRound.status = RoundStatus::Invalid;
        CHESS_ENGINE_LOG(TAG, "Invalid fen");
        return false;
    }
    history.clear();
    updateRound();
    lastMove = Move::none();
    oldestRound = rootRound;
    oldestBoard = board;
    return true;
}

bool Engine::undo(size_t num) {
    if (history.size() < num) {
        return false;
    }
    rootRound = oldestRound;
    board = oldestBoard;
    history.resize(history.size() - num);
    for (const auto& move : history) {
        applyMove(move);
    }
    updateRound();
    lastMove = history.empty() ? Move::none() : history.back();
    return true;
}

Move Engine::find(const Move& move) const {
    auto type = move.type;
    bool isPromotion = move.isPromotion();

    if (!isPromotion && isAutoPromotionNeeded(move)) {
        type = MoveType::PromoQueen;
        isPromotion = true;
        CHESS_ENGINE_LOG(TAG, "Auto promotion to queen");
    }

    for (const Move& toCheck : rootRound.moves) {
        if (toCheck.positions != move.positions) {
            continue;
        }
        if (isPromotion && toCheck.type != type) {
            continue;
        }
        return toCheck;
    }
    return Move::none();
}

Move Engine::find(const String& uci) const {
    const Move& move = convertUciToMove(uci);
    if (move.isNone()) {
        return move;
    }
    return find(move);
}

Move Engine::make(const Move& move) {
    Move found = find(move);
    if (found.isNone()) {
        return found;
    }
    history.push(found);
    applyMove(found);
    updateRound();
    lastMove = found;
    return found;
}

Move Engine::make(const String& uci) {
    const Move& move = convertUciToMove(uci);
    if (move.isNone()) {
        return move;
    }
    return make(move);
}

Move Engine::solve() {
    if (options.random) {
        return selectRandomMove(rootRound.moves);
    }

    solveMoves();

    const auto best = selectRandomMove(bestMoves);

#ifdef CHESS_ENGINE_LOGS
    CHESS_ENGINE_LOG_BEGIN(TAG, "Solved: weight: %d moves: ", best.weight);
    for (const auto& move : bestMoves) {
        CHESS_ENGINE_LOG_ADD(TAG, "%s ", convertMoveToUci(move).c_str());
    }
    CHESS_ENGINE_LOG_END(TAG, "\n");
#endif

    return best;
}

void Engine::stop() {
    supervisor.stop();
}

PieceColor Engine::getTurn() const {
    return rootRound.turn;
}

CastlingAbilities Engine::getCastlingAbilities() const {
    return rootRound.castling;
}
Position Engine::getEnPassant() const {
    return rootRound.enPassant;
}

HalfMove Engine::getHalfMove() const {
    return rootRound.halfMove;
}

FullMove Engine::getFullMove() const {
    return rootRound.fullMove;
}

RoundStatus Engine::getRoundStatus() const {
    return rootRound.status;
}

Position Engine::getCheck() const {
    return kingPositions.check;
}

Move Engine::getLastMove() const {
    return lastMove;
}

const Board& Engine::getBoard() const {
    return board;
}

bool Engine::isWhiteTurn() const {
    return rootRound.isWhiteTurn();
}

bool Engine::isBlackTurn() const {
    return rootRound.isBlackTurn();
}

bool Engine::isNormal() const {
    return rootRound.isNormal();
}

bool Engine::isCheck() const {
    return rootRound.isCheck();
}

bool Engine::isCheckmate() const {
    return rootRound.isCheckmate();
}

bool Engine::isStalemate() const {
    return rootRound.isStalemate();
}

bool Engine::isDeadPosition() const {
    return rootRound.isDeadPosition();
}

bool Engine::isFiftyMove() const {
    return rootRound.isFiftyMove();
}

bool Engine::isThreefoldRepetition() const {
    return rootRound.isThreefoldRepetition();
}

bool Engine::isStopped() const {
    return rootRound.isStopped();
}

bool Engine::isInvalid() const {
    return rootRound.isInvalid();
}

bool Engine::isDraw() const {
    return rootRound.isDraw();
}

bool Engine::isEnd() const {
    return rootRound.isEnd();
}

const Options& Engine::getOptions() const {
    return options;
}

void Engine::setOptions(const Options& opt) {
    options = opt;
}

Milliseconds Engine::getTimeLimit() const {
    return options.timeLimit;
}

void Engine::setTimeLimit(Milliseconds limit) {
    options.timeLimit = limit;
}

void Engine::setCyclicPull(CyclicPull& pull, Milliseconds period) {
    supervisor.setCyclicPull(pull, period);
}

#ifdef CHESS_ENGINE_MULTI_TASK
Task& Engine::setMultiTask() {
    task.isActive = true;
    return task;
}
#endif

const Moves& Engine::solveMoves() {
    auto best = Move::none();
    bestMoves.clear();

    if (rootRound.isEnd()) {
        return bestMoves;
    }

    auto& rootBest = rootRound.best;
    auto& rootMoves = rootRound.moves;
    Weight alpha = MIN_WEIGHT;
    Weight beta = MAX_WEIGHT;
    unsigned sameBestCount = 0;

    supervisor.begin();
    modes = Modes();

    for (Depth depthTarget = NEGAMAX_MIN_DEPTH; depthTarget <= options.maxDepth; depthTarget++) {
        maxReachedDepth = depthTarget;

        for (Move& move : rootMoves) {
            redoBoard(rootRound, move);
            move.isCheck = isOpponentKingInCheck(rootRound);
            move.weight += evaluate(rootRound);
            if (move.isCheck) {
                move.weight += CHECK_EXTRA_MOVE_WEIGHT;
            }
            if (move.isBeat()) {
                move.weight -= move.pieces.src.getKindWeight();
            }
            undoBoard(rootRound, move);
        }

        // PV-Move: https://chessprogramming.org/PV-Move
        rootMoves.front().weight += ROOT_FIRST_EXTRA_MOVE_WEIGHT;

        sortMoves(rootMoves);
        for (Move& move : rootMoves) {
            move.weight = UNSCORED_MOVE_WEIGHT;
        }

        const Weight weight = negamax(ROOT_DEPTH, alpha, beta, depthTarget);
        if (supervisor.isStopped()) {
            break;
        }

        supervisor.updateTime();
        modes.boardEval = BoardEvalMode::Positional;

        // Fail High: https://chessprogramming.org/Fail-High
        const bool isFailHigh = weight >= beta;
        // Aspiration Windows: https://chessprogramming.org/Aspiration_Windows
        if (!options.aspiration || sameBestCount > 2 || isFailHigh) {
            sameBestCount = 0;
            alpha = MIN_WEIGHT;
            beta = MAX_WEIGHT;
        } else if (supervisor.getTime() <= options.timeLimit / 5) {
            alpha = weight - ASPIRATION_WEIGHT_MARGIN;
            beta = weight + ASPIRATION_WEIGHT_MARGIN;
        } else {
            modes.boardEval = BoardEvalMode::Material;
            alpha = weight - WIDE_ASPIRATION_WEIGHT_MARGIN;
            beta = weight + WIDE_ASPIRATION_WEIGHT_MARGIN;
        }

        if (rootBest.isSame(best)) {
            sameBestCount++;
        } else {
            sameBestCount = 0;
        }
        best = rootBest;
        if (!moves.empty()) {
            bestMoves = moves;
        }

        sortMoves(rootMoves);

        CHESS_ENGINE_LOG(TAG, "Solved: move: %s, time: %s, depth: %d/%d, weight: %d, nodes: %lu",
                         convertMoveToUci(best).c_str(), convertMillisecondsToStr(supervisor.getTime()).c_str(),
                         depthTarget, maxReachedDepth, best.weight, supervisor.getNodesCount());

        if (isMateWeight(weight)) {
            break;
        }
        if (supervisor.checkTimeLimit()) {
            break;
        }
    }

    if (best.isNone()) {
        rootRound.status = RoundStatus::Stopped;
    }
    return bestMoves;
}

const Moves& Engine::createMoves(Position src) {
    moves.clear();

    for (const Move& move : rootRound.moves) {
        if (move.positions.src == src) {
            moves.push(move);
        } else if (move.isCastling()) {
            if (rootRound.isWhiteTurn()) {
                const auto rook =
                    move.type == MoveType::CastleKing ? WHITE_KING_CASTLING_MOVE.rook : WHITE_QUEEN_CASTLING_MOVE.rook;
                if (rook.src == src) {
                    moves.push(move);
                }
            } else {
                const auto rook =
                    move.type == MoveType::CastleKing ? BLACK_KING_CASTLING_MOVE.rook : BLACK_QUEEN_CASTLING_MOVE.rook;
                if (rook.src == src) {
                    moves.push(move);
                }
            }
        } else if (move.isEnPassant()) {
            if (rootRound.isWhiteTurn()) {
                if (WHITE_PAWN_DST_TO_EN_PASSANT_REMOVES[move.positions.dst].beat == src) {
                    moves.push(move);
                }
            } else {
                if (BLACK_PAWN_DST_TO_EN_PASSANT_REMOVES[move.positions.dst].beat == src) {
                    moves.push(move);
                }
            }
        }
    }

    return moves;
}

const Moves& Engine::createMoves(const String& srcUci) {
    const Position src = convertStrToPosition(srcUci);
    return createMoves(src);
}

String Engine::createFen() const {
    String fen{};
    for (uint8_t row = 0; row < BOARD_SQUARES_NUM; row++) {
        if (row > 0) {
            fen += "/";
        }

        uint8_t count = 0;
        for (uint8_t col = 0; col < BOARD_SQUARES_NUM; col++) {
            Piece piece = board[col + row * BOARD_SQUARES_NUM];
            if (piece.isEmpty()) {
                count++;
            }
            if (!piece.isEmpty() || col == BOARD_SQUARES_NUM - 1) {
                if (count > 0) {
                    fen += convertSquareNumberToChar(count);
                    count = 0;
                }
            }
            if (!piece.isEmpty()) {
                fen += piece.toChar();
            }
        }
    }

    fen += ' ';
    fen += convertPieceColorToChar(rootRound.turn);

    fen += ' ';
    const auto& castling = rootRound.castling;
    if (castling.WhiteKing or castling.WhiteQueen or castling.BlackKing or castling.BlackQueen) {
        if (castling.WhiteKing) {
            fen += "K";
        }
        if (castling.WhiteQueen) {
            fen += "Q";
        }
        if (castling.BlackKing) {
            fen += "k";
        }
        if (castling.BlackQueen) {
            fen += "q";
        }
    } else {
        fen += "-";
    }

    fen += ' ';
    const auto& enPassant = rootRound.enPassant;
    if (!enPassant.isNone()) {
        fen += convertPositionToStr(enPassant);
    } else {
        fen += "-";
    }

    fen += ' ';
    fen += convertMultiNumberToStr(rootRound.halfMove);

    fen += ' ';
    fen += convertMultiNumberToStr(rootRound.fullMove);

    return fen;
}

bool Engine::applyFen(const String& fen) {
    rootRound.turn = PieceColor::White;
    rootRound.castling = CastlingAbilities();
    rootRound.enPassant = Position::none();
    rootRound.halfMove = 0;
    rootRound.fullMove = 0;
    rootRound.status = RoundStatus::Normal;
    kingPositions = KingPositions{};
    for (Position pos = 0; pos < BOARD_SIZE; pos++) {
        board[pos] = Piece::empty();
    }

    uint8_t field = 0;
    Position pos = 0;
    for (size_t i = 0; i < fen.length(); i++) {
        const char ch = fen[i];
        if (ch == ' ') {
            field++;
            if (field > 5) {
                break;
            }
            continue;
        }
        if (field == 0) {
            if (ch == '/') {
                continue;
            }
            if (isSquareNumberChar(ch)) {
                pos += convertCharToSquareNumber(ch);
                continue;
            }
            if (pos >= BOARD_SIZE) {
                return false;
            }
            const Piece piece = Piece(ch);
            if (piece.isEmpty()) {
                return false;
            }
            board[pos++] = piece;
        } else if (field == 1) {
            if (!isPieceColorChar(ch)) {
                return false;
            }
            rootRound.turn = convertCharToPieceColor(ch);
        } else if (field == 2) {
            switch (ch) {
                case 'K':
                    rootRound.castling.WhiteKing = true;
                    break;
                case 'Q':
                    rootRound.castling.WhiteQueen = true;
                    break;
                case 'k':
                    rootRound.castling.BlackKing = true;
                    break;
                case 'q':
                    rootRound.castling.BlackQueen = true;
                    break;
                case '-':
                    break;
                default:
                    return false;
            }
        } else if (field == 3) {
            if (ch != '-') {
                rootRound.enPassant = convertStrToPosition(fen.substring(i, i + 2));
                if (rootRound.enPassant.isNone()) {
                    return false;
                }
                i++;
            }
        } else if (field == 4) {
            if (!isNumberChar(ch)) {
                break;
            }
            rootRound.halfMove = convertCharToMultiNumber(ch, rootRound.halfMove);
        } else if (field == 5) {
            if (!isNumberChar(ch)) {
                break;
            }
            rootRound.fullMove = convertCharToMultiNumber(ch, rootRound.fullMove);
        }
    }

    if (field < 1) {
        return false;
    }
    if (!rootRound.fullMove) {
        rootRound.fullMove = 1;
    }
    return true;
}

void Engine::applyMove(const Move& move) {
    redoBoard(rootRound, move);
    diveRound(rootRound, rootNextRound, move);
    rootRound = rootNextRound;
}

void Engine::updateRound() {
    if (rootRound.isEnd()) {
        return;
    }
    if (isDrawByDeadPosition()) {
        rootRound.status = RoundStatus::DeadPosition;
        return;
    }

    rootRound.best = Move::none();
    setRoundWeights(rootRound);
    setRoundPhase(rootRound);
    setKingsPositions();

    if (kingPositions.white.isNone() || kingPositions.black.isNone()) {
        rootRound.status = RoundStatus::Invalid;
        CHESS_ENGINE_LOG(TAG, "Invalid king positions");
        return;
    }

    setRoundCheckStatus(rootRound);
    setCheckPosition(rootRound);
    generateLegalMoves(ROOT_DEPTH);

    if (rootRound.moves.empty()) {
        rootRound.status = rootRound.isCheck() ? RoundStatus::Checkmate : RoundStatus::Stalemate;
        return;
    }
    if (isDrawByThreefoldRepetition()) {
        rootRound.status = RoundStatus::ThreefoldRepetition;
        return;
    }
    if (isDrawByFiftyMove(rootRound)) {
        rootRound.status = RoundStatus::FiftyMove;
    }
}

bool Engine::isAutoPromotionNeeded(const Move& move) const {
    const Position dst = move.positions.dst;
    if (move.isNone() || dst.isNone()) {
        return false;
    }
    const Piece piece = board[move.positions.src];
    return (piece == PieceType::WhitePawn && isWhitePawnPromotion(dst)) ||
           (piece == PieceType::BlackPawn && isBlackPawnPromotion(dst));
}

// Sliding Piece Attacks: https://chessprogramming.org/Sliding_Piece_Attacks
bool Engine::isWhiteKingInSlidingCheck() const {
#ifdef CHESS_ENGINE_LOGS
    if (kingPositions.white.isNone() || board[kingPositions.white] != PieceType::WhiteKing) {
        CHESS_ENGINE_LOG(TAG, "Invalid white king position: %d", static_cast<int>(kingPositions.white));
        return false;
    }
#endif
    for (auto dsts : BISHOP_DESTINATIONS[kingPositions.white]) {
        for (Position dst : dsts) {
            const auto piece = board[dst];
            if (piece == PieceType::BlackBishop || piece == PieceType::BlackQueen) {
                return true;
            }
            if (!piece.isEmpty()) {
                break;
            }
        }
    }
    for (auto dsts : ROOK_DESTINATIONS[kingPositions.white]) {
        for (Position dst : dsts) {
            const auto piece = board[dst];
            if (piece == PieceType::BlackRook || piece == PieceType::BlackQueen) {
                return true;
            }
            if (!piece.isEmpty()) {
                break;
            }
        }
    }
    return false;
}

bool Engine::isBlackKingInSlidingCheck() const {
#ifdef CHESS_ENGINE_LOGS
    if (kingPositions.black.isNone() || board[kingPositions.black] != PieceType::BlackKing) {
        CHESS_ENGINE_LOG(TAG, "Invalid black king position: %d", static_cast<int>(kingPositions.black));
        return false;
    }
#endif
    for (auto dsts : BISHOP_DESTINATIONS[kingPositions.black]) {
        for (Position dst : dsts) {
            const auto piece = board[dst];
            if (piece == PieceType::WhiteBishop || piece == PieceType::WhiteQueen) {
                return true;
            }
            if (!piece.isEmpty()) {
                break;
            }
        }
    }
    for (auto dsts : ROOK_DESTINATIONS[kingPositions.black]) {
        for (Position dst : dsts) {
            const auto piece = board[dst];
            if (piece == PieceType::WhiteRook || piece == PieceType::WhiteQueen) {
                return true;
            }
            if (!piece.isEmpty()) {
                break;
            }
        }
    }
    return false;
}

bool Engine::isOpponentKingInSlidingCheck(const Round& round) const {
    return round.isWhiteTurn() ? isBlackKingInSlidingCheck() : isWhiteKingInSlidingCheck();
}

bool Engine::isWhiteKingInCheck() const {
    if (isWhiteKingInSlidingCheck()) {
        return true;
    }
    for (Position dst : KNIGHT_DESTINATIONS[kingPositions.white]) {
        if (board[dst] == PieceType::BlackKnight) {
            return true;
        }
    }
    for (Position dst : WHITE_PAWN_BEAT_DESTINATIONS[kingPositions.white]) {
        if (board[dst] == PieceType::BlackPawn) {
            return true;
        }
    }
    for (Position dst : KING_DESTINATIONS[kingPositions.white]) {
        if (board[dst] == PieceType::BlackKing) {
            return true;
        }
    }
    return false;
}

bool Engine::isBlackKingInCheck() const {
    if (isBlackKingInSlidingCheck()) {
        return true;
    }
    for (Position dst : KNIGHT_DESTINATIONS[kingPositions.black]) {
        if (board[dst] == PieceType::WhiteKnight) {
            return true;
        }
    }
    for (Position dst : BLACK_PAWN_BEAT_DESTINATIONS[kingPositions.black]) {
        if (board[dst] == PieceType::WhitePawn) {
            return true;
        }
    }
    for (Position dst : KING_DESTINATIONS[kingPositions.black]) {
        if (board[dst] == PieceType::WhiteKing) {
            return true;
        }
    }
    return false;
}

bool Engine::isKingInCheck(PieceColor turn) const {
    return turn == PieceColor::White ? isWhiteKingInCheck() : isBlackKingInCheck();
}

bool Engine::isKingInCheck(const Round& round) const {
    return round.isWhiteTurn() ? isWhiteKingInCheck() : isBlackKingInCheck();
}

bool Engine::isOpponentKingInCheck(const Round& round) const {
    return round.isWhiteTurn() ? isBlackKingInCheck() : isWhiteKingInCheck();
}

void Engine::setKingsPositions() {
    for (Position pos = 0; pos < BOARD_SIZE; pos++) {
        if (board[pos] == PieceType::WhiteKing) {
            kingPositions.white = pos;
        }
        if (board[pos] == PieceType::BlackKing) {
            kingPositions.black = pos;
        }
    }
}

void Engine::setCheckPosition(Round& round) {
    if (round.isCheck()) {
        kingPositions.check = round.isWhiteTurn() ? kingPositions.white : kingPositions.black;
    } else {
        kingPositions.check = Position::none();
    }
}

void Engine::setRoundCheckStatus(Round& round) const {
    round.status = isKingInCheck(round) ? RoundStatus::Check : RoundStatus::Normal;
}

void Engine::setRoundWeights(Round& round) const {
    RoundWeights weights{};
    for (Position pos = 0; pos < BOARD_SIZE; pos++) {
        const Piece piece = board[pos];
        if (piece.isEmpty()) {
            continue;
        }
        const PieceKind kind = piece.getKind();
        if (piece.isWhite()) {
            weights.material.white += MATERIAL_WEIGHTS[kind];
            weights.pst += WHITE_PST_WEIGHTS(pos, kind);
        } else {
            weights.material.black += MATERIAL_WEIGHTS[kind];
            weights.pst -= BLACK_PST_WEIGHTS(pos, kind);
        }
    }
    round.weights = weights;
}

void Engine::setRoundPhase(Round& round) const {
    const auto& weights = round.weights;
    const auto materialWeight = weights.material.white + weights.material.black;
    round.phase = materialWeight < LATE_PHASE_MATERIAL_WEIGHT_THRESHOLD ? RoundPhase::Late : RoundPhase::Early;
}

void Engine::diveWhiteCastling(CastlingAbilities& castling, const Move& move) const {
    const auto& positions = move.positions;

    if (positions.src == WHITE_KING_CASTLING_MOVE.king.src) {
        castling.WhiteKing = false;
        castling.WhiteQueen = false;
    } else if (positions.src == WHITE_KING_CASTLING_MOVE.rook.src) {
        castling.WhiteKing = false;
    } else if (positions.src == WHITE_QUEEN_CASTLING_MOVE.rook.src) {
        castling.WhiteQueen = false;
    }
    if (positions.dst == BLACK_KING_CASTLING_MOVE.rook.src) {
        castling.BlackKing = false;
    } else if (positions.dst == BLACK_QUEEN_CASTLING_MOVE.rook.src) {
        castling.BlackQueen = false;
    }
}

void Engine::diveBlackCastling(CastlingAbilities& castling, const Move& move) const {
    const auto& positions = move.positions;

    if (positions.src == BLACK_KING_CASTLING_MOVE.king.src) {
        castling.BlackKing = false;
        castling.BlackQueen = false;
    } else if (positions.src == BLACK_KING_CASTLING_MOVE.rook.src) {
        castling.BlackKing = false;
    } else if (positions.src == BLACK_QUEEN_CASTLING_MOVE.rook.src) {
        castling.BlackQueen = false;
    }
    if (positions.dst == WHITE_KING_CASTLING_MOVE.rook.src) {
        castling.WhiteKing = false;
    } else if (positions.dst == WHITE_QUEEN_CASTLING_MOVE.rook.src) {
        castling.WhiteQueen = false;
    }
}

void Engine::diveWhiteEnPassant(Position& enPassant, const Move& move) const {
    const auto& positions = move.positions;

    if (move.isNormal() && move.pieces.src == PieceType::WhitePawn && isWhitePawnDoubleMove(positions)) {
        const auto& ep = WHITE_PAWN_DOUBLE_MOVE_SRC_TO_EN_PASSANT_MOVES[positions.src];
        if (!ep.isNone() && board[ep.dst].isEmpty() &&
            ((!ep.srcLeft.isNone() && board[ep.srcLeft] == PieceType::BlackPawn) ||
             (!ep.srcRight.isNone() && board[ep.srcRight] == PieceType::BlackPawn))) {
            enPassant = ep.dst;
        }
    }
}

void Engine::diveBlackEnPassant(Position& enPassant, const Move& move) const {
    const auto& positions = move.positions;

    if (move.isNormal() && move.pieces.src == PieceType::BlackPawn && isBlackPawnDoubleMove(positions)) {
        const auto& ep = BLACK_PAWN_DOUBLE_MOVE_SRC_TO_EN_PASSANT_MOVES[positions.src];
        if (!ep.isNone() && board[ep.dst].isEmpty() &&
            ((!ep.srcLeft.isNone() && board[ep.srcLeft] == PieceType::WhitePawn) ||
             (!ep.srcRight.isNone() && board[ep.srcRight] == PieceType::WhitePawn))) {
            enPassant = ep.dst;
        }
    }
}

void Engine::diveWhiteMaterialWeights(MaterialWeights& weights, const Move& move) const {
    if (move.isBeat()) {
        weights.black -= MATERIAL_WEIGHTS[move.pieces.dst.getKind()];
    }
    if (move.isPromotion()) {
        const auto promKind = convertMoveTypeToPieceKind(move.type);
        weights.white += MATERIAL_WEIGHTS[promKind] - MATERIAL_WEIGHTS[PieceKind::Pawn];
    }
}

void Engine::diveBlackMaterialWeights(MaterialWeights& weights, const Move& move) const {
    if (move.isBeat()) {
        weights.white -= MATERIAL_WEIGHTS[move.pieces.dst.getKind()];
    }
    if (move.isPromotion()) {
        const auto promKind = convertMoveTypeToPieceKind(move.type);
        weights.black += MATERIAL_WEIGHTS[promKind] - MATERIAL_WEIGHTS[PieceKind::Pawn];
    }
}

void Engine::diveWhitePositionalWeights(PstWeights& weights, const Move& move) const {
    const auto positions = move.positions;
    const auto pieces = move.pieces;
    const auto srcKind = pieces.src.getKind();
    const auto dstKind = move.isPromotion() ? convertMoveTypeToPieceKind(move.type) : srcKind;

    weights += WHITE_PST_WEIGHTS(positions.dst, dstKind) - WHITE_PST_WEIGHTS(positions.src, srcKind);

    if (move.isCastling()) {
        const auto rook =
            move.type == MoveType::CastleKing ? WHITE_KING_CASTLING_MOVE.rook : WHITE_QUEEN_CASTLING_MOVE.rook;
        weights += WHITE_PST_WEIGHTS(rook.dst, PieceKind::Rook) - WHITE_PST_WEIGHTS(rook.src, PieceKind::Rook);
    }

    if (move.isBeat()) {
        const auto beat = move.isEnPassant() ? WHITE_PAWN_DST_TO_EN_PASSANT_REMOVES[positions.dst].beat : positions.dst;
        weights += BLACK_PST_WEIGHTS(beat, pieces.dst.getKind());
    }
}

void Engine::diveBlackPositionalWeights(PstWeights& weights, const Move& move) const {
    const auto positions = move.positions;
    const auto pieces = move.pieces;
    const auto srcKind = pieces.src.getKind();
    const auto dstKind = move.isPromotion() ? convertMoveTypeToPieceKind(move.type) : srcKind;

    weights += BLACK_PST_WEIGHTS(positions.src, srcKind) - BLACK_PST_WEIGHTS(positions.dst, dstKind);

    if (move.isCastling()) {
        const auto rook =
            move.type == MoveType::CastleKing ? BLACK_KING_CASTLING_MOVE.rook : BLACK_QUEEN_CASTLING_MOVE.rook;
        weights += BLACK_PST_WEIGHTS(rook.src, PieceKind::Rook) - BLACK_PST_WEIGHTS(rook.dst, PieceKind::Rook);
    }

    if (move.isBeat()) {
        const auto beat = move.isEnPassant() ? BLACK_PAWN_DST_TO_EN_PASSANT_REMOVES[positions.dst].beat : positions.dst;
        weights -= WHITE_PST_WEIGHTS(beat, pieces.dst.getKind());
    }
}

void Engine::diveRound(const Round& round, Round& nextRound, const Move& move) {
    nextRound.turn = !round.turn;
    nextRound.castling = round.castling;
    nextRound.enPassant = Position::none();
    nextRound.weights = round.weights;
    nextRound.halfMove = isHalfMove(move) ? 0 : round.halfMove + 1;
    nextRound.fullMove = round.isBlackTurn() ? round.fullMove + 1 : round.fullMove;

    if (round.isWhiteTurn()) {
        diveWhiteCastling(nextRound.castling, move);
        diveWhiteEnPassant(nextRound.enPassant, move);
        diveWhiteMaterialWeights(nextRound.weights.material, move);
        if (modes.boardEval == BoardEvalMode::Positional) {
            diveWhitePositionalWeights(nextRound.weights.pst, move);
        }
    } else {
        diveBlackCastling(nextRound.castling, move);
        diveBlackEnPassant(nextRound.enPassant, move);
        diveBlackMaterialWeights(nextRound.weights.material, move);
        if (modes.boardEval == BoardEvalMode::Positional) {
            diveBlackPositionalWeights(nextRound.weights.pst, move);
        }
    }

    setRoundPhase(nextRound);
    supervisor.incrementNodesCount();
}

void Engine::redoMove(const Move& move) {
    const auto& positions = move.positions;
    const auto& pieces = move.pieces;

    board[positions.src] = Piece::empty();
    board[positions.dst] = pieces.src;
}

void Engine::redoNormalMove(Position& kingPos, Piece srcKing, const Move& move) {
    redoMove(move);
    if (move.pieces.src == srcKing) {
        kingPos = move.positions.dst;
    }
}

void Engine::redoEnPassantMove(const EnPassantDstToEnPassantRemovesMap& map, const Move& move) {
    redoMove(move);
    board[map[move.positions.dst].beat] = Piece::empty();
}

void Engine::redoCastlingMove(Position& kingPos, const CastlingMove& castle) {
    const auto& pieces = castle.pieces;
    const auto& king = castle.king;
    const auto& rook = castle.rook;

    board[king.src] = Piece::empty();
    board[rook.src] = Piece::empty();
    for (Position pos : castle.empties) {
        board[pos] = Piece::empty();
    }
    board[king.dst] = pieces.king;
    board[rook.dst] = pieces.rook;
    kingPos = king.dst;
}

void Engine::redoPromoMove(Piece promPiece, const Move& move) {
    redoMove(move);
    board[move.positions.dst] = promPiece;
}

void Engine::redoBoard(const Round& round, const Move& move) {
    const auto type = move.type;

    if (round.isWhiteTurn()) {
        switch (type) {
            case MoveType::Normal:
                redoNormalMove(kingPositions.white, PieceType::WhiteKing, move);
                break;
            case MoveType::EnPassant:
                redoEnPassantMove(WHITE_PAWN_DST_TO_EN_PASSANT_REMOVES, move);
                break;
            case MoveType::CastleKing:
                redoCastlingMove(kingPositions.white, WHITE_KING_CASTLING_MOVE);
                break;
            case MoveType::CastleQueen:
                redoCastlingMove(kingPositions.white, WHITE_QUEEN_CASTLING_MOVE);
                break;
            case MoveType::PromoKnight:
            case MoveType::PromoBishop:
            case MoveType::PromoRook:
            case MoveType::PromoQueen:
                redoPromoMove(Piece::white(convertMoveTypeToPieceKind(type)), move);
                break;
        }
    } else {
        switch (type) {
            case MoveType::Normal:
                redoNormalMove(kingPositions.black, PieceType::BlackKing, move);
                break;
            case MoveType::EnPassant:
                redoEnPassantMove(BLACK_PAWN_DST_TO_EN_PASSANT_REMOVES, move);
                break;
            case MoveType::CastleKing:
                redoCastlingMove(kingPositions.black, BLACK_KING_CASTLING_MOVE);
                break;
            case MoveType::CastleQueen:
                redoCastlingMove(kingPositions.black, BLACK_QUEEN_CASTLING_MOVE);
                break;
            case MoveType::PromoKnight:
            case MoveType::PromoBishop:
            case MoveType::PromoRook:
            case MoveType::PromoQueen:
                redoPromoMove(Piece::black(convertMoveTypeToPieceKind(type)), move);
                break;
        }
    }
}

void Engine::undoMove(const Move& move) {
    const auto& positions = move.positions;
    const auto& pieces = move.pieces;

    board[positions.src] = pieces.src;
    board[positions.dst] = pieces.dst;
}

void Engine::undoNormalMove(Position& kingPos, Piece srcKing, const Move& move) {
    undoMove(move);
    if (move.pieces.src == srcKing) {
        kingPos = move.positions.src;
    }
}

void Engine::undoEnPassantMove(const EnPassantDstToEnPassantRemovesMap& map, Piece beatPawn, const Move& move) {
    const auto& positions = move.positions;

    board[positions.src] = move.pieces.src;
    board[positions.dst] = Piece::empty();
    board[map[positions.dst].beat] = beatPawn;
}

void Engine::undoCastlingMove(Position& kingPos, const CastlingMove& castle) {
    const auto& pieces = castle.pieces;
    const auto& king = castle.king;
    const auto& rook = castle.rook;

    board[king.src] = pieces.king;
    board[rook.src] = pieces.rook;
    for (Position pos : castle.empties) {
        board[pos] = Piece::empty();
    }
    kingPos = king.src;
}

void Engine::undoPromoMove(const Move& move) {
    undoMove(move);
}

void Engine::undoBoard(const Round& round, const Move& move) {
    const auto type = move.type;

    if (round.isWhiteTurn()) {
        switch (move.type) {
            case MoveType::Normal:
                undoNormalMove(kingPositions.white, PieceType::WhiteKing, move);
                break;
            case MoveType::EnPassant:
                undoEnPassantMove(WHITE_PAWN_DST_TO_EN_PASSANT_REMOVES, PieceType::BlackPawn, move);
                break;
            case MoveType::CastleKing:
                undoCastlingMove(kingPositions.white, WHITE_KING_CASTLING_MOVE);
                break;
            case MoveType::CastleQueen:
                undoCastlingMove(kingPositions.white, WHITE_QUEEN_CASTLING_MOVE);
                break;
            case MoveType::PromoKnight:
            case MoveType::PromoBishop:
            case MoveType::PromoRook:
            case MoveType::PromoQueen:
                undoPromoMove(move);
                break;
        }
    } else {
        switch (type) {
            case MoveType::Normal:
                undoNormalMove(kingPositions.black, PieceType::BlackKing, move);
                break;
            case MoveType::EnPassant:
                undoEnPassantMove(BLACK_PAWN_DST_TO_EN_PASSANT_REMOVES, PieceType::WhitePawn, move);
                break;
            case MoveType::CastleKing:
                undoCastlingMove(kingPositions.black, BLACK_KING_CASTLING_MOVE);
                break;
            case MoveType::CastleQueen:
                undoCastlingMove(kingPositions.black, BLACK_QUEEN_CASTLING_MOVE);
                break;
            case MoveType::PromoKnight:
            case MoveType::PromoBishop:
            case MoveType::PromoRook:
            case MoveType::PromoQueen:
                undoPromoMove(move);
                break;
        }
    }
}

void Engine::addMove(Moves& moves, Position src, Position dst, MoveType type) const {
    moves.push(Move(board[src], board[dst], src, dst, type));
}

void Engine::addPawnMoves(Moves& moves, Position src) const {
    if (board[src] == PieceType::WhitePawn) {
        for (Position dst : WHITE_PAWN_MOVE_DESTINATIONS[src]) {
            if (!board[dst].isEmpty()) {
                break;
            }
            if (isWhitePawnPromotion(dst)) {
                addMove(moves, src, dst, MoveType::PromoKnight);
                addMove(moves, src, dst, MoveType::PromoBishop);
                addMove(moves, src, dst, MoveType::PromoRook);
                addMove(moves, src, dst, MoveType::PromoQueen);
            } else {
                addMove(moves, src, dst, MoveType::Normal);
            }
        }
        for (Position dst : WHITE_PAWN_BEAT_DESTINATIONS[src]) {
            if (!board[dst].isBlack()) {
                continue;
            }
            if (isWhitePawnPromotion(dst)) {
                addMove(moves, src, dst, MoveType::PromoKnight);
                addMove(moves, src, dst, MoveType::PromoBishop);
                addMove(moves, src, dst, MoveType::PromoRook);
                addMove(moves, src, dst, MoveType::PromoQueen);
            } else {
                addMove(moves, src, dst, MoveType::Normal);
            }
        }
    } else if (board[src] == PieceType::BlackPawn) {
        for (Position dst : BLACK_PAWN_MOVE_DESTINATIONS[src]) {
            if (!board[dst].isEmpty()) {
                break;
            }
            if (isBlackPawnPromotion(dst)) {
                addMove(moves, src, dst, MoveType::PromoKnight);
                addMove(moves, src, dst, MoveType::PromoBishop);
                addMove(moves, src, dst, MoveType::PromoRook);
                addMove(moves, src, dst, MoveType::PromoQueen);
            } else {
                addMove(moves, src, dst, MoveType::Normal);
            }
        }
        for (Position dst : BLACK_PAWN_BEAT_DESTINATIONS[src]) {
            if (!board[dst].isWhite()) {
                continue;
            }
            if (isBlackPawnPromotion(dst)) {
                addMove(moves, src, dst, MoveType::PromoKnight);
                addMove(moves, src, dst, MoveType::PromoBishop);
                addMove(moves, src, dst, MoveType::PromoRook);
                addMove(moves, src, dst, MoveType::PromoQueen);
            } else {
                addMove(moves, src, dst, MoveType::Normal);
            }
        }
    }
}

void Engine::addKingMoves(Moves& moves, Position src) const {
    Pieces pieces{board[src]};
    for (Position dst : KING_DESTINATIONS[src]) {
        pieces.dst = board[dst];
        if (pieces.src.canMoveTo(pieces.dst)) {
            moves.push(Move(pieces, Positions{src, dst}, MoveType::Normal));
        }
    }
}

void Engine::addKnightMoves(Moves& moves, Position src) const {
    Pieces pieces{board[src]};
    for (Position dst : KNIGHT_DESTINATIONS[src]) {
        pieces.dst = board[dst];
        if (pieces.src.canMoveTo(pieces.dst)) {
            moves.push(Move(pieces, Positions{src, dst}, MoveType::Normal));
        }
    }
}

void Engine::addRookMoves(Moves& moves, Position src) const {
    Pieces pieces{board[src]};
    for (auto dsts : ROOK_DESTINATIONS[src]) {
        for (Position dst : dsts) {
            pieces.dst = board[dst];
            if (pieces.src.canMoveTo(pieces.dst)) {
                moves.push(Move(pieces, Positions{src, dst}, MoveType::Normal));
            }
            if (!pieces.dst.isEmpty()) {
                break;
            }
        }
    }
}

void Engine::addBishopMoves(Moves& moves, Position src) const {
    Pieces pieces{board[src]};
    for (auto dsts : BISHOP_DESTINATIONS[src]) {
        for (Position dst : dsts) {
            pieces.dst = board[dst];
            if (pieces.src.canMoveTo(pieces.dst)) {
                moves.push(Move(pieces, Positions{src, dst}, MoveType::Normal));
            }
            if (!pieces.dst.isEmpty()) {
                break;
            }
        }
    }
}

void Engine::addQueenMoves(Moves& moves, Position src) const {
    addRookMoves(moves, src);
    addBishopMoves(moves, src);
}

void Engine::addEnPassantMoves(Moves& moves, const Round& round, const EnPassantRemoves& removes, Piece srcPawn,
                               Piece beatPawn) const {
    if (removes.isNone()) {
        return;
    }
    if (!removes.srcLeft.isNone() && board[removes.srcLeft] == srcPawn) {
        moves.push(Move(srcPawn, beatPawn, removes.srcLeft, round.enPassant, MoveType::EnPassant));
    }
    if (!removes.srcRight.isNone() && board[removes.srcRight] == srcPawn) {
        moves.push(Move(srcPawn, beatPawn, removes.srcRight, round.enPassant, MoveType::EnPassant));
    }
}

void Engine::addEnPassantMoves(Moves& moves, const Round& round) const {
    if (round.enPassant.isNone() || !board[round.enPassant].isEmpty()) {
        return;
    }
    if (round.isWhiteTurn()) {
        addEnPassantMoves(moves, round, WHITE_PAWN_DST_TO_EN_PASSANT_REMOVES[round.enPassant], PieceType::WhitePawn,
                          PieceType::BlackPawn);
    } else {
        addEnPassantMoves(moves, round, BLACK_PAWN_DST_TO_EN_PASSANT_REMOVES[round.enPassant], PieceType::BlackPawn,
                          PieceType::WhitePawn);
    }
}

void Engine::addCastlingMove(Moves& moves, Position& kingPos, const CastlingMove& castle) {
    const auto& pieces = castle.pieces;
    const auto& king = castle.king;
    const auto& rook = castle.rook;

    if (board[king.src] != pieces.king || board[rook.src] != pieces.rook) {
        return;
    }
    for (Position pos : castle.empties) {
        if (!board[pos].isEmpty()) {
            return;
        }
    }

    board[king.src] = Piece::empty();
    board[rook.dst] = pieces.king;
    kingPos = rook.dst;
    if (!isKingInCheck(pieces.king.getColor())) {
        moves.push(Move(pieces.king, Piece::empty(), king.src, king.dst, castle.type));
    }
    board[king.src] = pieces.king;
    board[rook.dst] = Piece::empty();
    kingPos = king.src;
}

void Engine::addCastlingMoves(Moves& moves, const Round& round) {
    if (round.isWhiteTurn() && !round.isCheck()) {
        if (round.castling.WhiteKing) {
            addCastlingMove(moves, kingPositions.white, WHITE_KING_CASTLING_MOVE);
        }
        if (round.castling.WhiteQueen) {
            addCastlingMove(moves, kingPositions.white, WHITE_QUEEN_CASTLING_MOVE);
        }
    } else if (round.isBlackTurn() && !round.isCheck()) {
        if (round.castling.BlackKing) {
            addCastlingMove(moves, kingPositions.black, BLACK_KING_CASTLING_MOVE);
        }
        if (round.castling.BlackQueen) {
            addCastlingMove(moves, kingPositions.black, BLACK_QUEEN_CASTLING_MOVE);
        }
    }
}

void Engine::addMoves(Moves& moves, const Round& round) {
    for (Position src = 0; src < BOARD_SIZE; src++) {
        const auto& piece = board[src];
        if (piece.isEmpty() || piece.getColor() != round.turn) {
            continue;
        }
        switch (piece.getKind()) {
            case PieceKind::Pawn:
                addPawnMoves(moves, src);
                break;
            case PieceKind::Knight:
                addKnightMoves(moves, src);
                break;
            case PieceKind::Bishop:
                addBishopMoves(moves, src);
                break;
            case PieceKind::Rook:
                addRookMoves(moves, src);
                break;
            case PieceKind::Queen:
                addQueenMoves(moves, src);
                break;
            case PieceKind::King:
                addKingMoves(moves, src);
                break;
            default:
                break;
        }
    }

    addEnPassantMoves(moves, round);
    addCastlingMoves(moves, round);
}

#ifdef CHESS_ENGINE_MULTI_TASK
void Engine::addMultiTaskMoves(Moves& moves, const Round& round, Depth depth) {
    task.depth = depth;
    task.isRunning = true;

    for (Position src = 0; src < BOARD_SIZE; src++) {
        const auto& piece = board[src];
        if (piece.isEmpty() || piece.getColor() != round.turn) {
            continue;
        }
        switch (piece.getKind()) {
            case PieceKind::Knight:
                addKnightMoves(moves, src);
                break;
            case PieceKind::Bishop:
                addBishopMoves(moves, src);
                break;
            case PieceKind::Rook:
                addRookMoves(moves, src);
                break;
            case PieceKind::Queen:
                addQueenMoves(moves, src);
                break;
            case PieceKind::King:
                if (round.phase == RoundPhase::Late) {
                    addKingMoves(moves, src);
                }
                break;
            default:
                break;
        }
    }
    while (task.isRunning) {
        delayMicroseconds(0);
    }

    moves.push(task.moves);
    addCastlingMoves(moves, round);
}

void Engine::addTaskMoves(Depth depth) {
    auto& round = search.getRound(depth);
    task.moves.clear();

    for (Position src = 0; src < BOARD_SIZE; src++) {
        const auto& piece = board[src];
        if (piece.isEmpty() || piece.getColor() != round.turn) {
            continue;
        }
        switch (piece.getKind()) {
            case PieceKind::Pawn:
                addPawnMoves(task.moves, src);
                break;
            case PieceKind::King:
                if (round.phase == RoundPhase::Early) {
                    addKingMoves(task.moves, src);
                }
                break;
            default:
                break;
        }
    }

    addEnPassantMoves(task.moves, round);
}
#endif

void Engine::addWeights(Depth depth) {
    auto& round = search.getRound(depth);

    for (Move& move : round.moves) {
        move.weight = move.pieces.dst.getKindWeight();
        if (move.isPromotion()) {
            move.weight += MATERIAL_WEIGHTS[convertMoveTypeToPieceKind(move.type)];
        }
        move.weight *= CAPTURE_AND_PROMOTION_MOVE_WEIGHT_FACTOR;

        if (search.isPrevRoundExist(depth)) {
            if (move.positions.dst == search.getPrevRound(depth).move.positions.dst) {
                move.weight += RECAPTURE_EXTRA_MOVE_WEIGHT;
            }
            if (round.best.positions == move.positions) {
                move.weight += SAME_AS_BEST_EXTRA_MOVE_WEIGHT;
            }
        }
    }
}

void Engine::sortMoves(Moves& moves) const {
    for (size_t i = 0; i + 1 < moves.size(); i++) {
        Weight maxWeight = moves[i].weight;
        size_t maxJ = i;
        for (size_t j = i + 1; j < moves.size(); j++) {
            if (moves[j].weight > maxWeight) {
                maxWeight = moves[j].weight;
                maxJ = j;
            }
        }

        if (maxJ != i) {
            std::swap(moves[i], moves[maxJ]);
        }
    }
}

void Engine::sortMovesUntilQuiet(Moves& moves) const {
    for (size_t i = 0; i + 1 < moves.size(); i++) {
        Weight maxWeight = moves[i].weight;
        size_t maxJ = i;
        for (size_t j = i + 1; j < moves.size(); j++) {
            if (moves[j].weight > maxWeight) {
                maxWeight = moves[j].weight;
                maxJ = j;
            }
        }

        // Quiet Moves: https://chessprogramming.org/Quiet_Moves
        if (maxWeight == QUIET_MOVE_WEIGHT) {
            return;
        }

        if (maxJ != i) {
            std::swap(moves[i], moves[maxJ]);
        }
    }
}

void Engine::generateMoves(Depth depth) {
    auto& round = search.getRound(depth);
    auto& moves = round.moves;

    moves.clear();

#ifdef CHESS_ENGINE_MULTI_TASK
    if (task.isActive) {
        addMultiTaskMoves(moves, round, depth);
    } else {
        addMoves(moves, round);
    }
#else
    addMoves(moves, round);
#endif

    addWeights(depth);
    sortMovesUntilQuiet(moves);
}

void Engine::generateLegalMoves(Depth depth) {
    auto& round = search.getRound(depth);
    auto& moves = round.moves;
    size_t legalCount = 0;

    generateMoves(depth);

    for (Move& move : moves) {
        redoBoard(round, move);
        const bool isInCheck = isKingInCheck(round);
        move.weight = isInCheck ? ILLEGAL_MOVE_WEIGHT : 0;
        if (!isInCheck) {
            legalCount++;
        }
        undoBoard(round, move);
    }

    sortMoves(moves);
    moves.resize(legalCount);
}

Weight Engine::evaluateMaterial(Weight own, Weight opponent) const {
    return own - opponent;
}

Weight Engine::evaluatePositional(Weight own, Weight opponent, Weight pst) const {
    constexpr Weight POSITIONAL_SCALE = 5000;
    constexpr Weight POSITIONAL_OFFSET = 2000;
    return POSITIONAL_SCALE * (own - opponent + pst) / (own + opponent + POSITIONAL_OFFSET);
}

Weight Engine::evaluate(const Round& round) const {
    const auto& weights = round.weights;
    const auto white = weights.material.white;
    const auto black = weights.material.black;
    const bool isWhite = round.isWhiteTurn();

    if (modes.boardEval == BoardEvalMode::Material) {
        return isWhite ? evaluateMaterial(white, black) : evaluateMaterial(black, white);
    } else {
        const Weight pst = round.phase == RoundPhase::Late ? weights.pst.late : weights.pst.early;
        return isWhite ? evaluatePositional(white, black, pst) : evaluatePositional(black, white, -pst);
    }
}

bool Engine::isDrawByFiftyMove(const Round& round) const {
    return round.halfMove >= FIFTY_MOVE_THRESHOLD;
}

// Obvious Draws: https://chessprogramming.org/Draw_Evaluation
bool Engine::isDrawByDeadPosition() const {
    unsigned pawnRookQueenCount = 0;
    unsigned knightCount = 0;
    unsigned whiteSquareBishopCount = 0;
    unsigned blackSquareBishopCount = 0;

    for (Position pos = 0; pos < BOARD_SIZE; pos++) {
        switch (board[pos].getKind()) {
            case PieceKind::Pawn:
            case PieceKind::Rook:
            case PieceKind::Queen:
                pawnRookQueenCount++;
                break;
            case PieceKind::Knight:
                knightCount++;
                break;
            case PieceKind::Bishop:
                if (isBlackSquare(pos)) {
                    blackSquareBishopCount++;
                } else {
                    whiteSquareBishopCount++;
                }
                break;
            default:
                break;
        }
    }

    const unsigned bishopCount = blackSquareBishopCount + whiteSquareBishopCount;

    if (knightCount == 1 && pawnRookQueenCount + bishopCount == 0) {
        return true;
    }
    if (bishopCount == 1 && pawnRookQueenCount + knightCount == 0) {
        return true;
    }
    if (pawnRookQueenCount + knightCount + blackSquareBishopCount == 0 ||
        pawnRookQueenCount + knightCount + whiteSquareBishopCount == 0) {
        return true;
    }
    return false;
}

bool Engine::isDrawByThreefoldRepetition(Depth depth) const {
#ifdef CHESS_ENGINE_THREEFOLD_REPETITION
    if (depth < DRAW_BY_REP_CHECK_MIN_DEPTH || modes.nullMove) {
        return false;
    }
    for (Depth i = depth; i > depth - DRAW_BY_REP_CYCLE_DEPTHS; i--) {
        const auto positions = search.getRound(i).move.positions;
        if (positions != search.getRound(i - DRAW_BY_REP_CYCLE_DEPTHS).move.positions ||
            positions != search.getRound(i - DRAW_BY_REP_TWO_CYCLES_DEPTHS).move.positions) {
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}

bool Engine::isDrawByThreefoldRepetition() const {
#ifdef CHESS_ENGINE_THREEFOLD_REPETITION
    if (history.size() < DRAW_BY_REP_CHECK_DEPTHS) {
        return false;
    }

    for (size_t i = history.size(); i > history.size() - DRAW_BY_REP_CYCLE_DEPTHS; i--) {
        const auto positions = history[i - 1].positions;
        if (positions != history[i - 1 - DRAW_BY_REP_CYCLE_DEPTHS].positions ||
            positions != history[i - 1 - DRAW_BY_REP_TWO_CYCLES_DEPTHS].positions) {
            return false;
        }
    }
    return true;
#else
    return false;
#endif
}

QuiescenceType Engine::getQuiescenceType(const Move& move) const {
    if (move.isBeat() || move.isPromotion()) {
        return QuiescenceType::Tactical;
    }
    if (move.pieces.dst.getKind() == PieceKind::King) {
        return QuiescenceType::Skip;
    }
    switch (move.pieces.src.getType()) {
        case PieceType::WhitePawn:
            if (isWhitePawnNearPromotion(move.positions.dst)) {
                return QuiescenceType::Tactical;
            }
            for (Position dst : WHITE_PAWN_BEAT_DESTINATIONS[move.positions.dst]) {
                if (dst == kingPositions.black) {
                    return QuiescenceType::Tactical;
                }
            }
            return QuiescenceType::Skip;
        case PieceType::BlackPawn:
            if (isBlackPawnNearPromotion(move.positions.dst)) {
                return QuiescenceType::Tactical;
            }
            for (Position dst : BLACK_PAWN_BEAT_DESTINATIONS[move.positions.dst]) {
                if (dst == kingPositions.white) {
                    return QuiescenceType::Tactical;
                }
            }
            return QuiescenceType::Skip;
        case PieceType::WhiteKnight:
            for (Position dst : KNIGHT_DESTINATIONS[move.positions.dst]) {
                if (board[dst] == PieceType::BlackKing) {
                    return QuiescenceType::Tactical;
                }
            }
            return QuiescenceType::MaybeOpponentInCheck;
        case PieceType::BlackKnight:
            for (Position dst : KNIGHT_DESTINATIONS[move.positions.dst]) {
                if (board[dst] == PieceType::WhiteKing) {
                    return QuiescenceType::Tactical;
                }
            }
            return QuiescenceType::MaybeOpponentInCheck;
        case PieceType::WhiteBishop:
            if (!isSameDiagonal(move.positions.dst, kingPositions.black)) {
                return QuiescenceType::Skip;
            }
            return QuiescenceType::MaybeOpponentInCheck;
        case PieceType::BlackBishop:
            if (!isSameDiagonal(move.positions.dst, kingPositions.white)) {
                return QuiescenceType::Skip;
            }
            return QuiescenceType::MaybeOpponentInCheck;
        case PieceType::WhiteRook:
            if (!isSameStraight(move.positions.dst, kingPositions.black)) {
                return QuiescenceType::Skip;
            }
            return QuiescenceType::MaybeOpponentInCheck;
        case PieceType::BlackRook:
            if (!isSameStraight(move.positions.dst, kingPositions.white)) {
                return QuiescenceType::Skip;
            }
            return QuiescenceType::MaybeOpponentInCheck;
        case PieceType::WhiteQueen:
            if (!isSameStraightOrDiagonal(move.positions.dst, kingPositions.black)) {
                return QuiescenceType::Skip;
            }
            return QuiescenceType::MaybeOpponentInCheck;
        case PieceType::BlackQueen:
            if (!isSameStraightOrDiagonal(move.positions.dst, kingPositions.white)) {
                return QuiescenceType::Skip;
            }
            return QuiescenceType::MaybeOpponentInCheck;
        default:
            break;
    }
    return QuiescenceType::MaybeOpponentInCheck;
}

// Quiescence: https://chessprogramming.org/Quiescence_Search
Weight Engine::quiescence(Depth depth, Weight alpha, Weight beta, Depth depthLeft) {
    if (supervisor.tryCheckTimeLimit()) {
        return ANY_WEIGHT;
    }

    auto& round = search.getRound(depth);

    if (depthLeft <= 0) {
        if (depth > maxReachedDepth) {
            maxReachedDepth = depth;
        }
        return evaluate(round);
    }

    Weight weight, maxWeight = MIN_WEIGHT;
    auto& nextRound = search.getNextRound(depth);

    setRoundCheckStatus(round);
    generateMoves(depth);

    if (!round.isCheck()) {
        weight = evaluate(round);
        if (weight >= maxWeight) {
            maxWeight = weight;
        }
        if (maxWeight > alpha) {
            alpha = maxWeight;
        }
        if (alpha >= beta) {
            return alpha;
        }
    }

    nextRound.best = Move::none();
    for (auto& move : round.moves) {
        QuiescenceType type = QuiescenceType::Tactical;
        Depth extraDepths = WITHOUT_EXTRA_DEPTHS;

        if (!round.isCheck()) {
            type = getQuiescenceType(move);
            if (type == QuiescenceType::Skip) {
                continue;
            }
        }

        redoBoard(round, move);

        if (type == QuiescenceType::MaybeOpponentInCheck) {
            if (!isOpponentKingInSlidingCheck(round)) {
                undoBoard(round, move);
                continue;
            } else if (depthLeft == QUIESCENCE_EXTRA_CHECK_DEPTH_LEFT &&
                       search.isNextRoundExist(depth + QUIESCENCE_EXTRA_DEPTHS)) {
                extraDepths = QUIESCENCE_EXTRA_DEPTHS;
            }
        }

        if (isKingInCheck(round)) {
            undoBoard(round, move);
            continue;
        }

        round.move = move;
        diveRound(round, nextRound, move);

        weight = -quiescence(depth + 1, -beta, -alpha, depthLeft - 1 + extraDepths);

        undoBoard(round, move);
        if (supervisor.isStopped()) {
            return weight;
        }

        if (!isMateWeight(weight) && (isDrawByThreefoldRepetition(depth) || isDrawByFiftyMove(nextRound))) {
            weight = DRAW_WEIGHT;
        }

        if (weight > maxWeight) {
            maxWeight = weight;
        }
        if (maxWeight > alpha) {
            alpha = maxWeight;
            round.best = move;
        }
        if (alpha >= beta) {
            return alpha;
        }
    }

    if (maxWeight == MIN_WEIGHT && round.isCheck()) {
        maxWeight = -MATE_WEIGHT + depth;
    }
    return maxWeight;
}

// Negamax: https://chessprogramming.org/Negamax
Weight Engine::negamax(Depth depth, Weight alpha, Weight beta, Depth depthLeft) {
    if (supervisor.tryCheckTimeLimit()) {
        return ANY_WEIGHT;
    }

    if (depthLeft <= 0) {
        const Depth quiescenceDepths =
            search.getPrevRound(depth).move.isBeat() ? QUIESCENCE_BEAT_DEPTHS : QUIESCENCE_MOVE_DEPTHS;
        return quiescence(depth, alpha, beta, quiescenceDepths);
    }

    auto& round = search.getRound(depth);

    if (depth > ROOT_DEPTH) {
        setRoundCheckStatus(round);
        generateMoves(depth);
    }

    Weight weight, maxWeight = MIN_WEIGHT;
    auto& nextRound = search.getNextRound(depth);

    // Null Move Pruning: https://chessprogramming.org/Null_Move_Pruning
    if (options.nullMove && !modes.nullMove && depth >= NULL_MOVE_CHECK_MIN_DEPTH &&
        depthLeft >= NULL_MOVE_CHECK_MIN_DEPTH_LEFT && !round.isCheck() && !search.getPrevRound(depth).move.isBeat()) {
        nextRound.castling = round.castling;
        nextRound.weights = round.weights;
        nextRound.phase = round.phase;
        nextRound.enPassant = Position::none();
        nextRound.halfMove = round.halfMove;
        nextRound.fullMove = round.fullMove;
        nextRound.best = Move::none();
        round.move = Move::none();
        modes.nullMove = NullMoveMode::Active;
        weight = -negamax(depth + 1, -beta, -beta + 1, depthLeft - 1 - NULL_MOVE_REDUCED_DEPTHS);
        modes.nullMove = NullMoveMode::Inactive;
        if (supervisor.isStopped()) {
            return weight;
        }
        if (weight >= beta) {
            return beta;
        }
    }

    // Reverse Futility Pruning: https://chessprogramming.org/Reverse_Futility_Pruning
    if (options.futility && !modes.nullMove && depth >= FUTILITY_CHECK_MIN_DEPTH &&
        depthLeft <= FUTILITY_CHECK_MAX_DEPTH_LEFT && !round.isCheck() && !search.getPrevRound(depth).move.isBeat()) {
        weight = evaluate(round);
        if (weight - FUTILITY_PRUNING_WEIGHT_MARGIN >= beta) {
            return beta;
        }
    }

    if (depth == MULTI_MOVE_DEPTH) {
        moves.clear();
    }

    nextRound.best = Move::none();
    for (auto& move : round.moves) {
        const Depth extraDepths =
            depth == NEGAMAX_EXTRA_CHECK_DEPTH && depthLeft <= MEGAMAX_EXTRA_CHECK_MAX_DEPTH_LEFT && move.isCheck ?
                NEGAMAX_EXTRA_DEPTHS :
                WITHOUT_EXTRA_DEPTHS;

        redoBoard(round, move);

        if (isKingInCheck(round)) {
            undoBoard(round, move);
            continue;
        }

        round.move = move;
        diveRound(round, nextRound, move);

        const Weight reducedAlpha =
            depth == MULTI_MOVE_DEPTH && !moves.empty() ? alpha - MULTI_MOVE_REDUCED_ALPHA_WEIGHT : alpha;

        // Lazy Evaluation: https://chessprogramming.org/Lazy_Evaluation
        if (options.lazyEval && depth >= LAZY_EVAL_CHECK_MIN_DEPTH && !modes.lazyEval && !modes.nullMove &&
            !round.isCheck() && !move.isBeat() && !rootRound.move.isCheck &&
            evaluate(nextRound) + LAZY_EVALUATION_WEIGHT_MARGIN <= alpha && !isOpponentKingInCheck(round)) {
            modes.lazyEval = LazyEvalMode::Active;
            weight = -negamax(depth + 1, -beta, -reducedAlpha, depthLeft - 1 - LAZY_EVAL_REDUCED_DEPTHS);
            if (!supervisor.isStopped()) {
                if (weight <= alpha) {
                    weight = alpha;
                } else {
                    modes.lazyEval = LazyEvalMode::Inactive;
                    weight = -negamax(depth + 1, -beta, -reducedAlpha, depthLeft - 1 + extraDepths);
                }
            }
            modes.lazyEval = LazyEvalMode::Inactive;
        } else {
            weight = -negamax(depth + 1, -beta, -reducedAlpha, depthLeft - 1 + extraDepths);
        }

        undoBoard(round, move);
        if (supervisor.isStopped()) {
            return weight;
        }

        if (!isMateWeight(weight) && (isDrawByThreefoldRepetition(depth) || isDrawByFiftyMove(nextRound))) {
            weight = DRAW_WEIGHT;
        }

        move.weight = weight;
        if (depth == MULTI_MOVE_DEPTH) {
            if (weight > alpha) {
                moves.clear();
                moves.push(move);
            } else if (weight == alpha && !moves.empty()) {
                moves.push(move);
            }
        }

        if (weight > maxWeight) {
            maxWeight = weight;
        }
        if (maxWeight > alpha) {
            alpha = maxWeight;
            round.best = move;
        }
        if (alpha >= beta) {
            return alpha;
        }
    }

    if (maxWeight == MIN_WEIGHT) {
        maxWeight = round.isCheck() ? -MATE_WEIGHT + depth : STALEMATE_WEIGHT;
    }
    return maxWeight;
}
}
