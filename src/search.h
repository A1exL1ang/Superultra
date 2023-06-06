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
    int dextension;
    
    move_t *counter;
    movescore_t (*contHist)[14][64];
};

struct searchData{
    int threadId;

    move_t pvTable[maximumPly + 5][maximumPly + 5] = {};
    depth_t pvLength[maximumPly + 5] = {};
    
    move_t killers[maximumPly + 5][2] = {};
    move_t counter[14][64] = {};

    movescore_t history[2][64][64] = {};
    movescore_t contHist[2][14][64][14][64] = {};

    uint64 nodes = 0;
    uint64 moveNodeStat[64][64] = {};

    depth_t selDepth = 0;

    timer T;
    bool stopped = false;
};

void initLMR();
void searchDriver(uciParams uci, position boardToSearch);
void beginSearch(position board, uciParams uci);