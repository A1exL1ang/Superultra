#pragma once

#include "types.h"
#include "helpers.h"

struct moveInfo{
    Move move;
    Movescore score;
};

struct moveList{
    int sz = 0;
    moveInfo moves[maxMovesInTurn];

    inline void addMove(Move move){
        moves[sz++].move = move;
    }
    inline void bringBest(int start){
        int best = start;  

        for (int i = start + 1; i < sz; i++){
            if (moves[i].score > moves[best].score){
                best = i;
            }
        }
        std::swap(moves[start], moves[best]);
    }
};

// Bits [0 ... 5] Start
inline Square moveFrom(Move move){
    return (move & 63);
}

// Bits [6 ... 11] End
inline Square moveTo(Move move){
    return ((move >> 6) & 63);
}

// Bits [12 ... 14] Promo
inline Piece movePromo(Move move){
    return ((move >> 12) & 7);
}

// Encode based on the above encoding
inline Move encodeMove(Square st, Square en, Square promo){
    return st + (static_cast<Move>(en) << 6) + (static_cast<Move>(promo) << 12);
}