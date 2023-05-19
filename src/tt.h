#include "types.h"
#include "helpers.h"
#include "assert.h"

#pragma once

// We store buckets of size 4.

// Memory is 16 bytes:
//     zhash: 8
//     score: 2
//     static eval: 2
//     best move: 2
//     depth: 1
//     age and bound: 1 (bits [0...5] are age and [6...7] is bound)

const int clusterSize = 4;

const ttFlagAge_t boundExact = 0; 
const ttFlagAge_t boundLower = 64;
const ttFlagAge_t boundUpper = 128;
const ttFlagAge_t ageCycle = 63;

const ttFlagAge_t ageBits = 63;
const ttFlagAge_t boundBits = 192;

struct ttEntry{
    ttKey_t zhash;
    score_t score;
    score_t staticEval;
    move_t bestMove;
    depth_t depth;
    ttFlagAge_t ageAndBound;

    // Empty slot constructor
    ttEntry(){
        zhash = noHash;
        score = staticEval = noScore;
        bestMove = nullOrNoMove;
        depth = ageAndBound = 0;
    }

    // Entry constructor
    ttEntry(ttKey_t zhash_, score_t score_, score_t staticEval_, move_t bestMove_, depth_t depth_, ttFlagAge_t ageAndBound_):
        zhash(zhash_),
        score(score_),
        staticEval(staticEval_),
        bestMove(bestMove_),
        depth(depth_),
        ageAndBound(ageAndBound_)
    {}
};

struct ttCluster{
    ttEntry dat[clusterSize];
};

struct ttStruct{
    int64_t sz;
    int64_t maskMod;
    ttFlagAge_t currentAge;
    
    ttCluster *table;

    void clearTT();
    void setSize(uint64_t megabytes);
    bool probe(ttKey_t zhash, ttEntry &tte, depth_t ply);

    void addToTT(ttKey_t zhash, score_t score, score_t staticEval, move_t bestMove, depth_t depth, depth_t ply, ttFlagAge_t bound, bool pvNode);
    void incrementAge();
    int hashFullness();
};

// Our global TT

extern ttStruct globalTT;

// ttRngPiece : Hash for each [Piece][Color][Square]
// ttRngTurn  : Hash for turn -- xor if black
// ttRngEnpass: Hash for en passant file; size is 15 since nullEnpass is 15
// ttRngCastle: Hash for each castle mask

extern ttKey_t ttRngPiece[7][2][64]; 
extern ttKey_t ttRngTurn;
extern ttKey_t ttRngEnpass[16];
extern ttKey_t ttRngCastle[16];

// Bits [0...5] are age and [6...7] is bound

inline ttFlagAge_t encodeAgeAndBound(ttFlagAge_t age, ttFlagAge_t bound){
    return age + bound;
}

inline ttFlagAge_t decodeBound(ttFlagAge_t val){
    return (val & boundBits);
}

inline ttFlagAge_t decodeAge(ttFlagAge_t val){
    return (val & ageBits);
}

// We want mate scores to be relative to the subtree searched.
// For example if we are at a subtree with root at 5 and our score is (mateScore - 10) relative
// to the entire search tree, then our score is (mateScore - 5) relative to the ply 5 node.

inline score_t scoreToTT(score_t score, depth_t rootPly){
    if (abs(score) >= foundMate) 
        return score > 0 ? score + rootPly : score - rootPly;
    return score;
}

inline score_t scoreFromTT(score_t score, depth_t rootPly){
    if (abs(score) >= foundMate) 
        return score > 0 ? score - rootPly : score + rootPly;
    return score;
}

// Quality

inline int quality(ttEntry entry, ttFlagAge_t ttCurrentAge){
    // If nothing is there then set quality to 0 (which is lowest)
    if (entry.zhash == noHash) return 0;

    // Be careful about age "wrapping around" and add ageCycle+1 to make age positive
    int age = static_cast<int>(decodeAge(entry.ageAndBound)) + static_cast<int>(ageCycle) + 1;
    if (age > ttCurrentAge) age -= ageCycle;
    assert(age > 0);

    // Quality is age * 4 + depth
    return age * 4 + entry.depth;
}

// Generate TT keys

void initTT();