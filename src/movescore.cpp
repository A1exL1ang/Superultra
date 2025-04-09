#include "types.h"
#include "helpers.h"
#include "movepick.h"
#include "search.h"
#include "movescore.h"

static inline void updateHistoryValue(Movescore &cur, Movescore bonus){
    // We use history gravity which means we scale based on its current magnitude
    // new bonus = bonus * (1 - |value| / max value)
    cur += bonus - static_cast<uint64>(abs(bonus)) * static_cast<uint64>(cur) / MAXIMUM_HIST;
}

static inline Movescore calcBonus(Depth depth){
    return std::min(8 * depth * depth + 32 * std::max(depth - 1, 0), 1200);
}

Movescore getQuietHistory(Move move, Depth ply, Position &board, SearchData &sd, SearchStack *ss){
    Movescore score = 0;

    score += sd.history[board.getTurn()][moveFrom(move)][moveTo(move)];
    
    if (ply >= 1 and (ss - 1)->move != NULL_OR_NO_MOVE){
        score += 2 * (*(ss - 1)->contHist)[board.movePieceEnc(move)][moveTo(move)];
    }
    if (ply >= 2 and (ss - 2)->move != NULL_OR_NO_MOVE){
        score += 2 * (*(ss - 2)->contHist)[board.movePieceEnc(move)][moveTo(move)];
    }
    return score;
}

void updateAllHistory(Move bestMove, moveList &quiets, Depth depth, Depth ply, Position &board, SearchData &sd, SearchStack *ss){
    // Set killer
    if (bestMove != sd.killers[ply][0]){
        sd.killers[ply][1] = sd.killers[ply][0];
        sd.killers[ply][0] = bestMove;
    }
    // Set counter
    if (ply >= 1 and (ss - 1)->move != NULL_OR_NO_MOVE){
        *(ss - 1)->counter = bestMove;
    }
    // Update history values by penalizing failing moves and incrementing best move
    for (int i = 0; i < quiets.sz; i++){
        Move move = quiets.moves[i].move;
        Movescore bonus = calcBonus(depth) * (move == bestMove ? 1 : -1);
            
        updateHistoryValue(sd.history[board.getTurn()][moveFrom(move)][moveTo(move)], bonus);

        if (ply >= 1 and (ss - 1)->move != NULL_OR_NO_MOVE){
            updateHistoryValue((*(ss - 1)->contHist)[board.movePieceEnc(move)][moveTo(move)], bonus);
        }   
        if (ply >= 2 and (ss - 2)->move != NULL_OR_NO_MOVE){
            updateHistoryValue((*(ss - 2)->contHist)[board.movePieceEnc(move)][moveTo(move)], bonus);
        }
    }   
}

void scoreMoves(moveList &moves, Move ttMove, Depth ply, Position &board, SearchData &sd, SearchStack *ss){
    for (int i = 0; i < moves.sz; i++){
        // Step 0) Variables
        Move move = moves.moves[i].move;
        Movescore &score = moves.moves[i].score;

        // Step 1) ttMove
        if (move == ttMove){
            score = TTMOVE_SCORE;
        }

        // Step 2) Promotion
        else if (movePromo(move)){
            score = PROMO_SCORE + PROMO_SCORE_BONUS[movePromo(move)];
        }

        // Step 3) Capture
        else if (board.moveCaptType(move)){
            score = (board.seeGreater(move, -50) ? GOOD_CAPT_SCORE : BAD_CAPT_SCORE) 
                  + MVV_LVA[board.moveCaptType(move)][board.movePieceType(move)];
        }

        // Step 4) Killer 1
        else if (move == sd.killers[ply][0]){
            score = PRIMARY_KILLER_SCORE;
        }

        // Step 5) Killer 2
        else if (move == sd.killers[ply][1]){
            score = SECONDARY_KILLER_SCORE;
        }

        // Step 6) Countermove
        else if (ply >= 1 and (ss - 1)->move != NULL_OR_NO_MOVE and move == *((ss - 1)->counter)){
            score = COUNTER_SCORE;
        }

        // Step 7) Rest of the moves
        else{
            score = NONSPECIAL_MOVE_SCORE + getQuietHistory(move, ply, board, sd, ss);
        }
    }
}