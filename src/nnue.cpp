#include "incbin/incbin.h"
#include "nnue.h"
#include "types.h"
#include "helpers.h"

// Loss: 0.004985
INCBIN(nnueNet, "nnueweights.bin");

static nnueWeight_t W1[inputHalf * hiddenHalf];
static nnueWeight_t B1[hiddenHalf];
static nnueWeight_t W2[hiddenHalf * 2];
static nnueWeight_t B2;

void neuralNetwork::addFeature(piece_t pieceType, square_t sq, color_t col, square_t wking, square_t bking){
    int whitePerspIdx = getInputIndex(pieceType, col, sq, white, wking) * hiddenHalf;
    int blackPerspIdx = getInputIndex(pieceType, col, sq, black, bking) * hiddenHalf;

    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < hiddenHalf; i++){
        accum[i] += W1[whitePerspIdx + i];
    }
    for (int i = 0; i < hiddenHalf; i++){
        accum[i + hiddenHalf] += W1[blackPerspIdx + i];
    }
}

void neuralNetwork::removeFeature(piece_t pieceType, square_t sq, color_t col, square_t wking, square_t bking){
    int whitePerspIdx = getInputIndex(pieceType, col, sq, white, wking) * hiddenHalf;
    int blackPerspIdx = getInputIndex(pieceType, col, sq, black, bking) * hiddenHalf;

    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < hiddenHalf; i++){
        accum[i] -= W1[whitePerspIdx + i];
    }
    for (int i = 0; i < hiddenHalf; i++){
        accum[i + hiddenHalf] -= W1[blackPerspIdx + i];
    }
}

void neuralNetwork::updateMove(piece_t pieceType, square_t st, square_t en, color_t col, square_t wking, square_t bking){
    // Faster than calling removeFeature and addFeature
    int whitePerspStIdx = getInputIndex(pieceType, col, st, white, wking) * hiddenHalf;
    int blackPerspStIdx = getInputIndex(pieceType, col, st, black, bking) * hiddenHalf;

    int whitePerspEnIdx = getInputIndex(pieceType, col, en, white, wking) * hiddenHalf;
    int blackPerspEnIdx = getInputIndex(pieceType, col, en, black, bking) * hiddenHalf;

    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < hiddenHalf; i++){
        accum[i] += -W1[whitePerspStIdx + i] + W1[whitePerspEnIdx + i];
    }
    for (int i = 0; i < hiddenHalf; i++){
        accum[i + hiddenHalf] += -W1[blackPerspStIdx + i] + W1[blackPerspEnIdx + i];
    }
}

void neuralNetwork::refresh(piece_t *board, square_t wking, square_t bking){
    // Init with bias
    for (int i = 0; i < hiddenHalf; i++){
        accum[i] = B1[i];
    }
    for (int i = 0; i < hiddenHalf; i++){
        accum[i + hiddenHalf] = B1[i];
    }

    // Iterate over pieces and add them
    for (square_t sq = 0; sq < 64; sq++){
        if (board[sq] != noPiece){
            addFeature(getPieceType(board[sq]), sq, getPieceColor(board[sq]), wking, bking);
        }
    }
}

score_t neuralNetwork::eval(color_t col){
    // To support perspective, we concatenate accumulators by putting col on top
    // Note to also remember to add bias 2
    int eval = B2;
    int topIdx = (col == white ? 0 : hiddenHalf);
    int botIdx = (col == white ? hiddenHalf : 0);

    // Doing it with 2 for loops and intializing the variables offers a 10x speedup
    for (int i = 0; i < hiddenHalf; i++){
        int16 input = accum[topIdx + i];
		int16 weight = W2[i];
		eval += std::clamp(input, creluMin, creluMax) * weight;
    }
    for (int i = 0; i < hiddenHalf; i++){
        int16 input = accum[botIdx + i];
        int16 weight = W2[hiddenHalf + i];
        eval += std::clamp(input, creluMin, creluMax) * weight;
    }
    // Undo quantization and eval scale
    eval = ((eval * evalScale) / (Q1 * Q2));
    return static_cast<score_t>(eval);
}

void initNNUEWeights(){
    int idx = 0;

    // W1
    memcpy(W1, gnnueNetData + idx, inputHalf * hiddenHalf * sizeof(nnueWeight_t));
    idx += inputHalf * hiddenHalf * sizeof(nnueWeight_t);

    // B1
    memcpy(B1, gnnueNetData + idx, hiddenHalf * sizeof(nnueWeight_t));
    idx += hiddenHalf * sizeof(nnueWeight_t);

    // W2
    memcpy(W2, gnnueNetData + idx, hiddenHalf * sizeof(nnueWeight_t) * 2);
    idx += hiddenHalf * sizeof(nnueWeight_t) * 2;
    
    // B2
    memcpy(&B2, gnnueNetData + idx, sizeof(nnueWeight_t)); 
}