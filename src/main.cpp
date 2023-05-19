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

    searchDriver(20000, board);
    board.makeMove(stringToMove("e2a6"));
    searchDriver(2000, board);
}
/*
Score of E5_TT2 vs E3_UpdatedTT: 2637 - 2644 - 6515  [0.500] 11796
...      E5_TT2 playing White: 1685 - 943 - 3270  [0.563] 5898
...      E5_TT2 playing Black: 952 - 1701 - 3245  [0.437] 5898
...      White vs Black: 3386 - 1895 - 6515  [0.563] 11796
Elo difference: -0.2 +/- 4.2, LOS: 46.2 %, DrawRatio: 55.2 %
SPRT: llr -2.95 (-100.3%), lbound -2.94, ubound 2.94 - H0 was accepted

.\cutechess-cli `
-engine conf="E7_UpdateHistBonus" `
-engine conf="E6_ContHist" `
-each tc=6+0.06 -openings file="C:\Program Files\Cute Chess\Chess Openings\openings-6ply-1000.pgn" `
-games 2 `
-rounds 10000 `
-repeat 2 `
-maxmoves 200 `
-sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 `
-concurrency 8 `
-ratinginterval 10
*/