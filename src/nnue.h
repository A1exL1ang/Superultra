#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include "assert.h"
#include "types.h"
#include "helpers.h"

const int KING_BUCKET_COUNT = 10;
const int OUTPUT_WEIGHT_BUCKET_COUNT = 8;
const int SINGLE_KING_BUCKET_SIZE = 768;

const int INPUT_HALF = KING_BUCKET_COUNT * SINGLE_KING_BUCKET_SIZE;
const int HIDDEN_HALF = 512;
const int evalScale = 400;

const int Q1 = 256;
const int Q2 = 256;

const NNUEWeight CRELU_L = 0;
const NNUEWeight CRELU_R = Q1;

const int KING_BUCKET_ID[64] = {
    0, 1, 2, 3, 3, 2, 1, 0,
    4, 5, 6, 7, 7, 6, 5, 4,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9,
    9, 9, 9, 9, 9, 9, 9, 9
};

struct NeuralNetwork{    
    alignas(32) std::array<NNUEWeight, HIDDEN_HALF * 2> accum;

    void addFeature(Piece pieceType, Square sq, Color col, Square wking, Square bking);
    void removeFeature(Piece pieceType, Square sq, Color col, Square wking, Square bking);
    void updateMove(Piece pieceType, Square st, Square en, Color col, Square wking, Square bking);
    void refresh(Piece *board, Square wking, Square bking);
    Score eval(Color col, int8 pieceCount);
};

inline int getInputIndex(Piece pieceType, Color col, Square sq, Color perspective, Square kingPos){
    return (col == perspective ? 0 : 384) 
           + 64 * (pieceType - 1) 
           + (perspective == WHITE ? sq : flip(sq))
           + KING_BUCKET_ID[perspective == WHITE ? kingPos : flip(kingPos)] * SINGLE_KING_BUCKET_SIZE;
}

inline int calculateOutputBucket(int8 pieceCount){
    return (pieceCount - 2) / 4;
}

void initNNUEWeights();