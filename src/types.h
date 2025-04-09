#pragma once

#include <iostream>
#include <cstdint>
#include <string>

// Shortcuts
using int8 = int8_t;
using uint8 = uint8_t;

using int16 = int16_t;
using uint16 = uint16_t;

using int32 = int32_t;
using uint32 = uint32_t;

using int64 = int64_t;
using uint64 = uint64_t;

// Aliases to help keep everything consistent
using Color = bool;

using Piece = int8;
using Square = int8;
using File = int8;
using Rank = int8;
using Depth = int8;

using Move = uint16;

using TTboundAge = uint8; 

using Score = int16;
using Movescore = int32;
using NNUEWeight = int16;

using TimePoint = int64;
using Bitboard = uint64;
using TTKey = uint64;

// Pieces: Bit 0 is the color, Bits [1...3] is the piece
const Piece NO_PIECE = 0;
const Piece PAWN = 1;
const Piece KNIGHT = 2;
const Piece BISHOP = 3;
const Piece ROOK = 4;
const Piece QUEEN = 5;
const Piece KING = 6;

// Colors
const Color WHITE = 0;
const Color BLACK = 1;

// Mask where every bit is set
const Bitboard ALL = -1;

// Score constants
const Score CHECKMATE_SCORE = 32000;
const Score FOUND_MATE = 28000;
const Score PIECE_SCORE[7] = {0, 99, 334, 346, 544, 1032, 0};

// Null constants
const Score NO_SCORE = 32001;
const Move NULL_OR_NO_MOVE = 0;
const TTKey NO_HASH = 0;

// Maximum size constants
const int MAX_MOVES_IN_TURN = 250;
const Depth MAX_PLY = 100;

// No enpassant
const File NO_EP = 15;

// Castling Bits
const int8 CASTLE_WHITE_K = (1 << 0);
const int8 CASTLE_WHITE_Q = (1 << 1);
const int8 CASTLE_BLACK_K = (1 << 2);
const int8 CASTLE_BLACK_Q = (1 << 3);

// Squares
enum{
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8
};

// Files
enum{
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

// Ranks
enum{
    RANK_1, RANK2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};