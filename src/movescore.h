#pragma once

#include "search.h"
#include "movepick.h"
#include "board.h"

const Movescore MVV_LVA[7][7] = {
    {},                                      // Victim: None
    {0, 2050, 2040, 2030, 2020, 2010, 2000}, // Victim: Pawn
    {0, 3050, 3040, 3030, 3020, 3010, 3000}, // Victim: Knight
    {0, 4050, 4040, 4030, 4020, 4010, 4000}, // Victim: Bishop
    {0, 5050, 5040, 5030, 5020, 5010, 5000}, // Victim: Rook
    {0, 6050, 6040, 6030, 6020, 6010, 6000}, // Victim: Queen
    {}                                       // Victim: King
};

const Movescore MAXIMUM_HIST = 16384;

const Movescore TTMOVE_SCORE             = 2000000000;
const Movescore PROMO_SCORE              = 1900000000;
const Movescore GOOD_CAPT_SCORE          = 1800000000;
const Movescore PRIMARY_KILLER_SCORE     = 1700000000;
const Movescore SECONDARY_KILLER_SCORE   = 1600000000;
const Movescore COUNTER_SCORE            = 1500000000;
const Movescore NONSPECIAL_MOVE_SCORE    = 1400000000;
const Movescore OKAY_THRESHOLD_SCORE     = 1350000000;
const Movescore BAD_CAPT_SCORE           = 1300000000;

const Movescore PROMO_SCORE_BONUS[7] = { 0, 0, 1, 2, 3, 4, 0 };

Movescore getQuietHistory(Move move, Depth ply, Position &board, SearchData &sd, SearchStack *ss);
void updateAllHistory(Move bestMove, moveList &quiets, Depth depth, Depth ply, Position &board, SearchData &sd, SearchStack *ss);
void scoreMoves(moveList &moves, Move ttMove, Depth ply, Position &board, SearchData &sd, SearchStack *ss);