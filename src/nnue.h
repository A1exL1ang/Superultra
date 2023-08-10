#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include "assert.h"
#include "types.h"
#include "helpers.h"

const int kingBucketCount = 6;
const int singleKingBucketSize = 768;

const int inputHalf = kingBucketCount * singleKingBucketSize;
const int hiddenHalf = 512;
const int evalScale = 400;

const int Q1 = 256;
const int Q2 = 256;

const NNUEWeight creluL = 0;
const NNUEWeight creluR = Q1;

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
    alignas(32) std::array<NNUEWeight, hiddenHalf * 2> accum;

    void addFeature(Piece pieceType, Square sq, Color col, Square wking, Square bking);
    void removeFeature(Piece pieceType, Square sq, Color col, Square wking, Square bking);
    void updateMove(Piece pieceType, Square st, Square en, Color col, Square wking, Square bking);
    void refresh(Piece *board, Square wking, Square bking);
    Score eval(Color col);
};

inline int getInputIndex(Piece pieceType, Color col, Square sq, Color perspective, Square kingPos){
    return (col == perspective ? 0 : 384) 
           + 64 * (pieceType - 1) 
           + (perspective == white ? sq : flip(sq))
           + kingBucketId[perspective == white ? kingPos : flip(kingPos)] * singleKingBucketSize;
}

void initNNUEWeights();