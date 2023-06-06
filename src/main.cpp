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

    /*
    CHANGES:
    1) QS SEE Pruning -- remove movesSeen > 1 condition
    2) MCP in main search -- remove !pvNode condition
    3) SEE pruning in main search -- remove !root condition
    4) HIstory pruning -- add !inCheck condition
    */

    globalTT.setSize(16);

    // testPerft();

      
    position board;
    board.readFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    board.getStackIndex();
    // 3504426
    // counter: 3064725
    // probcut: 790735
    // 2787455

    uciParams uci;
    uci.timeIncr[board.getTurn()] = 0;
    uci.movesToGo = 1;
    uci.timeLeft[board.getTurn()] = 20000;

    beginSearch(board, uci);
    board.makeMove(stringToMove("e2a6"));

    uci.timeIncr[board.getTurn()] = 0;
    uci.movesToGo = 1;
    uci.timeLeft[board.getTurn()] = 20000;

    searchDriver(uci, board);
}
/*
.\cutechess-cli `
-engine conf="E71_AWupdate1" `
-engine conf="E67_AWupdate" `
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
*/
