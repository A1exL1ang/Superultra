#include <iostream>
#include <cstdint>
#include <string>

#pragma once

// Shortcuts
using int8 = int_fast8_t;
using uint8 = uint_fast8_t;

using int16 = int_fast16_t;
using uint16 = uint_fast16_t;

using int32 = int_fast32_t;
using uint32 = uint_fast32_t;

using int64 = int64_t;
using uint64 = uint64_t;

// Aliases to help keep everything consistent
using color_t = bool;

using piece_t = int8;
using square_t = int8;
using file_t = int8;
using rank_t = int8;
using depth_t = int8;

using ttFlagAge_t = uint8; 

using score_t = int16;
using nnueWeight_t = int16;

using move_t = uint16;

using movescore_t = int32;
using spair_t = int32;

using move_extended_t = uint32;
using timePoint_t = uint64;
using bitboard_t = uint64;
using ttKey_t = uint64;

// Pieces: Bit 0 is the color, Bits [1...3] is the piece
const piece_t noPiece = 0;
const piece_t pawn = 1;
const piece_t knight = 2;
const piece_t bishop = 3;
const piece_t rook = 4;
const piece_t queen = 5;
const piece_t king = 6;

// Colors
const color_t white = 0;
const color_t black = 1;

// Mask where every bit is set
const bitboard_t all = -1;

// Score constants
const score_t checkMateScore = 32000;
const score_t foundMate = 31000;
const score_t pieceScore[7] = {0, 99, 334, 346, 544, 1032, 0};

// Null constants
const score_t noScore = 32001;
const move_t nullOrNoMove = 0;
const ttKey_t noHash = 0;

// Maximum size constants
const int maxMovesInTurn = 250;
const depth_t maximumPly = 100;

// No enpassant
const file_t noEP = 15;

// Castling Bits
const int8 castleWhiteK = (1 << 0);
const int8 castleWhiteQ = (1 << 1);
const int8 castleBlackK = (1 << 2);
const int8 castleBlackQ = (1 << 3);

// Squares
enum {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

// Files
enum {
    fileA, fileB, fileC, fileD, fileE, fileF, fileG, fileH
};

// Ranks
enum {
    rank1, rank2, rank3, rank4, rank5, rank6, rank7, rank8
};