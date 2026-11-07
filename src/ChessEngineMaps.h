#pragma once
#include "ChessEngineMove.h"
#include "ChessEngineRound.h"

namespace ChessEngine {
template <size_t... I>
struct index_sequence {};

template <size_t N, size_t... I>
struct make_index_sequence_impl : make_index_sequence_impl<N - 1, N - 1, I...> {};

template <size_t... I>
struct make_index_sequence_impl<0, I...> {
    using type = index_sequence<I...>;
};

template <size_t N>
using make_index_sequence = typename make_index_sequence_impl<N>::type;

constexpr size_t MAX_DESTINATIONS = 8;

template <size_t N>
class Destinations {
    Position size;
    Position dsts[N];

    template <size_t M>
    friend class Directions;

    static constexpr Position element(int) { return 0; }
    template <typename... Rest>
    static constexpr Position element(int index, Position pos1, Rest... rest) {
        return index == 0 ? pos1 : element(index - 1, rest...);
    }

    template <size_t... I, typename... Ps>
    constexpr Destinations(index_sequence<I...>, Position pos1, Ps... ps) :
        size(1 + sizeof...(Ps)),
        dsts{element(static_cast<int>(I), pos1, ps...)...} {}

 public:
    constexpr Destinations() :
        size(0),
        dsts{} {}

    template <typename... Ps>
    constexpr Destinations(Position pos1, Ps... ps) :
        Destinations(make_index_sequence<N>{}, pos1, ps...) {
        static_assert(1 + sizeof...(Ps) <= N, "too many destinations");
    }

    const Position* begin() const { return dsts; }
    const Position* end() const { return dsts + size; }
};

template <size_t N>
class Directions {
    Position sum;
    Position dsts[N];

    static constexpr Position element(int index, Destinations<MAX_DESTINATIONS> dsts1) {
        return index == 0 ? dsts1.size : index <= dsts1.size ? dsts1.dsts[index - 1] : Position::none();
    }
    template <typename... Rest>
    static constexpr Position element(int index, Destinations<MAX_DESTINATIONS> dsts1,
                                      Destinations<MAX_DESTINATIONS> dsts2, Rest... rest) {
        return index == 0          ? dsts1.size :
               index <= dsts1.size ? dsts1.dsts[index - 1] :
                                     element(index - dsts1.size - 1, dsts2, rest...);
    }

    template <size_t... I>
    constexpr Directions(index_sequence<I...>, Destinations<MAX_DESTINATIONS> dsts1) :
        sum(dsts1.size + 1),
        dsts{element(I, dsts1)...} {}
    template <size_t... I>
    constexpr Directions(index_sequence<I...>, Destinations<MAX_DESTINATIONS> dsts1,
                         Destinations<MAX_DESTINATIONS> dsts2) :
        sum(dsts1.size + dsts2.size + 2),
        dsts{element(I, dsts1, dsts2)...} {}
    template <size_t... I>
    constexpr Directions(index_sequence<I...>, Destinations<MAX_DESTINATIONS> dsts1,
                         Destinations<MAX_DESTINATIONS> dsts2, Destinations<MAX_DESTINATIONS> dsts3) :
        sum(dsts1.size + dsts2.size + dsts3.size + 3),
        dsts{element(I, dsts1, dsts2, dsts3)...} {}
    template <size_t... I>
    constexpr Directions(index_sequence<I...>, Destinations<MAX_DESTINATIONS> dsts1,
                         Destinations<MAX_DESTINATIONS> dsts2, Destinations<MAX_DESTINATIONS> dsts3,
                         Destinations<MAX_DESTINATIONS> dsts4) :
        sum(dsts1.size + dsts2.size + dsts3.size + dsts4.size + 4),
        dsts{element(I, dsts1, dsts2, dsts3, dsts4)...} {}

