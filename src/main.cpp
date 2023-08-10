#include "board.h"
#include "uci.h"
#include "attacks.h"
#include "search.h"
#include <iostream>
#include <fstream>
#include <math.h>
struct nn{
    float W1[inputHalf * hiddenHalf];
    float B1[hiddenHalf];
    float W2[hiddenHalf * 2];
    float B2;
};
nn model;

 void importCheckpoint(std::string path){
    // Note that we may only import checkpoints (networks with float weights)
    std::ifstream inputFile(path, std::ios::binary);
    inputFile.read(reinterpret_cast<char*>(&model.W1), sizeof(model.W1));
    inputFile.read(reinterpret_cast<char*>(&model.B1), sizeof(model.B1));
    inputFile.read(reinterpret_cast<char*>(&model.W2), sizeof(model.W2));
    inputFile.read(reinterpret_cast<char*>(&model.B2), sizeof(model.B2));
}

inline void exportNetworkQuantized(){
    std::ofstream outFile("weights.bin", std::ios::out | std::ios::binary);

    int16_t qW1[inputHalf * hiddenHalf];
    int16_t qB1[hiddenHalf];
    int16_t qW2[hiddenHalf * 2];
    int16_t qB2;
    
    // We need to convert these doubles to int16. Let's multiply W1 and B1 by Q1,
    // W2 by Q2, and B2 by Q1 * Q2 so that we can factor Q1 * Q2 out of everyone.
    // Then to get back the original value, we divide by Q1 * Q2

    for (int i = 0; i < inputHalf * hiddenHalf; i++){
        qW1[i] = round(model.W1[i] * Q1);
    }
    for (int i = 0; i < hiddenHalf; i++){
        qB1[i] = round(model.B1[i] * Q1);
    }
    for (int i = 0; i < hiddenHalf * 2; i++){
        qW2[i] = round(model.W2[i] * Q2);
    }
    qB2 = round(model.B2 * Q1 * Q2);

    outFile.write(reinterpret_cast<char*>(&qW1), sizeof(qW1));
    outFile.write(reinterpret_cast<char*>(&qB1), sizeof(qB1));
    outFile.write(reinterpret_cast<char*>(&qW2), sizeof(qW2));
    outFile.write(reinterpret_cast<char*>(&qB2), sizeof(qB2));
    outFile.close();
}
int main(){
    initLineBB();
    initMagicCache();
    initNNUEWeights();
    initLMR();
    initTT();

    // Default settings
    globalTT.setSize(16);
    threadCount = 1;

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
    CHANGES:
    QS Eval snapping: abs(tte.score) < foundMate
    fmr dampening
    */

    position board;

    
    threadCount = 1;
    board.readFen("rnbqkbnr/pppppppp/8/8/8/QQQQQQQQ/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout<<board.eval()<<std::endl;

    uciSearchLims lims;
    lims.movesToGo = 1;
    lims.timeLeft[board.getTurn()] = 10000;

    beginSearch(board, lims);
}
/*
.\cutechess-cli `
-engine conf="E100_NewNetQ" `
-engine conf="E88_SIMD2" `
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
*/