#include <immintrin.h>
#include <cstring>
#include "incbin/incbin.h"
#include "nnue.h"
#include "types.h"
#include "helpers.h"

INCBIN(nnueNet, "nnueweights.bin");

alignas(32) static NNUEWeight W1[INPUT_HALF * HIDDEN_HALF];
alignas(32) static NNUEWeight B1[HIDDEN_HALF];
alignas(32) static NNUEWeight W2[OUTPUT_WEIGHT_BUCKET_COUNT * HIDDEN_HALF * 2];
alignas(32) static NNUEWeight B2[OUTPUT_WEIGHT_BUCKET_COUNT];

void NeuralNetwork::addFeature(Piece pieceType, Square sq, Color col, Square wking, Square bking){
    const int whitePerspIdx = getInputIndex(pieceType, col, sq, WHITE, wking) * HIDDEN_HALF;
    const int blackPerspIdx = getInputIndex(pieceType, col, sq, BLACK, bking) * HIDDEN_HALF;
    
#if defined(__AVX__) || defined(__AVX2__)
    const auto vectorAccumWhitePtr = reinterpret_cast<__m256i*>(&accum[0]);
    const auto vectorWeightWhitePtr = reinterpret_cast<__m256i*>(&W1[whitePerspIdx]);

    const auto vectorAccumBlackPtr = reinterpret_cast<__m256i*>(&accum[HIDDEN_HALF]);
    const auto vectorWeightBlackPtr = reinterpret_cast<__m256i*>(&W1[blackPerspIdx]);

    for (int i = 0; i < HIDDEN_HALF / 16; i++){
        vectorAccumWhitePtr[i] = _mm256_add_epi16(vectorAccumWhitePtr[i], vectorWeightWhitePtr[i]);
    }
    for (int i = 0; i < HIDDEN_HALF / 16; i++){    
        vectorAccumBlackPtr[i] = _mm256_add_epi16(vectorAccumBlackPtr[i], vectorWeightBlackPtr[i]);
    }
#else
    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < HIDDEN_HALF; i++){
        accum[i] += W1[whitePerspIdx + i];
    }
    for (int i = 0; i < HIDDEN_HALF; i++){
        accum[i + HIDDEN_HALF] += W1[blackPerspIdx + i];
    }
#endif
}

void NeuralNetwork::removeFeature(Piece pieceType, Square sq, Color col, Square wking, Square bking){
    int whitePerspIdx = getInputIndex(pieceType, col, sq, WHITE, wking) * HIDDEN_HALF;
    int blackPerspIdx = getInputIndex(pieceType, col, sq, BLACK, bking) * HIDDEN_HALF;

#if defined(__AVX__) || defined(__AVX2__)
    const auto vectorAccumWhitePtr = reinterpret_cast<__m256i*>(&accum[0]);
    const auto vectorWeightWhitePtr = reinterpret_cast<__m256i*>(&W1[whitePerspIdx]);

    const auto vectorAccumBlackPtr = reinterpret_cast<__m256i*>(&accum[HIDDEN_HALF]);
    const auto vectorWeightBlackPtr = reinterpret_cast<__m256i*>(&W1[blackPerspIdx]);

    for (int i = 0; i < HIDDEN_HALF / 16; i++){
        vectorAccumWhitePtr[i] = _mm256_sub_epi16(vectorAccumWhitePtr[i], vectorWeightWhitePtr[i]);
    }
    for (int i = 0; i < HIDDEN_HALF / 16; i++){    
        vectorAccumBlackPtr[i] = _mm256_sub_epi16(vectorAccumBlackPtr[i], vectorWeightBlackPtr[i]);
    }
#else
    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < HIDDEN_HALF; i++){
        accum[i] -= W1[whitePerspIdx + i];
    }
    for (int i = 0; i < HIDDEN_HALF; i++){
        accum[i + HIDDEN_HALF] -= W1[blackPerspIdx + i];
    }
