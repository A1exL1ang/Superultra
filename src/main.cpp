#include "board.h"
#include "uci.h"
#include "attacks.h"
#include "search.h"
#include <iostream>
#include <fstream>
#include <math.h>


#include <chrono>
#include <vector>
static position perftBoard;
uint64 tot = 0;
uint64 cnt = 0;

int totLegal = 0;
std::vector<Move> mvs;
static uint64 seed = 1928777382391231823ULL;

static inline uint64 genRand(){
    return seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

// NN eval: 22.834187ns
// NN average: 35.858618ns
// Movegen is 82ns
// Movescore is 133ns

static uint64 perft(Depth depth, Depth depthLim){
    moveList moves;
    /*
    TimePoint st = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        perftBoard.genAllMoves(false, moves);
        TimePoint en = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        tot += en - st;
        cnt++;
    */

    perftBoard.genAllMoves(false, moves);

    // Return number of leaves without directly exploring them
    if (depth + 1 >= depthLim){
        return moves.sz;
    }
    
    if (mvs.size()){
        Move test = mvs[genRand() % (mvs.size())];
        bool answer = false;
        for (int i = 0; i < moves.sz; i++)
            answer |= moves.moves[i].move == test;
        bool got = perftBoard.isLegal(test);
        if (got != answer){
            std::cout<<perftBoard.getFen()<<"\n"<<moveToString(test)<<" "<<perftBoard.isLegal(test)<<std::endl;
        }
    }

    uint64 leaves = 0;
    for (int i = 0; i < moves.sz; i++){
        Move move = moves.moves[i].move;
        mvs.push_back(move);
        
        /*
        TimePoint st = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        totLegal += perftBoard.isLegal(move);
        
        TimePoint en = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        tot += en - st;
        cnt++;
        */
        
        /*
        if (!perftBoard.isLegal(move)){
            std::cout<<perftBoard.getFen()<<"\n"<<moveToString(move)<<" "<<perftBoard.isLegal(move)<<std::endl;
        }
        */

        
        

        perftBoard.makeMove(move);

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



    setThreadCount(1);
    
    /*
    importCheckpoint("checkpoint_886189998_336.bin");
    exportNetworkQuantized();
    return 0;
    */

    // Begin the UCI Loop
    if (false){
        doLoop();
        return 0;
    }

    /*
    #include <chrono>
    extern uint64 cnt;
    extern uint64 tot;

    TimePoint st = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    TimePoint en = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    tot += en - st;
    cnt++;
    */

    
    // perftBoard.readFen("r3k2r/p1ppqQb1/bn2p1p1/1N1nN3/1p2P3/7p/PPPBBPPP/R3K2R b KQkq - 0 2");
    // std::cout<<perftBoard.isLegal(stringToMove("e7f7"))<<std::endl;
    // return 0;

    perftBoard.readFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    // ~82ns per position
    std::cout<<perft(0, 8)<<std::endl;
    std::cout<<std::fixed<<(tot + 0.0) / cnt<<std::endl;
    std::cout<<totLegal<<std::endl;
    return 0;
    


    /*
    CHANGES:
    QS Eval snapping: abs(tte.score) < foundMate
    fmr dampening
    */

    // For a midgame position with a lot of positions (kiwipete):
    // Movegen is 82ns
    // Movescore is 133ns
    position board;

    
    setThreadCount(1);

    board.readFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout<<board.eval()<<std::endl;
    // return 0;
    // 6203035
    uciSearchLims lims;
    lims.movesToGo = 50;
    lims.timeIncr[board.getTurn()] = 0;
    lims.timeLeft[board.getTurn()] = 2000 * 50;

    beginSearch(board, lims);
    // beginSearch(board, lims);
}
/*
.\cutechess-cli `
-engine conf="E104_BiasBuckets" `
-engine conf="E102_BigNet" `
-each tc=6+0.06 timemargin=200 `
-openings file="C:\Program Files\Cute Chess\Chess Openings\openings-8ply-10k.pgn" `
-games 2 `
-rounds 25000 `
-repeat 2 `
-maxmoves 200 `
-draw movenumber=40 movecount=4 score=5 `
-recover `
-concurrency 8 `
-ratinginterval 10 `
-sprt elo0=0 elo1=5 alpha=0.05 beta=0.05
*/



/*
Score of E99_NewNetRetrain vs E88_SIMD2: 5664 - 5396 - 13044  [0.506] 24104
...      E99_NewNetRetrain playing White: 3626 - 2077 - 6350  [0.564] 12053
...      E99_NewNetRetrain playing Black: 2038 - 3319 - 6694  [0.447] 12051
...      White vs Black: 6945 - 4115 - 13044  [0.559] 24104
Elo difference: 3.9 +/- 3.0, LOS: 99.5 %, DrawRatio: 54.1 %
SPRT: llr 2.97 (100.7%), lbound -2.94, ubound 2.94 - H1 was accepted


Score of E100_NewNetQ vs E88_SIMD2: 354 - 255 - 790  [0.535] 1399
...      E100_NewNetQ playing White: 225 - 92 - 382  [0.595] 699
...      E100_NewNetQ playing Black: 129 - 163 - 408  [0.476] 700
...      White vs Black: 388 - 221 - 790  [0.560] 1399
Elo difference: 24.6 +/- 12.0, LOS: 100.0 %, DrawRatio: 56.5 %
SPRT: llr 2.97 (100.9%), lbound -2.94, ubound 2.94 - H1 was accepted

Score of E101_HistoryDecay vs E100_NewNetQ: 334 - 259 - 1412  [0.519] 2005
...      E101_HistoryDecay playing White: 234 - 89 - 680  [0.572] 1003
...      E101_HistoryDecay playing Black: 100 - 170 - 732  [0.465] 1002
...      White vs Black: 404 - 189 - 1412  [0.554] 2005
Elo difference: 13.0 +/- 8.3, LOS: 99.9 %, DrawRatio: 70.4 %
SPRT: llr 2.97 (100.8%), lbound -2.94, ubound 2.94 - H1 was accepted

Score of E101_HistoryDecay vs E100_NewNetQ: 180 - 161 - 1299  [0.506] 1640
...      E101_HistoryDecay playing White: 128 - 43 - 649  [0.552] 820
...      E101_HistoryDecay playing Black: 52 - 118 - 650  [0.460] 820
...      White vs Black: 246 - 95 - 1299  [0.546] 1640
Elo difference: 4.0 +/- 7.7, LOS: 84.8 %, DrawRatio: 79.2 %
SPRT: llr 0.499 (16.9%), lbound -2.94, ubound 2.94

Score of E102_BigNet vs E101_HistoryDecay: 270 - 177 - 616  [0.544] 1063
...      E102_BigNet playing White: 169 - 65 - 298  [0.598] 532
...      E102_BigNet playing Black: 101 - 112 - 318  [0.490] 531
...      White vs Black: 281 - 166 - 616  [0.554] 1063
Elo difference: 30.5 +/- 13.5, LOS: 100.0 %, DrawRatio: 57.9 %
SPRT: llr 2.97 (100.9%), lbound -2.94, ubound 2.94 - H1 was accepted

Score of E102_BigNet vs V1.0: 131 - 48 - 318 [0.584]
...      E102_BigNet playing White: 80 - 17 - 150  [0.628] 247
...      E102_BigNet playing Black: 51 - 31 - 168  [0.540] 250
...      White vs Black: 111 - 68 - 318  [0.543] 497
Elo difference: 58.6 +/- 18.1, LOS: 100.0 %, DrawRatio: 64.0 %
504 of 40000 games finished.

Score of E104_BiasBuckets vs E102_BigNet: 3692 - 3472 - 13419  [0.505] 20583
...      E104_BiasBuckets playing White: 2486 - 1114 - 6691  [0.567] 10291
...      E104_BiasBuckets playing Black: 1206 - 2358 - 6728  [0.444] 10292
...      White vs Black: 4844 - 2320 - 13419  [0.561] 20583
Elo difference: 3.7 +/- 2.8, LOS: 99.5 %, DrawRatio: 65.2 %
SPRT: llr 2.97 (101.0%), lbound -2.94, ubound 2.94 - H1 was accepted
*/