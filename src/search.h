#pragma once

#include "board.h"
#include "tt.h"
#include "helpers.h"
#include "types.h"
#include "uci.h"

struct searchStack{
    Score staticEval;
    Move excludedMove;
    Move move;
    int dextension;
    
    Move *counter;
    Movescore (*contHist)[14][64];
};

struct searchResultData{
    Depth depthSearched;
    Depth selDepth;
    Score score;
    std::vector<std::string> pvMoves; 
};

struct searchData{
    int threadId;
    bool stopped;

    Depth selDepth;
    searchResultData result;
    
    Move pvTable[maximumPly + 5][maximumPly + 5];
    Depth pvLength[maximumPly + 5];
    
    Move killers[maximumPly + 5][2];
    Move counter[14][64];

    Movescore history[2][64][64];
    Movescore contHist[2][14][64][14][64];

    uint64 nodes;
    uint64 moveNodeStat[64][64];
};

void initLMR();
void endSearch();
void beginSearch(position board, uciSearchLims lims);