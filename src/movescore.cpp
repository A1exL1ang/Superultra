#include "types.h"
#include "helpers.h"
#include "movepick.h"
#include "search.h"
#include "movescore.h"

const static movescore_t mvv[7] = {
    0, 100000, 200000, 200000, 300000, 400000, 0
};

const static movescore_t maximumHist = 16384;

const static movescore_t ttMoveScore            = 2000000000;
const static movescore_t promoScore             = 1900000000;
const static movescore_t goodCaptScore          = 1800000000;
const static movescore_t primaryKillerScore     = 1700000000;
const static movescore_t secondaryKillerScore   = 1600000000;
const static movescore_t counterScore           = 1500000000;
const static movescore_t nonspecialMoveScore    = 1400000000;
const static movescore_t badCaptScore           = 1300000000;

const static movescore_t promoScoreBonus[7] = { 0, 0, 1, 2, 3, 4, 0 };

void scoreMoves(moveList &moves, move_t ttMove, depth_t ply, position &board, searchData &sd, searchStack *ss){
    for (int i = 0; i < moves.sz; i++){
        // Step 0) Variables
        move_t move = moves.moves[i].move;
        movescore_t &score = moves.moves[i].score;

        // Step 1) ttMove
        if (move == ttMove){
            score = ttMoveScore;
        }

        // Step 2) Promotion
        else if (movePromo(move)){
            score = promoScore + promoScoreBonus[movePromo(move)];
        }

        // Step 3) Capture
        else if (board.moveCaptType(move)){
            score = (board.seeGreater(move, -50) ? goodCaptScore : badCaptScore) 
                  + mvv[board.moveCaptType(move)]
                  + getCaptHistory(move, ply, board, sd, ss);
        }

        // Step 4) Killer 1
        else if (move == sd.killers[ply][0]){
            score = primaryKillerScore;
        }

        // Step 5) Killer 2
        else if (move == sd.killers[ply][1]){
            score = secondaryKillerScore;
        }

        // Step 6) Countermove
        // else if (ply >= 1 and (ss - 1)->move != nullOrNoMove and move == *((ss - 1)->counter)){
        //     score = counterScore;
        // }

        // Step 6) Rest of the moves
        else{
            score = nonspecialMoveScore + getQuietHistory(move, ply, board, sd, ss);
        }
    }
}

static inline void updateHistoryValue(movescore_t &cur, movescore_t bonus){
    // We use history gravity which means we scale based on its current magnitude
    // new bonus = bonus * (1 - |value| / max value)
    cur += bonus - static_cast<uint64>(abs(bonus)) * static_cast<uint64>(cur) / maximumHist;
}

static inline movescore_t calcBonus(depth_t depth){
    return std::min(8 * depth * depth + 32 * std::max(depth - 1, 0), 1200);
}

void updateAllHistory(move_t bestMove, moveList &quiets, moveList &capts, depth_t depth, depth_t ply, position &board, searchData &sd, searchStack *ss){
    
    // If best move was a quiet move
    if (board.moveCaptType(bestMove) == noPiece and movePromo(bestMove) == noPiece){
        // Set killer
        if (bestMove != sd.killers[ply][0]){
            sd.killers[ply][1] = sd.killers[ply][0];
            sd.killers[ply][0] = bestMove;
        }
        // Set counter
        *(ss - 1)->counter = bestMove;

        // Update history values by penalizing failing moves and incrementing best move
        for (int i = 0; i < quiets.sz; i++){
            move_t move = quiets.moves[i].move;
            movescore_t bonus = calcBonus(depth) * (move == bestMove ? 1 : -1);
            
            updateHistoryValue(sd.history[board.getTurn()][moveFrom(move)][moveTo(move)], bonus);

            if (ply >= 1 and (ss - 1)->move != nullOrNoMove)
                updateHistoryValue((*(ss - 1)->contHist)[board.movePieceEnc(move)][moveTo(move)], bonus);
            
            if (ply >= 2 and (ss - 2)->move != nullOrNoMove)
                updateHistoryValue((*(ss - 2)->contHist)[board.movePieceEnc(move)][moveTo(move)], bonus);

            if (ply >= 4 and (ss - 4)->move != nullOrNoMove)
                updateHistoryValue((*(ss - 4)->contHist)[board.movePieceEnc(move)][moveTo(move)], bonus);
        }   
    }
    // Update capture history
    for (int i = 0; i < capts.sz; i++){
        move_t move = capts.moves[i].move;
        movescore_t bonus = calcBonus(depth) * (move == bestMove ? 1 : -1);
        
        updateHistoryValue(sd.chist[board.movePieceEnc(move)][moveTo(move)][board.moveCaptType(move)], bonus);
    }
    
}

movescore_t getQuietHistory(move_t move, depth_t ply, position &board, searchData &sd, searchStack *ss){
    movescore_t score = 0;

    score += sd.history[board.getTurn()][moveFrom(move)][moveTo(move)];

    if (ply >= 1 and (ss - 1)->move != nullOrNoMove)
        score += 2 * (*(ss - 1)->contHist)[board.movePieceEnc(move)][moveTo(move)];

    if (ply >= 2 and (ss - 2)->move != nullOrNoMove)
        score += 2 * (*(ss - 2)->contHist)[board.movePieceEnc(move)][moveTo(move)];
    
    if (ply >= 4 and (ss - 4)->move != nullOrNoMove)
        score += (*(ss - 4)->contHist)[board.movePieceEnc(move)][moveTo(move)];

    return score;
}

movescore_t getCaptHistory(move_t move, depth_t ply, position &board, searchData &sd, searchStack *ss){
    return sd.chist[board.movePieceEnc(move)][moveTo(move)][board.moveCaptType(move)];
}