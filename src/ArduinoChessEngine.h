#pragma once
#include "ChessEngine.h"
#include "ChessEngineLevels.h"

extern ChessEngine::Engine ArduinoChessEngine;

using ChessEngineDepth = ChessEngine::Depth;
using ChessEngineWeight = ChessEngine::Weight;

using ChessEnginePieceType = ChessEngine::PieceType;
using ChessEnginePieceKind = ChessEngine::PieceKind;
using ChessEnginePieceColor = ChessEngine::PieceColor;
using ChessEnginePiece = ChessEngine::Piece;
using ChessEnginePieces = ChessEngine::Pieces;

using ChessEnginePosition = ChessEngine::Position;
using ChessEnginePositions = ChessEngine::Positions;

using ChessEngineMoveType = ChessEngine::MoveType;
using ChessEngineMove = ChessEngine::Move;

using ChessEngineBoard = ChessEngine::Board;

using ChessEngineCastlingAbilities = ChessEngine::CastlingAbilities;
using ChessEngineHalfMove = ChessEngine::HalfMove;
using ChessEngineFullMove = ChessEngine::FullMove;
using ChessEngineRoundStatus = ChessEngine::RoundStatus;
using ChessEngineMoves = ChessEngine::Moves;

using ChessEngineNullMoveOption = ChessEngine::NullMoveOption;
using ChessEngineFutilityOption = ChessEngine::FutilityOption;
using ChessEngineLazyEvalOption = ChessEngine::LazyEvalOption;
using ChessEngineAspirationOption = ChessEngine::AspirationOption;
using ChessEngineRandomOption = ChessEngine::RandomOption;
using ChessEngineMaxDepthOption = ChessEngine::MaxDepthOption;
using ChessEngineTimeLimitOption = ChessEngine::TimeLimitOption;
using ChessEngineOptions = ChessEngine::Options;

using ChessEngineLevels = ChessEngine::Levels;

using ChessEngineMilliseconds = ChessEngine::Milliseconds;
using ChessEngineCyclicPull = ChessEngine::CyclicPull;

#ifdef CHESS_ENGINE_MULTI_TASK
using ChessEngineTask = ChessEngine::Task;
#endif
