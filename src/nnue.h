#include <algorithm>
#include <array>
#include <vector>
#include "assert.h"
#include "types.h"
#include "helpers.h"

#pragma once

const int kingBucketCount = 6;
const int singleKingBucketSize = 768;

const int inputHalf = kingBucketCount * singleKingBucketSize;
const int hiddenHalf = 384;
const int evalScale = 400;

const int Q1 = 255;
const int Q2 = 64;

const nnueWeight_t creluMin = 0;
const nnueWeight_t creluMax = Q1;

const int kingBucketId[64] = {
    0, 0, 1, 1, 2, 2, 3, 3,
    0, 0, 1, 1, 2, 2, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5
};

struct neuralNetwork{    
    std::array<nnueWeight_t, hiddenHalf * 2> accum;

    void addFeature(piece_t pieceType, square_t sq, color_t col, square_t wking, square_t bking);
    void removeFeature(piece_t pieceType, square_t sq, color_t col, square_t wking, square_t bking);
    void updateMove(piece_t pieceType, square_t st, square_t en, color_t col, square_t wking, square_t bking);
    void refresh(piece_t *board, square_t wking, square_t bking);
    score_t eval(color_t col);
};

inline int getInputIndex(piece_t pieceType, color_t col, square_t sq, color_t perspective, square_t kingPos){
    return (col == perspective ? 0 : 384) 
           + 64 * (pieceType - 1) 
           + (perspective == white ? sq : flip(sq))
           + kingBucketId[perspective == white ? kingPos : flip(kingPos)] * singleKingBucketSize;
}

void initNNUEWeights();