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
    // doLoop();

    /*
    CHANGES:
    QS Eval snapping: abs(tte.score) < foundMate
    
    */

    position board;

    
    threadCount = 1;
    board.readFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    uciSearchLims lims;
    lims.movesToGo = 1;
    lims.timeLeft[board.getTurn()] = 10000;

    beginSearch(board, lims);
}
/*

*/