#include "board.h"
#include "uci.h"
#include "attacks.h"
#include "search.h"
#include "movescore.h"
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

/*

info depth 17 seldepth 26 score cp 28 nodes 1485707 time 592 nps 2509597 hashfull 358 pv d2d4 g8f6 c2c4 e7e6 g1f3 d7d5 b1c3 c7c5 c4d5 f6d5 e2e4 d5c3 b2c3 c5d4 c3d4 f8b4 c1d2 
info depth 18 seldepth 33 score cp 33 nodes 2908343 time 1171 nps 2483619 hashfull 653 pv e2e4 c7c5 b1c3 d7d6 g1f3 e7e5 f1c4 f8e7 d2d3 g8f6 c1g5 e8g8 g5f6 e7f6 c3d5 g8h8 d5e3 b8c6 
info depth 19 seldepth 34 score cp 29 nodes 6113537 time 2474 nps 2471104 hashfull 948 pv d2d4 g8f6 c2c4 e7e6 b1c3 d7d5 c1g5 f8e7 c4d5 f6d5 g5e7 d8e7 d1c2 e8g8 g1f3 f8d8 e2e3 d5b4 c2e4 c7c5 a2a3

info depth 20 seldepth 30 score cp 31 nodes 7264762 time 2941 nps 2470158 hashfull 982 pv d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 d1c2 d7d5 a2a3 b4c3 c2c3 d5c4 c3c4 e8g8 c1f4 f6d5 f4g3 b7b6 g1f3 c8a6 c4c2

info depth 21 seldepth 34 score cp 28 nodes 9716873 time 3921 nps 2478155 hashfull 1000 pv d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 d1c2 d7d5 c4d5 e6d5 c1g5 h7h6 g5f6 d8f6 a2a3 b4c3 c2c3 b8d7 e2e3 c7c6 f1d3 e8g8
info depth 22 seldepth 34 score cp 36 nodes 13115003 time 5252 nps 2497139 hashfull 1000 pv d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 d1c2 d7d5 a2a3 b4c3 c2c3 e8g8 g1f3 b7b6 c1g5 d5c4 c3c4 b8d7 c4c6 c8a6 e2e4 a6f1
info depth 23 seldepth 37 score cp 27 nodes 20860498 time 8293 nps 2515431 hashfull 1000 pv d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 d1c2 d7d5 a2a3 b4c3 c2c3 e8g8 g1f3 b7b6 c1g5 d5c4 c3c4 c8b7 e2e3 c7c5 f1e2 b8d7 e1g1 h7h6 g5h4
info depth 24 seldepth 39 score cp 29 nodes 30260804 time 12173 nps 2485893 hashfull 1000 pv d2d4 g8f6 c2c4 e7e6 b1c3 f8b4 d1c2 d7d5 a2a3 b4c3 c2c3 e8g8 g1f3 b7b6 c1g5 d5c4 c3c4 c8b7 e2e3 c7c5 d4c5 b6c5 c4c5 b8d7 c5d6 d8a5 b2b4
*/

/*
Newer:
info depth 21 seldepth 34 score cp 33 nodes 6807524 time 2667 nps 2552492 hashfull 958 pv e2e4 e7e6 d2d4 d7d5 b1c3 g8f6 c1g5 d5e4 c3e4 f8e7 g5f6 g7f6 g1f3 b8d7 e4c3 a7a6 f1d3 b7b5 d4d5 d7b6 d5e6 c8e6
info depth 22 seldepth 40 score cp 29 nodes 14364340 time 5587 nps 2571024 hashfull 1000 pv e2e4 e7e6 d2d4 d7d5 b1c3 g8f6 c1g5 d5e4 c3e4 f8e7 g5f6 g7f6 g1f3 b8d7 e4c3 a7a6 d1e2 b7b5 d4d5 d7b6 d5e6 f7e6 g2g3
info depth 23 seldepth 37 score cp 31 nodes 17589889 time 6839 nps 2571993 hashfull 1000 pv e2e4 e7e6 d2d4 d7d5 b1c3 g8f6 c1g5 d5e4 c3e4 f8e7 g5f6 e7f6 c2c3 b8d7 g1f3 e8g8 f1d3 e6e5 e1g1 e5d4 c3d4 d7b6 d1c2 f6d4
info depth 24 seldepth 36 score cp 33 nodes 26221043 time 10400 nps 2521251 hashfull 1000 pv e2e4 e7e6 d2d4 d7d5 b1c3 g8f6 c1g5 d5e4 c3e4 f8e7 g5f6 e7f6 c2c3 b8d7 g1f3 e8g8 f1d3 e6e5 e4f6 d8f6 e1g1 e5d4 c3d4 f6b6 d1c2 d7f6
*/


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
    
    /*
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
    */
    // movePicker mp;
    

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
    if (true){
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

    // perftBoard.readFen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    // ~82ns per position
    // std::cout<<perft(0, 5)<<std::endl;
    // std::cout<<std::fixed<<(tot + 0.0) / cnt<<std::endl;
    // std::cout<<totLegal<<std::endl;
    // return 0;
    


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

    board.readFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    std::cout<<board.eval()<<std::endl;
    // return 0;
    // 6203035
    uciSearchLims lims;
    lims.movesToGo = 50;
    lims.timeIncr[board.getTurn()] = 0;
    lims.timeLeft[board.getTurn()] = 20000 * 50;

    beginSearch(board, lims);
    // beginSearch(board, lims);
}
/*
.\cutechess-cli `
-engine conf="E118_SEReduceCut" `
-engine conf="E117_BetterIIR" `
-each tc=6+0.06 timemargin=200 `
-openings file="C:\Program Files\Cute Chess\Chess Openings\openings-10ply-100k.pgn" order=random `
-games 2 `
-rounds 25000 `
-repeat 2 `
-maxmoves 200 `
-draw movenumber=40 movecount=4 score=5 `
-recover `
-concurrency 10 `
-ratinginterval 10 `
-sprt elo0=0 elo1=5 alpha=0.05 beta=0.05
*/

