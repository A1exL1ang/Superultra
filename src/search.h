#include "board.h"
#include "tt.h"
#include "helpers.h"
#include "types.h"
#include "timecontrol.h"

#pragma once

struct searchStack{
    score_t staticEval;
    move_t excludedMove;
    move_t move;

    move_t *counter;
    movescore_t (*contHist)[14][64];
};

struct searchData{
    move_t pvTable[maximumPly + 5][maximumPly + 5] = {};
    depth_t pvLength[maximumPly + 5] = {};
    
    move_t killers[maximumPly + 5][2] = {};
    move_t counter[14][64];

    movescore_t history[2][64][64] = {};
    movescore_t chist[14][64][7] = {};
    movescore_t contHist[2][14][64][14][64] = {};

    bool stopped = false;

    uint64 nodes = 0;
    depth_t selDepth = 0;

    timer T;
};

void initLMR();
void searchDriver(uint64 timeAlloted, position boardToSearch);