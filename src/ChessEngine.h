#pragma once
#include "ChessEngineMove.h"
#include "ChessEngineRound.h"
#include "ChessEngineBoard.h"
#include "ChessEngineMaps.h"
#include "ChessEngineSearch.h"
#include "ChessEngineModes.h"
#include "ChessEngineOptions.h"
#include "ChessEngineHistory.h"
#include "ChessEngineSupervisor.h"
#include "ChessEngineTask.h"
#include "ChessEngineTime.h"

namespace ChessEngine {
enum class QuiescenceType : uint8_t {
    Skip,
    MaybeOpponentInCheck,
    Tactical
};

class Engine {
 public:
    Engine();

    bool begin(const String& fen);
    bool undo(size_t num = 1);
    Move find(const Move& uci) const;
    Move find(const String& uci) const;
    Move make(const Move& move);
    Move make(const String& uci);
    Move solve();
    void stop();

    PieceColor getTurn() const;
    CastlingAbilities getCastlingAbilities() const;
    Position getEnPassant() const;
    HalfMove getHalfMove() const;
    FullMove getFullMove() const;
    RoundStatus getRoundStatus() const;
    Position getCheck() const;
    Move getLastMove() const;
    const Board& getBoard() const;

    bool isWhiteTurn() const;
    bool isBlackTurn() const;

    bool isNormal() const;
    bool isCheck() const;
    bool isCheckmate() const;
    bool isStalemate() const;
    bool isDeadPosition() const;
    bool isFiftyMove() const;
    bool isThreefoldRepetition() const;
    bool isStopped() const;
    bool isInvalid() const;
    bool isDraw() const;
    bool isEnd() const;

    const Options& getOptions() const;
    void setOptions(const Options& opt);
    Milliseconds getTimeLimit() const;
    void setTimeLimit(Milliseconds limit);

    void setCyclicPull(CyclicPull& pull, Milliseconds period);

#ifdef CHESS_ENGINE_MULTI_TASK
    Task& setMultiTask();
#endif

    const Moves& solveMoves();
    const Moves& createMoves(Position src);
    const Moves& createMoves(const String& srcUci);
    String createFen() const;

 private:
    bool applyFen(const String& fen);
    void applyMove(const Move& move);
    void updateRound();

    bool isAutoPromotionNeeded(const Move& move) const;
    bool isWhiteKingInSlidingCheck() const;
    bool isBlackKingInSlidingCheck() const;
    bool isOpponentKingInSlidingCheck(const Round& round) const;
    bool isWhiteKingInCheck() const;
    bool isBlackKingInCheck() const;
    bool isKingInCheck(PieceColor turn) const;
    bool isKingInCheck(const Round& round) const;
    bool isOpponentKingInCheck(const Round& round) const;

    void setKingsPositions();
    void setCheckPosition(Round& round);
    void setRoundCheckStatus(Round& round) const;
    void setRoundWeights(Round& round) const;
    void setRoundPhase(Round& round) const;

    void diveWhiteCastling(CastlingAbilities& castling, const Move& move) const;
    void diveBlackCastling(CastlingAbilities& castling, const Move& move) const;
    void diveWhiteEnPassant(Position& enPassant, const Move& move) const;
    void diveBlackEnPassant(Position& enPassant, const Move& move) const;
    void diveWhiteMaterialWeights(MaterialWeights& weights, const Move& move) const;
    void diveBlackMaterialWeights(MaterialWeights& weights, const Move& move) const;
    void diveWhitePositionalWeights(PstWeights& weights, const Move& move) const;
    void diveBlackPositionalWeights(PstWeights& weights, const Move& move) const;
    void diveRound(const Round& round, Round& nextRound, const Move& move);

    void redoMove(const Move& move);
    void redoNormalMove(Position& kingPos, Piece srcKing, const Move& move);
    void redoEnPassantMove(const EnPassantDstToEnPassantRemovesMap& map, const Move& move);
    void redoCastlingMove(Position& kingPos, const CastlingMove& castle);
    void redoPromoMove(Piece promPiece, const Move& move);
    void redoBoard(const Round& round, const Move& move);

    void undoMove(const Move& move);
    void undoNormalMove(Position& kingPos, Piece srcKing, const Move& move);
    void undoEnPassantMove(const EnPassantDstToEnPassantRemovesMap& map, Piece beatPawn, const Move& move);
    void undoCastlingMove(Position& kingPos, const CastlingMove& castle);
    void undoPromoMove(const Move& move);
    void undoBoard(const Round& round, const Move& move);

    void addMove(Moves& moves, Position src, Position dst, MoveType type) const;
    void addPawnMoves(Moves& moves, Position src) const;
    void addKingMoves(Moves& moves, Position src) const;
    void addKnightMoves(Moves& moves, Position src) const;
    void addRookMoves(Moves& moves, Position src) const;
    void addBishopMoves(Moves& moves, Position src) const;
    void addQueenMoves(Moves& moves, Position src) const;
    void addEnPassantMoves(Moves& moves, const Round& round, const EnPassantRemoves& removes, Piece srcPawn,
                           Piece beatPawn) const;
    void addEnPassantMoves(Moves& moves, const Round& round) const;
    void addCastlingMove(Moves& moves, Position& kingPos, const CastlingMove& castle);
    void addCastlingMoves(Moves& moves, const Round& round);
    void addMoves(Moves& moves, const Round& round);
#ifdef CHESS_ENGINE_MULTI_TASK
    void addMultiTaskMoves(Moves& moves, const Round& round, Depth depth);
    void addTaskMoves(Depth depth);
#endif
    void addWeights(Depth depth);

    void sortMoves(Moves& moves) const;
    void sortMovesUntilQuiet(Moves& moves) const;
    void generateMoves(Depth depth);
    void generateLegalMoves(Depth depth);

    Weight evaluateMaterial(Weight own, Weight opponent) const;
    Weight evaluatePositional(Weight own, Weight opponent, Weight pst) const;
    Weight evaluate(const Round& round) const;

    bool isDrawByDeadPosition() const;
    bool isDrawByFiftyMove(const Round& round) const;
    bool isDrawByThreefoldRepetition(Depth depth) const;
    bool isDrawByThreefoldRepetition() const;
    QuiescenceType getQuiescenceType(const Move& move) const;
    Weight quiescence(Depth depth, Weight alpha, Weight beta, Depth depthLeft);
    Weight negamax(Depth depth, Weight alpha, Weight beta, Depth depthLeft);

#ifdef CHESS_ENGINE_MULTI_TASK
    friend Task;
    Task task;
#endif

    Search search;
    Round& rootRound;
    Round& rootNextRound;
    Board board;
    KingPositions kingPositions;

    Moves moves;
    Moves bestMoves;

    Modes modes;
    Options options;

    Depth maxReachedDepth;
    Supervisor supervisor;

    History history;
    Round oldestRound;
    Board oldestBoard;
    Move lastMove;
};
}
