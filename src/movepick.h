#include "types.h"
#include "helpers.h"

#pragma once

struct moveInfo{
    move_t move;
    movescore_t score;
};

struct moveList{
    int sz = 0;
    moveInfo moves[maxMovesInTurn];

    inline void addMove(move_t move){
        moves[sz++].move = move;
    }
    inline void bringBest(int start){
        int best = start;  

        for (int i = start + 1; i < sz; i++)
            if (moves[i].score > moves[best].score)
                best = i;
            
        std::swap(moves[start], moves[best]);
    }
};

// Bits [0 ... 5] Start
inline square_t moveFrom(move_t move){
    return (move & 63);
}

// Bits [6 ... 11] End
inline square_t moveTo(move_t move){
    return ((move >> 6) & 63);
}

// Bits [12 ... 14] Promo
inline piece_t movePromo(move_t move){
    return ((move >> 12) & 7);
}

// Encode based on the above encoding
inline move_t encodeMove(square_t st, square_t en, square_t promo){
    return st + (static_cast<move_t>(en) << 6) + (static_cast<move_t>(promo) << 12);
}