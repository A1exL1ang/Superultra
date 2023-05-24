#include <chrono>
#include <algorithm>
#include <math.h>
#include "types.h"
#include "helpers.h"
#include "timecontrol.h"
#include "uci.h"

uint64 timer::getTime(){
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void timer::beginTimer(uint64 newAllottedTime){
    startTime = getTime();
    allottedTime = newAllottedTime;
}

uint64 timer::timeElapsed(){
    return getTime() - startTime;
}

bool timer::outOfTime(){
    return timeElapsed() >= allottedTime;
}

uint64 findOptimalTime(color_t col, uciParams uci){
    return std::min(uci.timeLeft[col], uci.timeLeft[col] / uci.movesToGo + uci.timeIncr[col] * 4 / 5 - moveLag);
}






/*
void timeMan::init(color_t col, uciParams uci){
    lastBestMove = nullOrNoMove;
    lastScore = noScore;
    stability = 0;
    startTime = getTime();

    // X base time, Y increment, Z moves till reset
    if (uci.movesToGo > 0){
        timePoint_t timeLeft = uci.timeLeft[col] + (uci.timeIncr[col] * uci.movesToGo) - (moveLag * uci.movesToGo);

        // Don't use more than 40% of time available
        optimalTime = averageTime = timeLeft / uci.movesToGo;
        maximumTime = 2 * timeLeft / 5;
    }
    // X base time, Y increment
    else{
        
    }
}

void timeMan::update(depth_t depthSearched, move_t bestMove, score_t score, double percentTimeSpentOnNonBest){
    // Update stability

    stability = (bestMove != lastBestMove) ? 1 : std::min(stability + 1, 10);

    // Linearly scale time based on how unstable the best move is
    // Stability range is [1, 10] and scale multiplier range is [0.75, 1.20]

    double stabilityScale = 1.2 - 0.05 * stability;

    // Logistically scale time based on score fluctuations
    // Range is [0.5, 1.5] (note that if diff is negative, then we scale up and vice versa)

    score_t diff = score - lastScore;
    double scoreChangeScale = abs(score) < foundMate ? 1.0 : (1.0 / (1.0 + exp(0.035 * diff)) + 0.5);
    
    // Linearly scale time spent on non-best move nodes 
    // Note that the larger the value is the more complex our position is so the higher our scale should be
    // Range is [0.75, 1.25]

    double complexityScale = 0.75 + (percentTimeSpentOnNonBest / 2);

    // Update info    

    lastBestMove = bestMove;
    lastScore = score;
    optimalTime = averageTime * stabilityScale * scoreChangeScale * complexityScale;
}

bool timeMan::stopAfterSearch(){
    return getTime() - startTime >= optimalTime;
}

bool timeMan::stopDuringSearch(){
    return getTime() - startTime >= maximumTime;
}
*/