#pragma once

#include <string>
#include <vector>
#include "types.h"

// Global number of threads
extern int threadCount;

// FEN of the default position
const std::string startPosFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct uciSearchLims{
    uint64 timeLeft[2];
    uint64 timeIncr[2];
    uint64 movesToGo;
};

// UCI conversion functions and other utility functions
char pieceToChar(Piece p);
Piece charToPiece(char c);
std::string squareToString(Square sq);
std::string moveToString(Move move);
Move stringToMove(std::string move);

// UCI driver
void doLoop();