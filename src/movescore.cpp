#include "types.h"
#include "helpers.h"
#include "movepick.h"
#include "search.h"
#include "movescore.h"

const static Movescore mvvlva[7][7] = {
    {},                                      // Victim: None
    {0, 2050, 2040, 2030, 2020, 2010, 2000}, // Victim: Pawn
    {0, 3050, 3040, 3030, 3020, 3010, 3000}, // Victim: Knight
    {0, 4050, 4040, 4030, 4020, 4010, 4000}, // Victim: Bishop
    {0, 5050, 5040, 5030, 5020, 5010, 5000}, // Victim: Rook
    {0, 6050, 6040, 6030, 6020, 6010, 6000}, // Victim: Queen
    {}                                       // Victim: King
};

const static Movescore maximumHist = 16384;

const static Movescore ttMoveScore            = 2000000000;
const static Movescore promoScore             = 1900000000;
const static Movescore goodCaptScore          = 1800000000;
const static Movescore primaryKillerScore     = 1700000000;
const static Movescore secondaryKillerScore   = 1600000000;
const static Movescore counterScore           = 1500000000;
const static Movescore nonspecialMoveScore    = 1400000000;
const static Movescore badCaptScore           = 1300000000;

const static Movescore promoScoreBonus[7] = { 0, 0, 1, 2, 3, 4, 0 };

static inline void updateHistoryValue(Movescore &cur, Movescore bonus){
    // We use history gravity which means we scale based on its current magnitude
    // new bonus = bonus * (1 - |value| / max value)
    cur += bonus - static_cast<uint64>(abs(bonus)) * static_cast<uint64>(cur) / maximumHist;
}

static inline Movescore calcBonus(Depth depth){
    return std::min(8 * depth * depth + 32 * std::max(depth - 1, 0), 1200);
}

Movescore getQuietHistory(Move move, Depth ply, position &board, searchData &sd, searchStack *ss){
    Movescore score = 0;

    score += sd.history[board.getTurn()][moveFrom(move)][moveTo(move)];
    
    if (ply >= 1 and (ss - 1)->move != nullOrNoMove){
        score += 2 * (*(ss - 1)->contHist)[board.movePieceEnc(move)][moveTo(move)];
    }
    if (ply >= 2 and (ss - 2)->move != nullOrNoMove){
        score += 2 * (*(ss - 2)->contHist)[board.movePieceEnc(move)][moveTo(move)];
    }
    return score;
}

void updateAllHistory(Move bestMove, moveList &quiets, Depth depth, Depth ply, position &board, searchData &sd, searchStack *ss){
    // Set killer
    if (bestMove != sd.killers[ply][0]){
        sd.killers[ply][1] = sd.killers[ply][0];
        sd.killers[ply][0] = bestMove;
    }
    // Set counter
    if (ply >= 1 and (ss - 1)->move != nullOrNoMove){
        *(ss - 1)->counter = bestMove;
    }
    // Update history values by penalizing failing moves and incrementing best move
    for (int i = 0; i < quiets.sz; i++){
        Move move = quiets.moves[i].move;
        Movescore bonus = calcBonus(depth) * (move == bestMove ? 1 : -1);
            
        updateHistoryValue(sd.history[board.getTurn()][moveFrom(move)][moveTo(move)], bonus);

        if (ply >= 1 and (ss - 1)->move != nullOrNoMove){
            updateHistoryValue((*(ss - 1)->contHist)[board.movePieceEnc(move)][moveTo(move)], bonus);
        }   
        if (ply >= 2 and (ss - 2)->move != nullOrNoMove){
            updateHistoryValue((*(ss - 2)->contHist)[board.movePieceEnc(move)][moveTo(move)], bonus);
        }
    }   
}

void scoreMoves(moveList &moves, Move ttMove, Depth ply, position &board, searchData &sd, searchStack *ss){
    for (int i = 0; i < moves.sz; i++){
        // Step 0) Variables
        Move move = moves.moves[i].move;
        Movescore &score = moves.moves[i].score;

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
                  + mvvlva[board.moveCaptType(move)][board.movePieceType(move)];
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
        else if (ply >= 1 and (ss - 1)->move != nullOrNoMove and move == *((ss - 1)->counter)){
            score = counterScore;
        }

        // Step 7) Rest of the moves
        else{
            score = nonspecialMoveScore + getQuietHistory(move, ply, board, sd, ss);
        }
    }
}