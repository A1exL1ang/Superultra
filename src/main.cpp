#include "board.h"
#include "uci.h"
#include "attacks.h"
#include "search.h"

int main(){
    initLineBB();
    initMagicCache();
    initNNUEWeights();
    initLMR();
    initTT();

    // Default settings
    globalTT.setSize(16);
    threadCount = 1;

    // Begin the UCI Loop
    doLoop();
}