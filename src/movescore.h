#include "search.h"
#include "movepick.h"
#include "board.h"

#pragma once

void updateAllHistory(move_t bestMove, moveList &quiets, depth_t depth, depth_t ply, position &board, searchData &sd, searchStack *ss);
void scoreMoves(moveList &moves, move_t ttMove, depth_t ply, position &board, searchData &sd, searchStack *ss);