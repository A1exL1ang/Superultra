#include <immintrin.h>
#include "incbin/incbin.h"
#include "nnue.h"
#include "types.h"
#include "helpers.h"

INCBIN(nnueNet, "nnueweights.bin");

alignas(32) static nnueWeight_t W1[inputHalf * hiddenHalf];
alignas(32) static nnueWeight_t B1[hiddenHalf];
alignas(32) static nnueWeight_t W2[hiddenHalf * 2];
alignas(32) static nnueWeight_t B2;

void neuralNetwork::addFeature(piece_t pieceType, square_t sq, color_t col, square_t wking, square_t bking){
    const int whitePerspIdx = getInputIndex(pieceType, col, sq, white, wking) * hiddenHalf;
    const int blackPerspIdx = getInputIndex(pieceType, col, sq, black, bking) * hiddenHalf;
    
#if defined(__AVX__) || defined(__AVX2__)
    const auto vectorAccumWhitePtr = reinterpret_cast<__m256i*>(&accum);
    const auto vectorWeightWhitePtr = reinterpret_cast<__m256i*>(&W1[whitePerspIdx]);

    const auto vectorAccumBlackPtr = reinterpret_cast<__m256i*>(&accum[hiddenHalf]);
    const auto vectorWeightBlackPtr = reinterpret_cast<__m256i*>(&W1[blackPerspIdx]);

    for (int i = 0; i < hiddenHalf / 16; i++){
        vectorAccumWhitePtr[i] = _mm256_add_epi16(vectorAccumWhitePtr[i], vectorWeightWhitePtr[i]);
    }
    for (int i = 0; i < hiddenHalf / 16; i++){    
        vectorAccumBlackPtr[i] = _mm256_add_epi16(vectorAccumBlackPtr[i], vectorWeightBlackPtr[i]);
    }
#else
    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < hiddenHalf; i++){
        accum[i] += W1[whitePerspIdx + i];
    }
    for (int i = 0; i < hiddenHalf; i++){
        accum[i + hiddenHalf] += W1[blackPerspIdx + i];
    }
#endif
}

void neuralNetwork::removeFeature(piece_t pieceType, square_t sq, color_t col, square_t wking, square_t bking){
    int whitePerspIdx = getInputIndex(pieceType, col, sq, white, wking) * hiddenHalf;
    int blackPerspIdx = getInputIndex(pieceType, col, sq, black, bking) * hiddenHalf;

#if defined(__AVX__) || defined(__AVX2__)
    const auto vectorAccumWhitePtr = reinterpret_cast<__m256i*>(&accum);
    const auto vectorWeightWhitePtr = reinterpret_cast<__m256i*>(&W1[whitePerspIdx]);

    const auto vectorAccumBlackPtr = reinterpret_cast<__m256i*>(&accum[hiddenHalf]);
    const auto vectorWeightBlackPtr = reinterpret_cast<__m256i*>(&W1[blackPerspIdx]);

    for (int i = 0; i < hiddenHalf / 16; i++){
        vectorAccumWhitePtr[i] = _mm256_sub_epi16(vectorAccumWhitePtr[i], vectorWeightWhitePtr[i]);
    }
    for (int i = 0; i < hiddenHalf / 16; i++){    
        vectorAccumBlackPtr[i] = _mm256_sub_epi16(vectorAccumBlackPtr[i], vectorWeightBlackPtr[i]);
    }
#else
    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < hiddenHalf; i++){
        accum[i] -= W1[whitePerspIdx + i];
    }
    for (int i = 0; i < hiddenHalf; i++){
        accum[i + hiddenHalf] -= W1[blackPerspIdx + i];
    }
#endif
}

void neuralNetwork::updateMove(piece_t pieceType, square_t st, square_t en, color_t col, square_t wking, square_t bking){
    int whitePerspStIdx = getInputIndex(pieceType, col, st, white, wking) * hiddenHalf;
    int blackPerspEnIdx = getInputIndex(pieceType, col, en, black, bking) * hiddenHalf;

    int whitePerspEnIdx = getInputIndex(pieceType, col, en, white, wking) * hiddenHalf;
    int blackPerspStIdx = getInputIndex(pieceType, col, st, black, bking) * hiddenHalf;

#if defined(__AVX__) || defined(__AVX2__)
    const auto vectorAccumWhitePtr = reinterpret_cast<__m256i*>(&accum);
    const auto vectorWeightStWhitePtr = reinterpret_cast<__m256i*>(&W1[whitePerspStIdx]);
    const auto vectorWeightEnWhitePtr = reinterpret_cast<__m256i*>(&W1[whitePerspEnIdx]);

    const auto vectorAccumBlackPtr = reinterpret_cast<__m256i*>(&accum[hiddenHalf]);
    const auto vectorWeightStBlackPtr = reinterpret_cast<__m256i*>(&W1[blackPerspStIdx]);
    const auto vectorWeightEnBlackPtr = reinterpret_cast<__m256i*>(&W1[blackPerspEnIdx]);

    for (int i = 0; i < hiddenHalf / 16; i++){
        vectorAccumWhitePtr[i] = _mm256_add_epi16(vectorAccumWhitePtr[i], _mm256_sub_epi16(vectorWeightEnWhitePtr[i], vectorWeightStWhitePtr[i]));
    }
    for (int i = 0; i < hiddenHalf / 16; i++){
        vectorAccumBlackPtr[i] = _mm256_add_epi16(vectorAccumBlackPtr[i], _mm256_sub_epi16(vectorWeightEnBlackPtr[i], vectorWeightStBlackPtr[i]));
    }
#else
    // Doing it with 2 for loops offers a small speedup
    for (int i = 0; i < hiddenHalf; i++){
        accum[i] += -W1[whitePerspStIdx + i] + W1[whitePerspEnIdx + i];
    }
    for (int i = 0; i < hiddenHalf; i++){
        accum[i + hiddenHalf] += -W1[blackPerspStIdx + i] + W1[blackPerspEnIdx + i];
    }
#endif
}

