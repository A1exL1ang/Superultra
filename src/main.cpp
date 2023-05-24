#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <iomanip>  
#include <chrono>
#include "types.h"
#include "helpers.h"
#include "board.h"
#include "uci.h"
#include "timecontrol.h"
#include "attacks.h"
#include "movepick.h"
#include "search.h"
#include "test.h"


position perftBoard;
uint64 totalTime, nodes;

uint64 getTime(){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64 perft(depth_t depth, depth_t depthLim){
    moveList moves;
    perftBoard.genAllMoves(false, moves);

    // Return number of leaves without directly exploring them
    if (depth + 1 >= depthLim)
        return moves.sz;
    
    uint64 leaves = 0;
    for (int i = 0; i < moves.sz; i++){
        move_t move = moves.moves[i].move;

        uint64 st = getTime();
        perftBoard.makeMove(move);
        totalTime += getTime() - st;
        nodes++;

        uint64 value = perft(depth + 1, depthLim);
        leaves += value;

        perftBoard.undoLastMove();
    }
    return leaves;
}

int main(){
    initLineBB();
    initMagicCache();
    initNNUEWeights();
    initLMR();
    initTT();

    // Recent loss: 0.004985
    if (true){
        doLoop();
        return 0;
    }
    // perftBoard.readFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // std::cout<<perftBoard.eval()<<std::endl;
    // return 0;
    // perft(0, 5);
    // std::cout<<std::setprecision(15)<<totalTime / (nodes + 0.0)<<std::endl;
    // return 0;

    globalTT.setSize(16);

    // testPerft();

      
    position board;
    board.readFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    // 3504426
    // counter: 3064725
    // probcut: 790735
    searchDriver(20000, board);
    board.makeMove(stringToMove("e2a6"));
    searchDriver(2000, board);
}
/*
.\cutechess-cli `
-engine conf="E55_NE" `
-engine conf="E54_DE" `
-each tc=6+0.06 timemargin=200 `
-openings file="C:\Program Files\Cute Chess\Chess Openings\openings-8ply-10k.pgn" `
-games 2 `
-rounds 25000 `
-repeat 2 `
-maxmoves 200 `
-draw movenumber=40 movecount=4 score=5 `
-recover `
-sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 `
-concurrency 8 `
-ratinginterval 10




Score of E53_SE vs E47_TTinQS: 263 - 159 - 444  [0.560] 866
...      E53_SE playing White: 169 - 48 - 216  [0.640] 433
...      E53_SE playing Black: 94 - 111 - 228  [0.480] 433
...      White vs Black: 280 - 142 - 444  [0.580] 866
Elo difference: 41.9 +/- 16.1, LOS: 100.0 %, DrawRatio: 51.3 %
SPRT: llr 2.95 (100.2%), lbound -2.94, ubound 2.94 - H1 was accepted

Score of E54_DE vs E53_SE: 568 - 537 - 1577  [0.506] 2682
...      E54_DE playing White: 356 - 195 - 790  [0.560] 1341
...      E54_DE playing Black: 212 - 342 - 787  [0.452] 1341
...      White vs Black: 698 - 407 - 1577  [0.554] 2682
Elo difference: 4.0 +/- 8.4, LOS: 82.4 %, DrawRatio: 58.8 %
SPRT: llr 0.409 (13.9%), lbound -2.94, ubound 2.94

Score of E55_NE vs E54_DE: 645 - 584 - 1687  [0.510] 2916
...      E55_NE playing White: 397 - 238 - 824  [0.554] 1459
...      E55_NE playing Black: 248 - 346 - 863  [0.466] 1457
...      White vs Black: 743 - 486 - 1687  [0.544] 2916
Elo difference: 7.3 +/- 8.2, LOS: 95.9 %, DrawRatio: 57.9 %
SPRT: llr 1.37 (46.5%), lbound -2.94, ubound 2.94
*/
