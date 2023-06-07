#include <string>
#include <vector>
#include "types.h"

#pragma once

struct uciParams{
    uint64 timeLeft[2] = {0, 0};
    uint64 timeIncr[2] = {0, 0};
    uint64 movesToGo = 0;
    uint64 nodeLim = 1e18;
    bool infiniteSearch = 0;

    depth_t depthLim = maximumPly;
    std::vector<std::string> movesToSearch;
};

// Number of thread global
extern int threadCount;

// FEN of the default position
const std::string startPosFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// UCI conversion functions for printing and debugging
std::string squareToString(square_t sq);
std::string moveToString(move_t move);
move_t stringToMove(std::string move);
char pieceToChar(piece_t p);
piece_t charToPiece(char c);
void printMask(bitboard_t msk);

// Helpers
void doLoop();