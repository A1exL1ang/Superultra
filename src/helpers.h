#pragma once

#include "types.h"
#include <chrono>

// Gets the piece of an encoded piece
inline Piece getPieceType(Piece encPiece){
    return encPiece >> 1;
}

// Gets the color of an encoded piece
inline Piece getPieceColor(Piece encPiece){
    return (encPiece & 1);
}

// Encode a piece given the piece type and color
inline Piece encodePiece(Piece piece, Color col){
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

// Vertical flip a square
inline Square flip(Square sq){
    return sq ^ 56;
}

// Flip square if black
inline Square flipIfBlack(Square sq, Color col){
    return col == white ? sq : flip(sq);
}

// Gets the file of a square
inline File getFile(Square sq){
    return (sq & 7);
}

// Gets the rank of a square
inline Rank getRank(Square sq){
    return (sq >> 3);
}

// Gets relative rank (distance to the bottom of the board from your perspective)
inline Rank relativeRank(Square sq, Color col){
    return getRank(flipIfBlack(sq, col));
}

// Determine if (rank, file) is inside the board
inline bool isInGrid(Rank i, File j){ 
    return i >= 0 and i < 8 and j >= 0 and j < 8;
}

// Get the square representing (rank, file)
inline Square posToSquare(Rank i, File j){
    return i * 8 + j;
}

// Get current time in ms
inline TimePoint getTime(){
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}