#endif
}

void NeuralNetwork::updateMove(Piece pieceType, Square st, Square en, Color col, Square wking, Square bking){
    int whitePerspStIdx = getInputIndex(pieceType, col, st, WHITE, wking) * HIDDEN_HALF;
    int blackPerspEnIdx = getInputIndex(pieceType, col, en, BLACK, bking) * HIDDEN_HALF;

    int whitePerspEnIdx = getInputIndex(pieceType, col, en, WHITE, wking) * HIDDEN_HALF;
    int blackPerspStIdx = getInputIndex(pieceType, col, st, BLACK, bking) * HIDDEN_HALF;

#if defined(__AVX__) || defined(__AVX2__)
    const auto vectorAccumWhitePtr = reinterpret_cast<__m256i*>(&accum[0]);
    const auto vectorWeightStWhitePtr = reinterpret_cast<__m256i*>(&W1[whitePerspStIdx]);
    const auto vectorWeightEnWhitePtr = reinterpret_cast<__m256i*>(&W1[whitePerspEnIdx]);

    const auto vectorAccumBlackPtr = reinterpret_cast<__m256i*>(&accum[HIDDEN_HALF]);
    const auto vectorWeightStBlackPtr = reinterpret_cast<__m256i*>(&W1[blackPerspStIdx]);
    const auto vectorWeightEnBlackPtr = reinterpret_cast<__m256i*>(&W1[blackPerspEnIdx]);

    for (int i = 0; i < HIDDEN_HALF / 16; i++){
        vectorAccumWhitePtr[i] = _mm256_add_epi16(vectorAccumWhitePtr[i], _mm256_sub_epi16(vectorWeightEnWhitePtr[i], vectorWeightStWhitePtr[i]));
    }
    for (int i = 0; i < HIDDEN_HALF / 16; i++){
        vectorAccumBlackPtr[i] = _mm256_add_epi16(vectorAccumBlackPtr[i], _mm256_sub_epi16(vectorWeightEnBlackPtr[i], vectorWeightStBlackPtr[i]));
    }
#else
    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < HIDDEN_HALF; i++){
        accum[i] += -W1[whitePerspStIdx + i] + W1[whitePerspEnIdx + i];
    }
    for (int i = 0; i < HIDDEN_HALF; i++){
        accum[i + HIDDEN_HALF] += -W1[blackPerspStIdx + i] + W1[blackPerspEnIdx + i];
    }
#endif
}

void NeuralNetwork::refresh(Piece *board, Square wking, Square bking){
    // Our first step is to init with biases
#if defined(__AVX__) || defined(__AVX2__)
    const auto vectorAccumTopPtr = reinterpret_cast<__m256i*>(&accum[0]);
    const auto vectorAccumBotPtr = reinterpret_cast<__m256i*>(&accum[HIDDEN_HALF]);
    const auto vectorBiasPtr = reinterpret_cast<__m256i*>(&B1[0]);
    
    for (int i = 0; i < HIDDEN_HALF / 16; i++){
        vectorAccumTopPtr[i] = vectorBiasPtr[i];
    }
    for (int i = 0; i < HIDDEN_HALF / 16; i++){
        vectorAccumBotPtr[i] = vectorBiasPtr[i];
    }
#else
    for (int i = 0; i < HIDDEN_HALF; i++){
        accum[i] = B1[i];
    }
    for (int i = 0; i < HIDDEN_HALF; i++){
        accum[i + HIDDEN_HALF] = B1[i];
    }
#endif
    // Iterate over pieces and add them
    for (Square sq = 0; sq < 64; sq++){
        if (board[sq] != NO_PIECE){
            addFeature(getPieceType(board[sq]), sq, getPieceColor(board[sq]), wking, bking);
        }
    }
}

