#pragma once

#include <string>
#include <vector>
#include "types.h"

// Global number of threads
extern int threadCount;

// FEN of the default position
const std::string startPosFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// 100 fens randomly selected from training data used to measure average nps of engine
const std::vector<std::string> benchFens = {
    "5k2/2p2p2/1bPp2p1/1p4P1/1P6/5B2/r3RPK1/8 b - - 8 40",
    "8/8/8/7k/2r2p2/5K2/1R6/8 w - - 16 71",
    "8/8/1k3K2/8/pb6/5R2/8/8 b - - 2 60",
    "6k1/pb4p1/5p1p/2p5/1bN2P1P/1P1rN3/P4PP1/2R2K2 b - - 3 25",
    "8/5k1K/8/8/p7/N7/8/8 b - - 1 80",
    "8/6pk/3Bp3/7p/4r3/4n1PP/8/1R4K1 w - - 0 37",
    "1r6/6p1/4Rpk1/7p/7P/5PK1/1P4P1/8 w - - 1 44",
    "2rk1b1r/Bp3ppp/p2p1n2/4p3/P1P1P3/1PNQ1P1q/2KP4/3R1R2 w - - 6 19",
    "8/3k4/pB4p1/1pK3P1/1P5P/P7/8/3b4 w - - 1 53",
    "1k6/8/4p3/2N3p1/R2P2pp/6P1/prr5/4R1K1 b - - 1 57",
    "2b5/p3qpk1/1p6/1B1PP2r/5P1P/b5P1/P2Q3K/3R4 w - - 3 36",
    "8/2k5/5r2/8/8/8/3K4/4R3 w - - 34 99",
    "1R6/R4pk1/8/3P1p2/8/8/r4r2/6K1 b - - 0 35",
    "8/6pk/7p/1Q6/2p5/3q3P/6PK/8 b - - 23 57",
    "4r1k1/4q2p/4r1p1/1p3p2/p1p1Pb2/P1P5/1PQ1R1PP/1B2R2K w - - 0 35",
    "6r1/4k1p1/1p2p3/pb2P2B/4P1RP/8/PP3K2/8 b - - 4 40",
    "r1bbk2r/pp3ppp/8/8/2p1B3/P5P1/1P2nP1P/R1B1NR1K b kq - 0 15",
    "r4r1k/pb2b1p1/1p4Q1/5p2/3n3q/P1N4P/1P2BPP1/R1B2RK1 b - - 2 21",
    "r1b1k2r/2pq1ppn/p2p4/1p1Pp2p/2P1P3/2N1Q2P/PP3PP1/2RB1RK1 b kq - 0 16",
    "2r3k1/pp3pp1/4b2p/3p4/3Bn3/1P4P1/P3PPBP/R4K2 b - - 2 21",
    "4k3/5pp1/3P3p/1p6/1Pr5/5P1P/3R2P1/6K1 w - - 1 33",
    "Q7/8/5k2/R7/2K5/8/5r2/6q1 w - - 0 47",
    "R7/p1p2k2/6p1/P1b1Kp1p/7P/6P1/8/8 w - - 1 50",
    "8/4r1n1/8/5k2/7Q/8/8/7K b - - 26 164",
    "4K3/6k1/6p1/p1p3P1/P1P4P/1P3B2/3b4/8 w - - 27 87",
    "5q2/6k1/2Q2bp1/7p/7P/6PN/5PK1/8 b - - 5 52",
    "r5rk/2n4p/8/p1Np4/1p4q1/6P1/PP3QBK/5R2 w - - 0 30",
    "8/6p1/5p1k/2q2P1p/8/1B3QP1/3p2KP/8 w - - 2 59",
    "1k1r4/pq4p1/2p1Q2p/2N1p3/8/7P/PP3PPK/8 b - - 0 33",
    "8/8/Pb2n3/5k1B/5p1B/4p3/4K3/8 w - - 1 71",
    "3r3k/3P1qpp/6r1/2p1Pp2/p4P2/5Q2/4BKP1/2BR4 w - - 3 42",
    "8/6k1/7p/1R1N2p1/1p6/2b2K2/2r5/8 w - - 8 55",
    "2qrr1k1/1p2npp1/pbn4p/8/PP1p4/3N2PP/2PBQPB1/1R2R1K1 w - - 2 24",
    "3r4/p2qb1k1/Q4p1n/8/5B1p/P1Np3P/1P3PP1/R5K1 b - - 1 30",
    "5k2/r2n1p2/1rp2np1/pb2p2p/4P3/1P3NP1/2PN1P1P/R3RBK1 w - - 0 23",
    "2rq1rk1/1b2bpp1/ppn1p2p/3p4/P2Pn3/1P2PN1P/4BPP1/RNBQ1RK1 w - - 5 14",
    "4r3/5k2/5P2/8/1K6/2R5/8/8 b - - 22 114",
    "6k1/3r1p1p/1pq3p1/p1r5/3p3P/P5P1/1PQR1P2/3R2K1 w - - 5 34",
    "r2qr1k1/pp1b1ppp/5n2/2n1p3/2Pp4/3P1NP1/P2NPPBP/R1Q2RK1 w - - 4 14",
    "6k1/5p2/4q3/P1b1p3/3pP1p1/1r4P1/4QBKP/R7 b - - 1 39",
    "6k1/3P2p1/1rn4p/p7/P3P3/1P6/2R4P/6K1 b - - 2 37",
    "5k2/R4p1r/p1r3n1/1p2pRP1/7p/P1B5/1PP3P1/5K2 w - - 10 38",
    "6k1/1R1nbppp/2p1pn2/3p4/3P1BP1/4PN1P/2rN1P2/6K1 b - - 0 28",
    "2k2r2/1p6/2p1p3/4R3/p3P1P1/P3K2P/1P2RP2/7r w - - 1 35",
    "r3k3/5ppp/8/p1R5/P2P1KPP/b3P3/5P2/8 w - - 1 37",
    "r3r1k1/3q1pbp/4p1p1/p1p5/2Pp1P2/1P1Q2P1/P2BR2P/5RK1 w - - 0 21",
    "8/5n2/8/2k4P/2p3P1/4K3/8/8 w - - 2 55",
    "1k5r/1pp3pp/r2b2q1/p7/3Q1B2/P2P3P/1P4P1/2R2RK1 b - - 2 25",
    "1k3r2/1pq5/3R4/pRP1pB1p/P3P2p/2K5/6PP/8 w - - 3 39",
    "4K3/r7/3R3P/5k2/8/8/8/8 b - - 62 67",
    "2r3k1/1p3pp1/p4b2/B2p3p/3PnPP1/1P1N3P/R7/6K1 w - - 1 22",
    "r4r1k/p6p/8/4qN2/5R2/P4Q1P/5PP1/6K1 w - - 5 35",
    "1r4k1/2p1qpn1/B1Q3n1/4Pb1p/5P2/6P1/PP3R1P/3R2K1 w - - 4 27",
    "2R5/5pkp/6p1/p2p4/1p1Pr3/1P5P/P4PP1/5K2 w - - 0 38",
    "8/1kb5/8/7r/3NK3/2P5/8/8 b - - 8 38",
    "r3r1k1/7p/pqb2bp1/2Np1p2/1p1P1P2/5Q2/PPBR2PP/5RK1 w - - 4 23",
    "5rk1/ppb3rp/4p3/8/PP2B3/4nNPK/4P3/2R4R w - - 1 25",
    "5n2/R2r1pp1/5P2/3P4/6k1/8/8/6KB b - - 0 48",
    "8/p5k1/1rp3p1/8/6R1/P3P1P1/1r3PK1/2R5 w - - 0 31",
    "8/5ppk/4p3/3pP3/3BbPP1/7P/3R3K/2r5 w - - 3 49",
    "rnbqr1k1/1p3pp1/pN1b1n1p/8/1P1B4/P2P2P1/2Q1PPBP/R4RK1 b - - 2 18",
    "8/8/8/3N2k1/7p/5p2/R6K/4r3 w - - 4 68",
    "r1b1r1k1/2qnbpp1/2p1pn1p/ppPp4/1P1P3P/P1NBPN2/1B3PP1/R2QK2R w KQ - 2 13",
    "5r2/p3r2k/1p1q2p1/2p1p2p/P1PbR2P/5PB1/4R1PK/2Q5 b - - 20 53",
    "r6k/ppq1p1b1/2n1b2p/3pP1p1/P1p5/B1P1Q3/2P1BPP1/1R2R1K1 b - - 1 22",
    "r2r2k1/pp2qppp/2ppn3/4p2n/1PP1P3/P1NQB2P/2P2PP1/R3R1K1 w - - 4 16",
    "3R4/1r4p1/p1p1k3/PbP4p/4p2P/P3B3/5PPK/8 w - - 0 33",
    "r4k1r/pbp2p2/1p5p/5Rq1/2P3pN/2NQ2P1/PP4PP/6K1 b - - 0 22",
    "8/r4p1R/4k3/5R2/r5P1/5PK1/8/8 w - - 5 60",
    "2k2brr/1p3qp1/p1p1pn2/3p1p1p/2PP1P2/1P2PQ1B/PB4R1/2KN3R w - - 1 20",
    "7r/5pk1/2qpp2p/1p4pR/r1pP2P1/P1P2PQ1/5PK1/R7 w - - 6 36",
    "r2r2k1/1p1bnpbp/4p1p1/pP6/P1N1P3/5NP1/4BPP1/1R4KR b - - 0 20",
    "8/1p2r1PB/3p4/2k5/2b5/8/2K5/6R1 b - - 5 50",
    "8/5p2/6bk/3p1p1p/qb1P1P1P/2r1P3/1QpNBKP1/2R5 b - - 4 32",
    "r5k1/1pp2p2/5p1p/8/p2Pq3/2P5/P5PP/R1Q3K1 b - - 1 23",
    "8/5kp1/7p/2r1PB2/8/1b4KP/2p3P1/2R5 w - - 9 39",
    "2rqr1k1/pp3pp1/5n2/b2ppb2/N3P2P/PP1P4/1BQ2PBP/2R2RK1 w - - 1 19",
    "8/1R3pk1/2R3pp/1p6/5n2/3P1B1P/r4rP1/6K1 w - - 1 42",
    "8/8/3k3p/PR4pP/4KpP1/r7/8/8 b - - 4 29",
    "8/1k3ppp/2pBp3/3pP3/n2P4/5K2/5PPP/8 b - - 1 35",
    "7R/4kp2/3r4/8/2pPB1b1/2P3K1/8/8 b - - 1 18",
    "R7/1p2b1p1/8/5k2/1P5P/p1B3P1/r7/3K4 b - - 2 16",
    "r3r1k1/p4pp1/2p4p/6q1/N1P1n1B1/P6P/1P3P2/R2QRK2 b - - 2 21",
    "6k1/3R2p1/2P4r/3P4/8/1R4K1/8/5r2 w - - 5 58",
    "r4rk1/p5pp/6p1/2p5/2P5/8/PP1R1PPK/4R3 w - - 0 23",
    "5k2/7R/6P1/5P2/5K2/8/8/r7 w - - 17 48",
    "8/8/3k1p2/5r2/1R1B2P1/8/4KP2/7r b - - 0 60",
    "8/5pk1/2B4p/2p5/R4n1p/2r2P2/5P2/7K b - - 3 38",
    "8/4n1pk/4P1Rp/3p1r1P/3P2K1/2P1R3/5P2/2r5 w - - 2 59",
    "8/5p2/2p5/2P2kp1/pB5p/Pb3P1P/5KP1/8 b - - 3 50",
    "1r6/5b2/1p1p1kp1/1PpP1p1p/p1P1pPnP/P1R1P1K1/6N1/1R6 w - - 10 47",
    "1R6/8/4k3/4p3/r7/4K3/8/8 w - - 18 71",
    "1k6/8/p7/2RB1p2/1K6/PP4P1/3r3P/3b4 w - - 3 48",
    "8/5pkn/6p1/3pN3/7P/1r3PP1/1P1R2K1/8 w - - 3 34",
    "8/8/3r1k2/5pp1/7p/R6P/6PK/8 b - - 47 68",
    "r6k/2R5/6K1/6P1/8/8/8/8 b - - 22 84",
    "r4rk1/3b2p1/2p1q3/3p1p2/p2P3b/P3PR2/1PQB2BP/5RK1 w - - 0 25",
    "r1bq1rk1/1p2bppp/2np1n2/p3pP2/4P3/PNNBBQ2/1PP3PP/3R1RK1 b - - 0 14",
    "1n1r1rk1/q4pp1/1b1Np2p/p3P3/Pp3B1P/8/1PR1QPP1/2R3K1 b - - 4 25",
    "8/p1k5/2P2p2/1K1B1P2/2P4p/P1P4n/8/8 w - - 0 39",
    "6k1/7p/6p1/8/8/4P2P/3b1PP1/5K2 b - - 1 52"
};

struct uciSearchLims{
    // Note that 0 indicates that the value is not given
    TimePoint timeLeft[2];
    TimePoint timeIncr[2];
    int movesToGo;

    Depth depthLim;
    TimePoint moveTime;
    uint64 nodeLim;
    bool infinite;
};

// UCI conversion functions and other utility functions
char pieceToChar(Piece p);
Piece charToPiece(char c);
std::string squareToString(Square sq);
std::string moveToString(Move move);
Move stringToMove(std::string move);

// Bench
void bench();

// UCI driver
void doLoop();