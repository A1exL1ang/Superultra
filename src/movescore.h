#pragma once

#include "search.h"
#include "movepick.h"
#include "board.h"

const Movescore mvvlva[7][7] = {
    {},                                      // Victim: None
    {0, 2050, 2040, 2030, 2020, 2010, 2000}, // Victim: Pawn
    {0, 3050, 3040, 3030, 3020, 3010, 3000}, // Victim: Knight
    {0, 4050, 4040, 4030, 4020, 4010, 4000}, // Victim: Bishop
    {0, 5050, 5040, 5030, 5020, 5010, 5000}, // Victim: Rook
    {0, 6050, 6040, 6030, 6020, 6010, 6000}, // Victim: Queen
    {}                                       // Victim: King
};

const Movescore maximumHist = 16384;

const Movescore ttMoveScore            = 2000000000;
const Movescore promoScore             = 1900000000;
const Movescore goodCaptScore          = 1800000000;
const Movescore primaryKillerScore     = 1700000000;
const Movescore secondaryKillerScore   = 1600000000;
const Movescore counterScore           = 1500000000;
const Movescore nonspecialMoveScore    = 1400000000;
const Movescore okayThresholdScore     = 1350000000;
const Movescore badCaptScore           = 1300000000;

const Movescore promoScoreBonus[7] = { 0, 0, 1, 2, 3, 4, 0 };

Movescore getQuietHistory(Move move, Depth ply, position &board, searchData &sd, searchStack *ss);
void updateAllHistory(Move bestMove, moveList &quiets, Depth depth, Depth ply, position &board, searchData &sd, searchStack *ss);
void scoreMoves(moveList &moves, Move ttMove, Depth ply, position &board, searchData &sd, searchStack *ss);