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

    // Default settings
    globalTT.setSize(16);
    threadCount = 4;

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

    // TODO: TT Prefetch
    // SIMD
    // EGTB
    // Thinking on opponent time
    // Other UCI stuffs
    // Make neat and then release

    // testPerft();

      
    position board;
    board.readFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    uciParams uci;
    uci.timeIncr[board.getTurn()] = 0;
    uci.movesToGo = 1;
    uci.timeLeft[board.getTurn()] = 2000;

    beginSearch(board, uci);
    board.makeMove(stringToMove("e2e4"));

    uci.timeIncr[board.getTurn()] = 0;
    uci.movesToGo = 1;
    uci.timeLeft[board.getTurn()] = 10000;

    beginSearch(board, uci);
}
/*
.\cutechess-cli `
-engine conf="E74_Revert" `
-engine conf="E67_AWupdate" `
-each tc=6+0.06 timemargin=200 `
-openings file="C:\Program Files\Cute Chess\Chess Openings\openings-8ply-10k.pgn" `
-games 2 `
-rounds 25000 `
-repeat 2 `
-recover `
-sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 `
-concurrency 8 `
-ratinginterval 10
*/



/*
Score of E73_LazySMP2 vs E67_AWupdate: 297 - 197 - 590  [0.546] 1084
...      E73_LazySMP2 playing White: 192 - 60 - 290  [0.622] 542
...      E73_LazySMP2 playing Black: 105 - 137 - 300  [0.470] 542
...      White vs Black: 329 - 165 - 590  [0.576] 1084
Elo difference: 32.1 +/- 13.9, LOS: 100.0 %, DrawRatio: 54.4 %
SPRT: llr 2.96 (100.4%), lbound -2.94, ubound 2.94 - H1 was accepted

Player: E73_LazySMP2
   "Draw by 3-fold repetition": 429
   "Draw by fifty moves rule": 72
   "Draw by insufficient mating material": 85
   "Draw by stalemate": 4
   "Loss: Black disconnects": 34
   "Loss: Black mates": 32
   "Loss: White disconnects": 28
   "Loss: White mates": 103
   "No result": 4
   "Win: Black disconnects": 1
   "Win: Black mates": 105
   "Win: White mates": 191
Player: E67_AWupdate
   "Draw by 3-fold repetition": 429
   "Draw by fifty moves rule": 72
   "Draw by insufficient mating material": 85
   "Draw by stalemate": 4
   "Loss: Black disconnects": 1
   "Loss: Black mates": 105
   "Loss: White mates": 191
   "No result": 4
   "Win: Black disconnects": 34
   "Win: Black mates": 32
   "Win: White disconnects": 28
   "Win: White mates": 103
Finished match
*/
