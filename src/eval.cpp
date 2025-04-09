#include "board.h"
#include "types.h"
#include "helpers.h"

bool Position::drawByRepetition(Depth searchPly){
    // True if one of the following (sensitive to en passant square and castle rights) is true
    // a) Twofold rep in current search (matching ancestor in search)
    // b) Threefold rep if matching ancestors are not in the search
    
    int8 counter = 0;
    int curPos = stk;

    for (int i = 2; i <= pos[stk].halfMoveClock; i += 2){
        // If we are at [0] or [1] then break
        if (curPos <= 1){
            break;
        }
        // Decrement
        curPos -= 2;

        // Repetition check
        if (pos[stk].zhash == pos[curPos].zhash){
            if (i <= searchPly or ++counter == 2){
                return true;
            }
        }
    }
    return false;
}

bool Position::drawByFiftyMoveRule(){
    // Note that checkmate has a higher priority. Very hard to deal with especially when
    // it comes to TT so we often just ignore it
    
    return pos[stk].halfMoveClock >= 100;
}

bool Position::drawByInsufficientMaterial(){
    // Draw: KvK, KvN, KvB (KvNN is not an automatic draw by some rules)
    // To do this, we check if we have 2 pieces (the 2 kings) or 3 pieces (including the 2 kings)
    // and one of the pieces is a minor

    return (countOnes(allBB) == 2)
           or (countOnes(allBB) == 3 and (allPiece(KNIGHT) | allPiece(BISHOP)));
}

Score Position::eval(){
    return pos[stk].nnue.eval(turn, countOnes(allBB));
}