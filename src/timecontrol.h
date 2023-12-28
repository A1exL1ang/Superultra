#pragma once

#include "types.h"
#include "helpers.h"
#include "uci.h"
#include "search.h"
#include <chrono>

struct timeMan{
    bool forceStop;
    TimePoint startTime;
    
    // Time variables
    TimePoint averageTime;
    TimePoint maximumTime;
    TimePoint optimalTime;

    // Statistics to update time
    Move lastBestMove; 
    Score lastScore;
    int stability;

    // Special UCI time specifications
    bool infinite;
    TimePoint fixedMoveTime;

    void init(Color col, uciSearchLims uci);
    void update(Depth depthSearched, Move bestMove, Score score, double timeSpentOnNonBest);

    inline TimePoint timeSpent(){
        return getTime() - startTime;
    }
    inline bool stopAfterSearch(){
        if (pondering){
            return false;
        }
        else if (infinite){
            return false;
        }
        else if (fixedMoveTime){
            return timeSpent() >= fixedMoveTime;
        }
        return forceStop or timeSpent() >= optimalTime;
    }
    inline bool stopDuringSearch(){
        if (pondering){
            return false;
        }
        else if (infinite){
            return false;
        }
        else if (fixedMoveTime){
            return timeSpent() >= fixedMoveTime;
        }
        return forceStop or timeSpent() >= maximumTime;
    }
};