#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <cassert>
#include "types.h"
#include "helpers.h"

constexpr int kingBucketCount = 10, outputWeightBucketCount = 8, singleKingBucketSize = 768;
constexpr int inputHalf = kingBucketCount * singleKingBucketSize, hiddenHalf = 512, evalScale = 400;
constexpr int Q1 = 256, Q2 = 256;
constexpr NNUEWeight creluL = 0, creluR = Q1;

constexpr int kingBucketId[64] = {
    0, 1, 2, 3, 3, 2, 1, 0,
    4, 5, 6, 7, 7, 6, 5, 4,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9
};

struct neuralNetwork {    
    alignas(32) std::array<NNUEWeight, hiddenHalf * 2> accum;

    void addFeature(Piece pieceType, Square sq, Color col, Square wking, Square bking);
    void removeFeature(Piece pieceType, Square sq, Color col, Square wking, Square bking);
    void updateMove(Piece pieceType, Square st, Square en, Color col, Square wking, Square bking);
    void refresh(Piece *board, Square wking, Square bking);
    Score eval(Color col, int8 pieceCount);
};

inline int getInputIndex(Piece pieceType, Color col, Square sq, Color perspective, Square kingPos) {
    return (col == perspective ? 0 : 384) + 64 * (pieceType - 1) + (perspective == white ? sq : flip(sq)) + kingBucketId[perspective == white ? kingPos : flip(kingPos)] * singleKingBucketSize;
}

inline int calculateOutputBucket(int8 pieceCount) {
    return (pieceCount - 2) / 4;
}

void initNNUEWeights();

