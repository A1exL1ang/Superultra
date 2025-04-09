#pragma once

#include <memory>
#include "types.h"
#include "helpers.h"
#include "assert.h"

// We store buckets of size 4.

// Memory is 16 bytes:
//     zhash: 8
//     score: 2
//     static eval: 2
//     best move: 2
//     depth: 1
//     age and bound: 1 (bits [0...5] are age and [6...7] is bound)

const int CLUSTER_SIZE = 4;

// Values for age and bound encoding
const TTboundAge BOUND_LOWER = 64;
const TTboundAge BOUND_UPPER = 128;
const TTboundAge BOUND_EXACT = BOUND_LOWER | BOUND_UPPER; 
const TTboundAge AGE_CYCLE = 63;
const TTboundAge AGE_BITS = 63;
const TTboundAge BOUND_BITS = 192;

// Hash values
extern TTKey ttRngPiece[7][2][64]; 
extern TTKey ttRngTurn;
extern TTKey ttRngEnpass[16];
extern TTKey ttRngCastle[16];

struct ttEntry{
    TTKey zhash;
    Score score;
    Score staticEval;
    Move bestMove;
    Depth depth;
    TTboundAge ageAndBound;

    // Empty slot constructor
    ttEntry(){
        zhash = NO_HASH;
        score = staticEval = NO_SCORE;
        bestMove = NULL_OR_NO_MOVE;
        depth = ageAndBound = 0;
    }

    // Regular entry constructor
    ttEntry(TTKey zhash_, Score score_, Score staticEval_, Move bestMove_, Depth depth_, TTboundAge ageAndBound_):
        zhash(zhash_),
        score(score_),
        staticEval(staticEval_),
        bestMove(bestMove_),
        depth(depth_),
        ageAndBound(ageAndBound_)
    {}
};

struct ttCluster{
    ttEntry dat[CLUSTER_SIZE];
};

struct ttStruct{
    int64_t sz;
    int64_t maskMod;
    TTboundAge currentAge;
    std::unique_ptr<ttCluster[]> table;

    void clearTT();
    void setSize(uint64_t megabytes);

    bool probe(TTKey zhash, ttEntry &tte, Depth ply);
    void addToTT(TTKey zhash, Score score, Score staticEval, Move bestMove, Depth depth, Depth ply, TTboundAge bound, bool pvNode);
    
    int hashFullness();

    inline void incrementAge(){
        currentAge = ((currentAge + 1) & AGE_CYCLE);
    }
    inline void prefetch(TTKey zhash){
        __builtin_prefetch(&table[zhash & maskMod]);
    }
};

// Our global TT
extern ttStruct globalTT;

// Bits [0...5] are age and [6...7] is bound
inline TTboundAge encodeAgeAndBound(TTboundAge age, TTboundAge bound){
    return age + bound;
}

// Bits [6...7] is bound
inline TTboundAge decodeBound(TTboundAge val){
    return (val & BOUND_BITS);
}

// Bits [0...5] is age
inline TTboundAge decodeAge(TTboundAge val){
    return (val & AGE_BITS);
}

// We want mate scores to be relative to the subtree searched.
// For example if we are at a subtree with root at 5 and our score is (mateScore - 10) relative
// to the entire search tree, then our score is (mateScore - 5) relative to the ply 5 node.

inline Score scoreToTT(Score score, Depth rootPly){
    if (abs(score) >= FOUND_MATE){
        return score > 0 ? score + rootPly : score - rootPly;
    }
    return score;
}

inline Score scoreFromTT(Score score, Depth rootPly){
    if (abs(score) >= FOUND_MATE){
        return score > 0 ? score - rootPly : score + rootPly;
    }
    return score;
}

inline int quality(ttEntry entry, TTboundAge ttCurrentAge){
    // If nothing is there then set quality to 0 (which is lowest)
    if (entry.zhash == NO_HASH){
        return 0;
    }
    // Be careful about age "wrapping around" and add AGE_CYCLE+1 to make age positive
    int age = static_cast<int>(decodeAge(entry.ageAndBound)) + static_cast<int>(AGE_CYCLE) + 1;

    if (age > ttCurrentAge){
        age -= AGE_CYCLE;
    }
    // Quality is age * 4 + depth
    return age * 4 + entry.depth;
}

// Generate TT keys
void initTT();