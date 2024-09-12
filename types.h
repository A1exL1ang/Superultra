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
const Piece noPiece = 0;
const Piece pawn = 1;
const Piece knight = 2;
const Piece bishop = 3;
const Piece rook = 4;
const Piece queen = 5;
const Piece king = 6;

// Colors
const Color white = 0;
const Color black = 1;

// Mask where every bit is set
const Bitboard all = -1;

// Score constants
const Score checkMateScore = 32000;
const Score foundMate = 28000;
const Score pieceScore[7] = {0, 99, 334, 346, 544, 1032, 0};

// Null constants
const Score noScore = 32001;
const Move nullOrNoMove = 0;
const TTKey noHash = 0;

// Maximum size constants
const int maxMovesInTurn = 250;
const Depth maximumPly = 100;

// No enpassant
const File noEP = 15;

// Castling Bits
const int8 castleWhiteK = (1 << 0);
const int8 castleWhiteQ = (1 << 1);
const int8 castleBlackK = (1 << 2);
const int8 castleBlackQ = (1 << 3);

// Squares
enum{
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
enum{
    fileA, fileB, fileC, fileD, fileE, fileF, fileG, fileH
};

// Ranks
enum{
    rank1, rank2, rank3, rank4, rank5, rank6, rank7, rank8
};
