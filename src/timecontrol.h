#include <chrono>
#include "types.h"
#include "helpers.h"
#include "uci.h"

#pragma once

struct timeMan{
    bool forceStop;
    timePoint_t startTime;

    timePoint_t averageTime;
    timePoint_t maximumTime;
    timePoint_t optimalTime;

    move_t lastBestMove; 
    score_t lastScore;
    int stability;

    void init(color_t col, uciSearchLims uci);
    void update(depth_t depthSearched, move_t bestMove, score_t score, double timeSpentOnNonBest);

    inline timePoint_t getTime(){
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    inline timePoint_t timeSpent(){
        return getTime() - startTime;
    }
    inline bool stopAfterSearch(){
        return forceStop or timeSpent() >= optimalTime;
    }
    inline bool stopDuringSearch(){
        return forceStop or timeSpent() >= maximumTime;
    }
};