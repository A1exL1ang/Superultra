#include <chrono>
#include "types.h"
#include "helpers.h"
#include "uci.h"

#pragma once

struct timer{
    uint64 startTime;
    uint64 allottedTime;

    // Time is in milliseconds
    uint64 getTime();
    void beginTimer(uint64 newAllotedTime = 0);
    uint64 timeElapsed();
    bool outOfTime();
};

struct timeMan{
    bool forceStop;
    timePoint_t startTime;

    timePoint_t averageTime;
    timePoint_t maximumTime;
    timePoint_t optimalTime;

    move_t lastBestMove; 
    score_t lastScore;
    int stability;

    void init(color_t col, uciParams uci);
    void update(depth_t depthSearched, move_t bestMove, score_t score, double timeSpentOnNonBest);
    timePoint_t timeSpent();
    bool stopAfterSearch();
    bool stopDuringSearch();
};

inline timePoint_t getTime(){
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64 findOptimalTime(color_t col, uciParams uci);