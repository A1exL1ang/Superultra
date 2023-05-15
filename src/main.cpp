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
    if (false){
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

    searchDriver(2000, board);
    board.makeMove(stringToMove("e2a6"));
    searchDriver(2000, board);
}
// info depth 14 seldepth 24 score cp -97 nodes 735311 time 539 nps 1364188 hashfull 6 pv e2a6 b4c3 d2c3 e6d5 e5g4 h3g2 f3g2 d5e4 e1c1 e8f8 g4f6 g7f6 c3f6 e7f6 
// info depth 22 seldepth 34 score cp -133 nodes 27088550 time 8382 nps 3231748 hashfull 394 pv e2a6 h3g2 f3g2 b4c3 d2c3 e6d5 e1g1 f6e4 e5c6 d7c6 c3g7 h8h5 g1h1 e8d7 a1e1 a8g8 a6e2 h5h4 g7d4 e7e6 f2f3 c6c5
// info depth 22 seldepth 34 score cp -133 nodes 27088550 time 8717 nps 3107550 hashfull 394 pv e2a6 h3g2 f3g2 b4c3 d2c3 e6d5 e1g1 f6e4 e5c6 d7c6 c3g7 h8h5 g1h1 e8d7 a1e1 a8g8 a6e2 h5h4 g7d4 e7e6 f2f3 c6c5