 public:
    constexpr Directions(Destinations<MAX_DESTINATIONS> dsts1) :
        Directions(make_index_sequence<N>{}, dsts1) {}
    constexpr Directions(Destinations<MAX_DESTINATIONS> dsts1, Destinations<MAX_DESTINATIONS> dsts2) :
        Directions(make_index_sequence<N>{}, dsts1, dsts2) {}
    constexpr Directions(Destinations<MAX_DESTINATIONS> dsts1, Destinations<MAX_DESTINATIONS> dsts2,
                         Destinations<MAX_DESTINATIONS> dsts3) :
        Directions(make_index_sequence<N>{}, dsts1, dsts2, dsts3) {}
    constexpr Directions(Destinations<MAX_DESTINATIONS> dsts1, Destinations<MAX_DESTINATIONS> dsts2,
                         Destinations<MAX_DESTINATIONS> dsts3, Destinations<MAX_DESTINATIONS> dsts4) :
        Directions(make_index_sequence<N>{}, dsts1, dsts2, dsts3, dsts4) {}

    class DestinationsView {
        Position size;
        const Position* dsts;

     public:
        constexpr DestinationsView(Position size, const Position* dsts) :
            size(size),
            dsts(dsts) {}
        const Position* begin() const { return dsts; }
        const Position* end() const { return dsts + size; }
    };

    class Iterator {
        const Position* current;

        constexpr Position size() const { return current[0]; }
        constexpr const Position* data() const { return current + 1; }

     public:
        constexpr Iterator(const Position* current) :
            current(current) {}
        Iterator& operator++() {
            current += size() + 1;
            return *this;
        }
        DestinationsView operator*() const { return {size(), data()}; }
        bool operator!=(const Iterator& other) const { return current != other.current; }
    };

    Iterator begin() const { return {dsts}; }
    Iterator end() const { return {dsts + sum}; }
    constexpr Position size() const { return sum; }
    static constexpr Position capacity() { return N; };
};

template <size_t N>
using SourceToDestinationsMap = Destinations<N>[BOARD_SIZE];
template <size_t N>
using SourceToDirectionsMap = Directions<N>[BOARD_SIZE];

struct EnPassantMoves {
    Position dst;
    Position beat;
    Position srcLeft;
    Position srcRight;

    bool isNone() const { return dst.isNone() || beat.isNone(); }

    static const EnPassantMoves& none() {
        static constexpr EnPassantMoves value{};
        return value;
    }
};

struct EnPassantRemoves {
    Position beat;
    Position srcLeft;
    Position srcRight;

    bool isNone() const { return beat.isNone(); }

    static const EnPassantRemoves& none() {
        static constexpr EnPassantRemoves value{};
        return value;
    }
};

struct DoubleMoveSrcToEnPassantMovesMap {
    Position srcBegin;
    Position srcEnd;
    EnPassantMoves enPassantMoves[BOARD_SQUARES_NUM];

    const EnPassantMoves& operator[](Position src) const {
        if (src < srcBegin || src > srcEnd) {
            return EnPassantMoves::none();
        }
        return enPassantMoves[static_cast<size_t>(src - srcBegin)];
    }
};

struct EnPassantDstToEnPassantRemovesMap {
    Position dstBegin;
    Position dstEnd;
    EnPassantRemoves enPassantRemoves[BOARD_SQUARES_NUM];

    const EnPassantRemoves& operator[](Position dst) const {
        if (dst < dstBegin || dst > dstEnd) {
            return EnPassantRemoves::none();
        }
        return enPassantRemoves[static_cast<size_t>(dst - dstBegin)];
    }
};

struct CastlingPieces {
    Piece king;
    Piece rook;
};

struct CastlingMove {
    CastlingPieces pieces;
    Positions king;
    Positions rook;
    Destinations<3> empties;
    MoveType type;
};

struct PieceToWeightMap {
    Weight weights[7];

    Weight operator[](PieceKind kind) const { return weights[static_cast<size_t>(kind)]; }
};

struct PiecePositionToWeightMap {
    Weight weights[8][BOARD_SIZE];

    PstWeights operator()(Position pos, PieceKind kind) const {
        return {weights[getIndex(kind, RoundPhase::Early)][pos], weights[getIndex(kind, RoundPhase::Late)][pos]};
    }

 private:
    static size_t getIndex(PieceKind kind, RoundPhase phase) {
        if (phase == RoundPhase::Early) {
            return static_cast<size_t>(kind);
        }
        if (kind == PieceKind::Pawn) {
            return 0;
        }
        if (kind == PieceKind::King) {
            return 7;
        }
        return static_cast<size_t>(kind);
    }
};

using PositionToNumMap = std::array<uint8_t, BOARD_SIZE>;
}
