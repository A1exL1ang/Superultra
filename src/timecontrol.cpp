#include <chrono>
#include <algorithm>
#include <math.h>
#include "types.h"
#include "helpers.h"
#include "timecontrol.h"
#include "uci.h"

const static TimePoint MOVE_LAG = 30;

void timeMan::init(Color col, uciSearchLims uci){
    forceStop = false;
    lastBestMove = NULL_OR_NO_MOVE;
    lastScore = NO_SCORE;
    stability = 0;
    startTime = getTime();

    // If no time is given assume infinite
    infinite = uci.infinite or (!uci.timeLeft[col] and !uci.moveTime);
    fixedMoveTime = uci.moveTime;

    if (infinite or fixedMoveTime)
        return;
    
    // X base time, Y increment, Z moves till reset (Z is 50 if there is no time reset)

    int mtg = (uci.movesToGo == 0) ? 50 : uci.movesToGo;

    TimePoint totalTime = uci.timeLeft[col] + (uci.timeIncr[col] * mtg) - MOVE_LAG * mtg;
    
    optimalTime = averageTime = std::clamp(static_cast<double>(totalTime) / mtg, (0.95 * uci.timeLeft[col]) / mtg, 0.8 * uci.timeLeft[col]);
    
    maximumTime = std::min(5.5 * averageTime, 0.8 * uci.timeLeft[col]);
}

void timeMan::update(Depth depthSearched, Move bestMove, Score score, double percentTimeSpentOnNonBest){
    // Don't do anything if we infinitely searching or searching for a fixed time
    if (infinite or fixedMoveTime)
        return;

    // Update stability

    stability = (bestMove != lastBestMove) ? 1 : std::min(stability + 1, 10);

    // Linearly scale time based on how unstable the best move is
    // Stability range is [1, 10] and scale multiplier range is [0.75, 1.20]

    double stabilityScale = 1.2 - 0.05 * stability;

    // Linearly scale based on score fluctuation. Note that we should be more inclined
    // to increase time if our score suddenly decreases so we handle the score
    // increase and decrease cases seperately

    double absdiff = abs(score - lastScore);
    double scoreChangeScale = 1;

    // Score increase
    if (score >= lastScore){
        scoreChangeScale = 0.75 + 0.5 * std::min(absdiff, 30.0) / 30.0;
    }
    // Score decrease
    else{
        scoreChangeScale = 0.75 + 0.5 * std::min(absdiff, 15.0) / 15.0;
    }
    
    // Linearly scale time based on time spent on non-best move nodes 
    // Note that the larger the value is the more complex our position is so the higher our scale should be
    // Range is [0.7, 1.7] and 30% time is average which maps to 1.0

    double complexityScale = 0.70 + percentTimeSpentOnNonBest;

    // Update info    

    lastBestMove = bestMove;
    lastScore = score;

    // First few depths are unstable

    if (depthSearched >= 10){
        optimalTime = averageTime * stabilityScale * scoreChangeScale * complexityScale;
    }
}