#pragma once

#include "board.h"
#include "tt.h"
#include "helpers.h"
#include "types.h"
#include "uci.h"
#include <cstring>

// Global pondering flag. Note that pondering is only ended after stop / ponderhit / quit
// command so we should keep the logic seperate from endSearch

extern bool pondering;

struct SearchStack{
    Score staticEval;
    Move excludedMove;
    Move move;
    int dextension;
    
    Move *counter;
    Movescore (*contHist)[14][64];
};

struct SearchResultData{
    Depth depthSearched;
    Depth selDepth;
    Score score;
    std::vector<std::string> pvMoves; 
};

struct SearchData{
    int threadId;
    bool stopped;

    Depth selDepth;
    SearchResultData result;
    
    Move pvTable[MAX_PLY + 5][MAX_PLY + 5];
    Depth pvLength[MAX_PLY + 5];
    
    Move killers[MAX_PLY + 5][2];
    Move counter[14][64];

    Movescore history[2][64][64];
    Movescore contHist[2][14][64][14][64];

    uint64 nodes;
    uint64 moveNodeStat[64][64];

    inline void resetNonHistory(int id){
        threadId = id;
        stopped = false;
        selDepth = 0;
        result = {};

        memset(pvTable, 0, sizeof(pvTable));
        memset(pvLength, 0, sizeof(pvLength));

        memset(killers, 0, sizeof(killers));
        memset(counter, 0, sizeof(counter));

        nodes = 0;
        memset(moveNodeStat, 0, sizeof(moveNodeStat)); 
    }

    inline void decayHistory(){
        // Decay history
        for (int i = 0; i < 2; i++){
            for (int j = 0; j < 64; j++){
                for (int k = 0; k < 64; k++){
                    history[i][j][k] /= 4;
                }
            }
        }
        // Decay continuation history
        for (int i = 0; i < 2; i++){
            for (int j = 0; j < 14; j++){
                for (int k = 0; k < 64; k++){
                    for (int l = 0; l < 14; l++){
                        for (int m = 0; m < 64; m++){
                            contHist[i][j][k][l][m] /= 4;
                        }
                    }
                }
            }
        }
    }

    inline void clearHistory(){
        memset(history, 0, sizeof(history));
        memset(contHist, 0, sizeof(contHist));
    }
};

// Init
void initLMR();
void setThreadCount(int tds);

// Thread related
void resetAllSearchDataNonHistory();
void decayAllSearchDataHistory();
void clearAllSearchDataHistory();

// Search related
void endSearch();
void beginSearch(Position board, uciSearchLims lims);