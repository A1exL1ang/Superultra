#include <chrono>
#include <algorithm>
#include <math.h>
#include "types.h"
#include "helpers.h"
#include "timecontrol.h"
#include "uci.h"

const static timePoint_t moveLag = 30;

timePoint_t timer::getTime(){
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







void timeMan::init(color_t col, uciParams uci){
    forceStop = false;
    lastBestMove = nullOrNoMove;
    lastScore = noScore;
    stability = 0;
    startTime = getTime();

    // X base time, Y increment, Z moves till reset (Z is 50 if there is no time reset)

    int mtg = (uci.movesToGo == 0) ? 50 : uci.movesToGo;

    timePoint_t totalTime = uci.timeLeft[col] + (uci.timeIncr[col] * mtg) - moveLag * mtg;
    
    optimalTime = averageTime = std::clamp(static_cast<double>(totalTime) / mtg, (0.95 * uci.timeLeft[col]) / mtg, 0.8 * uci.timeLeft[col]);

    maximumTime = std::min(5.5 * averageTime, 0.8 * uci.timeLeft[col]);
}

void timeMan::update(depth_t depthSearched, move_t bestMove, score_t score, double percentTimeSpentOnNonBest){
    // Update stability

    stability = (bestMove != lastBestMove) ? 1 : std::min(stability + 1, 10);

    // Linearly scale time based on how unstable the best move is
    // Stability range is [1, 10] and scale multiplier range is [0.75, 1.20]

    double stabilityScale = 1.2 - 0.05 * stability;

    // Exponentially scale time based on score fluctuations (formula from Stash Bot)
    // Range is [0.5, 2] (we scale up when score falls and down otherwise)

    int diff = score - lastScore;
    double scoreChangeScale = pow(2, -1 * std::clamp(diff / 100.0, -1.0, 1.0));
    
    // Linearly scale time based on time spent on non-best move nodes 
    // Note that the larger the value is the more complex our position is so the higher our scale should be
    // Range is [0.7, 1.7] and 30% time is average which maps to 1.0

    double complexityScale = 0.70 + percentTimeSpentOnNonBest;

    // Update info    

    lastBestMove = bestMove;
    lastScore = score;

    // First few depths are unstable

    if (depthSearched >= 10)
        optimalTime = averageTime * stabilityScale * scoreChangeScale * complexityScale;
}

timePoint_t timeMan::timeSpent(){
    return getTime() - startTime;
}

bool timeMan::stopAfterSearch(){
    return forceStop or timeSpent() >= optimalTime;
}

bool timeMan::stopDuringSearch(){
    return forceStop or timeSpent() >= maximumTime;
}