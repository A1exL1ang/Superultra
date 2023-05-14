#include <chrono>
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