void neuralNetwork::refresh(piece_t *board, square_t wking, square_t bking){
    // Our first step is to init with biases
#if defined(__AVX__) || defined(__AVX2__)
    const auto vectorAccumTopPtr = reinterpret_cast<__m256i*>(&accum[0]);
    const auto vectorAccumBotPtr = reinterpret_cast<__m256i*>(&accum[hiddenHalf]);
    const auto vectorBiasPtr = reinterpret_cast<__m256i*>(&B1);
    
    for (int i = 0; i < hiddenHalf / 16; i++){
        vectorAccumTopPtr[i] = vectorBiasPtr[i];
    }
    for (int i = 0; i < hiddenHalf / 16; i++){
        vectorAccumBotPtr[i] = vectorBiasPtr[i];
    }
#else
    for (int i = 0; i < hiddenHalf; i++){
        accum[i] = B1[i];
    }
    for (int i = 0; i < hiddenHalf; i++){
        accum[i + hiddenHalf] = B1[i];
    }
#endif
    // Iterate over pieces and add them
    for (square_t sq = 0; sq < 64; sq++){
        if (board[sq] != noPiece){
            addFeature(getPieceType(board[sq]), sq, getPieceColor(board[sq]), wking, bking);
        }
    }
}

score_t neuralNetwork::eval(color_t col){
    int topIdx = (col == white ? 0 : hiddenHalf);
    int botIdx = (col == white ? hiddenHalf : 0);
    int eval = B2;

#if defined(__AVX__) || defined(__AVX2__)
    // Vector of int32 to avoid overflow
    __m256i vectorEval = _mm256_setzero_si256();
    const __m256i vectorCreluL = _mm256_set1_epi16(creluL);
    const __m256i vectorCreluR = _mm256_set1_epi16(creluR);

    const auto vectorAccumTopPtr = reinterpret_cast<__m256i*>(&accum[topIdx]);
    const auto vectorWeightTopPtr = reinterpret_cast<__m256i*>(&W2[0]);

    const auto vectorAccumBotPtr = reinterpret_cast<__m256i*>(&accum[botIdx]);
    const auto vectorWeightBotPtr = reinterpret_cast<__m256i*>(&W2[hiddenHalf]);

    // The only thing i'll note here is that _mm256_madd_epi16(a, b) is very useful.
    // The function takes in two int16 vectors and multiplies eveyr pair of elems and then
    // adds adjacent elements so you are finally left with a int32 vector!

    for (int i = 0; i < hiddenHalf / 16; i++){
        vectorEval = _mm256_add_epi32(
            vectorEval, 
            _mm256_madd_epi16(
                _mm256_min_epi16(_mm256_max_epi16(vectorAccumTopPtr[i], vectorCreluL), vectorCreluR), 
                vectorWeightTopPtr[i]
            )
        );
    }
    for (int i = 0; i < hiddenHalf / 16; i++){
        vectorEval = _mm256_add_epi32(
            vectorEval, 
            _mm256_madd_epi16(
                _mm256_min_epi16(_mm256_max_epi16(vectorAccumBotPtr[i], vectorCreluL), vectorCreluR), 
                vectorWeightBotPtr[i]
            )
        );
    }
    // Finally, horizontally add. Speed here doesnt really matter
    for (int i = 0; i < 8; i++)
        eval += _mm256_extract_epi32(vectorEval, i);
#else
    for (int i = 0; i < hiddenHalf; i++){
        int16 input = accum[topIdx + i];
		int16 weight = W2[i];
		eval += std::clamp(input, creluL, creluR) * weight;
    }
    for (int i = 0; i < hiddenHalf; i++){
        int16 input = accum[botIdx + i];
        int16 weight = W2[hiddenHalf + i];
        eval += std::clamp(input, creluL, creluR) * weight;
    }
#endif
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