Score NeuralNetwork::eval(Color col, int8 pieceCount){
    int topIdx = (col == WHITE ? 0 : HIDDEN_HALF);
    int botIdx = (col == WHITE ? HIDDEN_HALF : 0);

    int outputWeightBucket = calculateOutputBucket(pieceCount);
    int outputWeightsIndex = outputWeightBucket * HIDDEN_HALF * 2;

    int eval = B2[outputWeightBucket];

#if defined(__AVX__) || defined(__AVX2__)
    // Vector of int32 to avoid overflow
    auto vectorEval = _mm256_setzero_si256();
    const auto vectorCreluL = _mm256_set1_epi16(CRELU_L);
    const auto vectorCreluR = _mm256_set1_epi16(CRELU_R);

    const auto vectorAccumTopPtr = reinterpret_cast<__m256i*>(&accum[topIdx]);
    const auto vectorWeightTopPtr = reinterpret_cast<__m256i*>(&W2[outputWeightsIndex]);

    const auto vectorAccumBotPtr = reinterpret_cast<__m256i*>(&accum[botIdx]);
    const auto vectorWeightBotPtr = reinterpret_cast<__m256i*>(&W2[outputWeightsIndex + HIDDEN_HALF]);

    // The only thing i'll note here is that _mm256_madd_epi16(a, b) is very useful.
    // The function takes in two int16 vectors and multiplies every pair of elems and then
    // adds adjacent elements so you are finally left with a int32 vector

    for (int i = 0; i < HIDDEN_HALF / 16; i++){
        vectorEval = _mm256_add_epi32(
            vectorEval, 
            _mm256_madd_epi16(
                _mm256_min_epi16(_mm256_max_epi16(vectorAccumTopPtr[i], vectorCreluL), vectorCreluR), 
                vectorWeightTopPtr[i]
            )
        );
    }
    for (int i = 0; i < HIDDEN_HALF / 16; i++){
        vectorEval = _mm256_add_epi32(
            vectorEval, 
            _mm256_madd_epi16(
                _mm256_min_epi16(_mm256_max_epi16(vectorAccumBotPtr[i], vectorCreluL), vectorCreluR), 
                vectorWeightBotPtr[i]
            )
        );
    }
    // Finally, horizontally add. Speed here doesnt really matter
    for (int i = 0; i < 8; i++){
        eval += _mm256_extract_epi32(vectorEval, i);
    }
#else
    for (int i = 0; i < HIDDEN_HALF; i++){
        int16 input = accum[topIdx + i];
        int16 weight = W2[outputWeightsIndex + i];
        eval += std::clamp(input, CRELU_L, CRELU_R) * weight;
    }
    for (int i = 0; i < HIDDEN_HALF; i++){
        int16 input = accum[botIdx + i];
        int16 weight = W2[outputWeightsIndex + HIDDEN_HALF + i];
        eval += std::clamp(input, CRELU_L, CRELU_R) * weight;
    }
#endif
    // Undo quantization and eval scale
    eval = ((eval * evalScale) / (Q1 * Q2));
    return static_cast<Score>(eval);
}

void initNNUEWeights(){
    int idx = 0;

    // W1
    memcpy(W1, gnnueNetData + idx, INPUT_HALF * HIDDEN_HALF * sizeof(NNUEWeight));
    idx += INPUT_HALF * HIDDEN_HALF * sizeof(NNUEWeight);

    // B1
    memcpy(B1, gnnueNetData + idx, HIDDEN_HALF * sizeof(NNUEWeight));
    idx += HIDDEN_HALF * sizeof(NNUEWeight);

    // W2
    memcpy(W2, gnnueNetData + idx, OUTPUT_WEIGHT_BUCKET_COUNT * HIDDEN_HALF * 2 * sizeof(NNUEWeight));
    idx += HIDDEN_HALF * sizeof(NNUEWeight) * 2;
    
    // B2
    memcpy(B2, gnnueNetData + idx, OUTPUT_WEIGHT_BUCKET_COUNT * sizeof(NNUEWeight));
}