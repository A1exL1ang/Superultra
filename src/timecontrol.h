#pragma once

#include "types.h"
#include "helpers.h"
#include "uci.h"
#include <chrono>

struct timeMan{
    bool forceStop;
    TimePoint startTime;

    TimePoint averageTime;
    TimePoint maximumTime;
    TimePoint optimalTime;

    Move lastBestMove; 
    Score lastScore;
    int stability;

    void init(Color col, uciSearchLims uci);
    void update(Depth depthSearched, Move bestMove, Score score, double timeSpentOnNonBest);

    inline TimePoint getTime(){
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    inline TimePoint timeSpent(){
        return getTime() - startTime;
    }
    inline bool stopAfterSearch(){
        return forceStop or timeSpent() >= optimalTime;
    }
    inline bool stopDuringSearch(){
        return forceStop or timeSpent() >= maximumTime;
    }
};