/*
Score of E117_BetterIIR vs E114_QSspeedup: 2017 - 1992 - 8602  [0.501] 12611
...      E117_BetterIIR playing White: 1353 - 677 - 4275  [0.554] 6305
...      E117_BetterIIR playing Black: 664 - 1315 - 4327  [0.448] 6306
...      White vs Black: 2668 - 1341 - 8602  [0.553] 12611
Elo difference: 0.7 +/- 3.4, LOS: 65.4 %, DrawRatio: 68.2 %
SPRT: llr -2.98 (-101.1%), lbound -2.94, ubound 2.94 - H0 was accepted
*/

/*
Score of E118_SEReduceCut vs E117_BetterIIR: 1946 - 1936 - 7363  [0.500] 11245
...      E118_SEReduceCut playing White: 1313 - 680 - 3630  [0.556] 5623
...      E118_SEReduceCut playing Black: 633 - 1256 - 3733  [0.445] 5622
...      White vs Black: 2569 - 1313 - 7363  [0.556] 11245
Elo difference: 0.3 +/- 3.8, LOS: 56.4 %, DrawRatio: 65.5 %
SPRT: llr -2.96 (-100.4%), lbound -2.94, ubound 2.94 - H0 was accepted
*/

/*
Score of E116_Cutnode vs E114_QSspeedup: 1067 - 1083 - 4676  [0.499] 6826
...      E116_Cutnode playing White: 714 - 331 - 2369  [0.556] 3414
...      E116_Cutnode playing Black: 353 - 752 - 2307  [0.442] 3412
...      White vs Black: 1466 - 684 - 4676  [0.557] 6826
Elo difference: -0.8 +/- 4.6, LOS: 36.5 %, DrawRatio: 68.5 %
SPRT: llr -2.98 (-101.0%), lbound -2.94, ubound 2.94 - H0 was accepted
*/

/*
Score of E114_QSspeedup vs E107_ScoreLaterBetterProbcut: 283 - 207 - 1037  [0.525] 1527
...      E114_QSspeedup playing White: 190 - 73 - 501  [0.577] 764
...      E114_QSspeedup playing Black: 93 - 134 - 536  [0.473] 763
...      White vs Black: 324 - 166 - 1037  [0.552] 1527
Elo difference: 17.3 +/- 9.8, LOS: 100.0 %, DrawRatio: 67.9 %
SPRT: llr 2.95 (100.0%), lbound -2.94, ubound 2.94 - H1 was accepted
*/

/*
Score of E113_QSFP vs E107_ScoreLaterBetterProbcut: 915 - 926 - 3932  [0.499] 5773
...      E113_QSFP playing White: 629 - 298 - 1960  [0.557] 2887
...      E113_QSFP playing Black: 286 - 628 - 1972  [0.441] 2886
...      White vs Black: 1257 - 584 - 3932  [0.558] 5773
Elo difference: -0.7 +/- 5.1, LOS: 39.9 %, DrawRatio: 68.1 %
SPRT: llr -2.37 (-80.5%), lbound -2.94, ubound 2.94
*/

