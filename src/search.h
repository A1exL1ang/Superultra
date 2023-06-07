#include "board.h"
#include "tt.h"
#include "helpers.h"
#include "types.h"
#include "uci.h"

#pragma once

struct searchStack{
    score_t staticEval;
    move_t excludedMove;
    move_t move;
    int dextension;
    
    move_t *counter;
    movescore_t (*contHist)[14][64];
};

struct searchResultData{
    depth_t depthSearched;
    depth_t selDepth;
    score_t score;
    std::vector<std::string> pvMoves; 
};

struct searchData{
    int threadId;
    bool stopped = false;

    depth_t selDepth = 0;
    searchResultData result;
    
    move_t pvTable[maximumPly + 5][maximumPly + 5] = {};
    depth_t pvLength[maximumPly + 5] = {};
    
    move_t killers[maximumPly + 5][2] = {};
    move_t counter[14][64] = {};

    movescore_t history[2][64][64] = {};
    movescore_t contHist[2][14][64][14][64] = {};

    uint64 nodes = 0;
    uint64 moveNodeStat[64][64] = {};
};

void initLMR();
void beginSearch(position board, uciParams uci);