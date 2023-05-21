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
    searchDriver(20000, board);
    board.makeMove(stringToMove("e2a6"));
    searchDriver(2000, board);
}
/*
.\cutechess-cli `
-engine conf="E17_AggressiveLMR" `
-engine conf="E16_Counter" `
-each tc=6+0.06 -openings file="C:\Program Files\Cute Chess\Chess Openings\openings-6ply-1000.pgn" `
-games 2 `
-rounds 10000 `
-repeat 2 `
-maxmoves 200 `
-sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 `
-concurrency 8 `
-ratinginterval 10
*/
