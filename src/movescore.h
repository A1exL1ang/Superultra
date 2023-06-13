#pragma once

#include "search.h"
#include "movepick.h"
#include "board.h"

Movescore getQuietHistory(Move move, Depth ply, position &board, searchData &sd, searchStack *ss);
void updateAllHistory(Move bestMove, moveList &quiets, Depth depth, Depth ply, position &board, searchData &sd, searchStack *ss);
void scoreMoves(moveList &moves, Move ttMove, Depth ply, position &board, searchData &sd, searchStack *ss);