#include "search.h"
#include "movepick.h"
#include "board.h"

#pragma once

void scoreMoves(moveList &moves, move_t ttMove, depth_t ply, position &board, searchData &sd, searchStack *ss);
void updateAllHistory(move_t bestMove, moveList &quiets, moveList &capts, depth_t depth, depth_t ply, position &board, searchData &sd, searchStack *ss);
movescore_t getQuietHistory(move_t move, depth_t ply, position &board, searchData &sd, searchStack *ss);
movescore_t getCaptHistory(move_t move, depth_t ply, position &board, searchData &sd, searchStack *ss);