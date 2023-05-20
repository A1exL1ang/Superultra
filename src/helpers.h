#include "types.h"

#pragma once

// Gets the piece of an encoded piece
inline piece_t getPieceType(piece_t encPiece){
    return encPiece >> 1;
}

// Gets the color of an encoded piece
inline piece_t getPieceColor(piece_t encPiece){
    return (encPiece & 1);
}

// Encode a piece given the piece type and color
inline piece_t encodePiece(piece_t piece, color_t col){
    return ((piece << 1) | col);
}

// Gets the lowest set bit
inline int8 lsb(uint64 num){
    return __builtin_ctzll(num);
}

// Gets the highest set bit
inline int8 msb(uint64 num){
    return __builtin_clzll(num) ^ 63;
}

// Counts the number of set bits
inline int8 countOnes(uint64 num){
    return __builtin_popcountll(num);
}

// Returns the lowest set bit and removes it
inline int8 poplsb(uint64 &num){
    int8 b = __builtin_ctzll(num);
    num &= (num - 1);
    return b;
}

// Does the bitmask have the bit
inline bool hasBit(bitboard_t mask, int8 bit){
    return (mask & (1ULL << bit));
}

// Vertical flip a square
inline square_t flip(square_t sq){
    return sq ^ 56;
}

// Flip square if black
inline square_t flipIfBlack(square_t sq, color_t col){
    return col == white ? sq : flip(sq);
}

// Gets the file of a square
inline file_t getFile(square_t sq){
    return (sq & 7);
}

// Gets the rank of a square
inline rank_t getRank(square_t sq){
    return (sq >> 3);
}

// Gets relative rank (distance to the bottom of the board from your perspective)
inline rank_t relativeRank(square_t sq, color_t col){
    return getRank(flipIfBlack(sq, col));
}

// Determine if (rank, file) is inside the board
inline bool isInGrid(rank_t i, file_t j){ 
    return i >= 0 and i < 8 and j >= 0 and j < 8;
}

// Get the square representing (rank, file)
inline square_t posToSquare(rank_t i, file_t j){
    return i * 8 + j;
}