/*
Score of E112_CheckDecreaseReduction vs E107_ScoreLaterBetterProbcut: 422 - 437 - 1923  [0.497] 2782
...      E112_CheckDecreaseReduction playing White: 297 - 148 - 946  [0.554] 1391
...      E112_CheckDecreaseReduction playing Black: 125 - 289 - 977  [0.441] 1391
...      White vs Black: 586 - 273 - 1923  [0.556] 2782
Elo difference: -1.9 +/- 7.2, LOS: 30.4 %, DrawRatio: 69.1 %
SPRT: llr -1.63 (-55.4%), lbound -2.94, ubound 2.94
*/

/*
Score of E111_Razoring2 vs E107_ScoreLaterBetterProbcut: 2269 - 2234 - 9527  [0.501] 14030
...      E111_Razoring2 playing White: 1504 - 749 - 4762  [0.554] 7015
...      E111_Razoring2 playing Black: 765 - 1485 - 4765  [0.449] 7015
...      White vs Black: 2989 - 1514 - 9527  [0.553] 14030
Elo difference: 0.9 +/- 3.3, LOS: 69.9 %, DrawRatio: 67.9 %
SPRT: llr -2.96 (-100.4%), lbound -2.94, ubound 2.94 - H0 was accepted

Player: E111_Razoring2
   "Draw by 3-fold repetition": 1536
   "Draw by adjudication": 6978
   "Draw by adjudication: maximal game length": 177
   "Draw by fifty moves rule": 433
   "Draw by insufficient mating material": 378
   "Draw by stalemate": 25
   "Loss: Black disconnects": 3
   "Loss: Black mates": 744
   "Loss: White disconnects": 5
   "Loss: White mates": 1482
   "No result": 7
   "Win: Black mates": 765
   "Win: White mates": 1504
Player: E107_ScoreLaterBetterProbcut
   "Draw by 3-fold repetition": 1536
   "Draw by adjudication": 6978
   "Draw by adjudication: maximal game length": 177
   "Draw by fifty moves rule": 433
   "Draw by insufficient mating material": 378
   "Draw by stalemate": 25
   "Loss: Black mates": 765
   "Loss: White mates": 1504
   "No result": 7
   "Win: Black disconnects": 3
   "Win: Black mates": 744
   "Win: White disconnects": 5
   "Win: White mates": 1482
*/

/*
Score of E110_Razoring1 vs E107_ScoreLaterBetterProbcut: 317 - 362 - 1591  [0.490] 2270
...      E110_Razoring1 playing White: 210 - 125 - 800  [0.537] 1135
...      E110_Razoring1 playing Black: 107 - 237 - 791  [0.443] 1135
...      White vs Black: 447 - 232 - 1591  [0.547] 2270
Elo difference: -6.9 +/- 7.8, LOS: 4.2 %, DrawRatio: 70.1 %
SPRT: llr -2.96 (-100.4%), lbound -2.94, ubound 2.94 - H0 was accepted
*/

/*
Score of E108_SnapScore vs E107_ScoreLaterBetterProbcut: 197 - 253 - 943  [0.480] 1393
...      E108_SnapScore playing White: 134 - 87 - 476  [0.534] 697
...      E108_SnapScore playing Black: 63 - 166 - 467  [0.426] 696
...      White vs Black: 300 - 150 - 943  [0.554] 1393
Elo difference: -14.0 +/- 10.3, LOS: 0.4 %, DrawRatio: 67.7 %
SPRT: llr -2.96 (-100.5%), lbound -2.94, ubound 2.94 - H0 was accepted
*/

/*
Score of E107_ScoreLaterBetterProbcut vs E104_BiasBuckets: 1037 - 927 - 4134  [0.509] 6098
...      E107_ScoreLaterBetterProbcut playing White: 711 - 299 - 2039  [0.568] 3049
...      E107_ScoreLaterBetterProbcut playing Black: 326 - 628 - 2095  [0.450] 3049
...      White vs Black: 1339 - 625 - 4134  [0.559] 6098
Elo difference: 6.3 +/- 4.9, LOS: 99.3 %, DrawRatio: 67.8 %
SPRT: llr 2.96 (100.5%), lbound -2.94, ubound 2.94 - H1 was accepted
*/

/*
Score of E105_StagedMoveGen1 vs E104_BiasBuckets: 2054 - 2033 - 8299  [0.501] 12386
...      E105_StagedMoveGen1 playing White: 1404 - 644 - 4145  [0.561] 6193
...      E105_StagedMoveGen1 playing Black: 650 - 1389 - 4154  [0.440] 6193
...      White vs Black: 2793 - 1294 - 8299  [0.561] 12386
Elo difference: 0.6 +/- 3.5, LOS: 62.9 %, DrawRatio: 67.0 %
SPRT: llr -2.97 (-100.9%), lbound -2.94, ubound 2.94 - H0 was accepted
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