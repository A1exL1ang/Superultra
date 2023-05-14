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

const uint64 moveLag = 30;

uint64 findOptimalTime(color_t col, uciParams uci);