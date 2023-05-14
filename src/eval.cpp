#include "board.h"
#include "types.h"
#include "helpers.h"

bool position::drawByRepetition(depth_t searchPly){
    // True if one of the following (sensitive to en passant square and castle rights) is true
    // a) Twofold rep in current search (matching ancestor in search)
    // b) Threefold rep if matching ancestors are not in the search
    
    int8 counter = 0;
    boardState *curPos = stk;

    for (int i = 2; i <= stk->halfMoveClock; i += 2){
        // If we are at [0] or [1] then break
        if (curPos == historyArray or (curPos - 1) == historyArray)
            break;
        
        // Decrement
        curPos -= 2;

        // Repetition check
        if (stk->zhash == curPos->zhash)
            if (i <= searchPly or ++counter == 2)
                return true;
    }
    return false;
}

bool position::drawByFiftyMoveRule(){
    // Note that checkmate has a higher priority. TBH it doesn't make much of a difference...
    return stk->halfMoveClock >= 100;
}

bool position::drawByInsufficientMaterial(){
    // Draw: KvK, KvN, KvB (KvNN is not an automatic draw by some rules)
    // To do this, we check if we have 2 pieces (the 2 kings) or 3 pieces (including the 2 kings)
    // and one of the pieces is a minor.

    return (countOnes(allBB) == 2)
           or (countOnes(allBB) == 3 and (allPiece(knight) | allPiece(bishop)));
}

score_t position::eval(){
    return stk->nnue.eval(turn);
}