#include "search.h"
#include "timecontrol.h"
#include "movescore.h"
#include <math.h>

static depth_t lmrReduction[maximumPly + 5][maxMovesInTurn];

void initLMR(){
    for (depth_t depth = 1; depth <= maximumPly; depth++){
        for (int i = 0; i < maxMovesInTurn; i++){
            lmrReduction[depth][i] = 1 + log(depth) * log(i + 1) / 1.7;
        }
    }
}

static inline void checkEnd(searchData &sd){
    sd.stopped = sd.T.outOfTime();
}

static inline void adjustEval(ttEntry &tte, score_t &staticEval){
    // Adjust evaluation based on TT (~10 elo)
    // Note that tte must exist and have a score
    if (decodeBound(tte.ageAndBound) == boundExact
        or (decodeBound(tte.ageAndBound) == boundLower and tte.score < staticEval)
        or (decodeBound(tte.ageAndBound) == boundUpper and tte.score > staticEval))
    {
        staticEval = tte.score;
    }
}

template<bool pvNode> static score_t qsearch(score_t alpha, score_t beta, depth_t ply, position &board, searchData &sd, searchStack *ss){

    // 1) Leaf node and misc stuff
    sd.selDepth = std::max(sd.selDepth, ply);
    
    if ((sd.nodes & 2047) == 0)
        checkEnd(sd);

    if (sd.stopped)
        return 0;

    if (board.drawByRepetition(ply) or board.drawByInsufficientMaterial() or board.drawByFiftyMoveRule())
        return 0;
    
    if (ply > maximumPly)
        return board.eval();

    // 2) Probe the TT and initalize variables

    ttEntry tte = ttEntry();
    bool foundEntry = globalTT.probe(board.getHash(), tte, ply);

    score_t originalAlpha = alpha;
    bool inCheck = board.inCheck();

    ss->staticEval = noScore;

    if (foundEntry
        and !pvNode)
    {
        if (decodeBound(tte.ageAndBound) == boundExact 
            or (decodeBound(tte.ageAndBound) == boundLower and tte.score >= beta)
            or (decodeBound(tte.ageAndBound) == boundUpper and tte.score <= alpha))
        {
            return tte.score;
        }
    }

    // 3) Standing Pat
    if (!inCheck){
        ss->staticEval = foundEntry ? tte.staticEval : board.eval();

        // Adjust static eval based on TT
        // if (foundEntry and tte.score != noScore){
        //     adjustEval(tte, ss->eval);
        // }

        if (ss->staticEval >= beta)
            return ss->staticEval;

        alpha = std::max(alpha, ss->staticEval);
    }

    // 4) Move gen and related 
    moveList moves;
    board.genAllMoves(!inCheck, moves);

    if (moves.sz == 0){
        // We don't consider draws (if there are no quiets then we simply return the eval)
        return inCheck ? -(checkMateScore - ply) : ss->staticEval;
    }
    scoreMoves(moves, foundEntry ? tte.bestMove : nullOrNoMove, ply, board, sd, ss);

    // 5) Iterate over moves
    score_t bestScore = -checkMateScore;
    move_t bestMove = 0;

    for (int i = 0; i < moves.sz; i++){
        // Bring best
        moves.bringBest(i);
        move_t move = moves.moves[i].move;

        // SEE pruning (~6.5 elo)
        if (i > 0 and bestScore > -foundMate and !board.seeGreater(move, -50)){
            continue;
        }

        // Make and update
        
        ss->move = move;
        (ss->counter) = &(sd.counter[board.movePieceEnc(move)][moveTo(move)]);
        (ss->contHist) = &(sd.contHist[board.moveCaptType(move) != noPiece][board.movePieceEnc(move)][moveTo(move)]);
        sd.nodes++;

        board.makeMove(move);

        // Recurse
        score_t score = -qsearch<pvNode>(-beta, -alpha, ply + 1, board, sd, ss + 1);

        // Undo
        board.undoLastMove();

        // Update
        if (score > bestScore){
            bestScore = score;
            bestMove = move;
            
            // Beta cutoff 

            if (score >= beta){
                break;
            }

            // Raise alpha

            if (score > alpha)
                alpha = score;
        }
    }

    // Set best score to be static eval (we do this late so that we always have a best move)
    if (!inCheck)
        bestScore = std::max(bestScore, ss->staticEval);

    // Update TT
    if (!sd.stopped){
        ttFlagAge_t bound = boundExact;

        if (bestScore <= originalAlpha)
            bound = boundUpper;
        else if (bestScore >= beta)
            bound = boundLower;

        globalTT.addToTT(board.getHash(), bestScore, ss->staticEval, bestMove, 0, ply,bound, pvNode);
    }
    return bestScore;
}

template<bool pvNode> static score_t negamax(score_t alpha, score_t beta, depth_t ply, depth_t depth, position &board, searchData &sd, searchStack *ss){
    
    // 1) Leaf node and misc stuff
    
    sd.pvLength[ply] = ply;
    sd.selDepth = std::max(sd.selDepth, ply);
    
    if ((sd.nodes & 2047) == 0)
        checkEnd(sd);
    
    if (sd.stopped)
        return 0;

    if (ply > maximumPly)
        return board.eval();
    
    if (depth <= 0)
        return qsearch<pvNode>(alpha, beta, ply, board, sd, ss);
    
    if (board.drawByRepetition(ply) or board.drawByInsufficientMaterial() or board.drawByFiftyMoveRule())
        return 0;
    
    // Step 2) Mate distance pruning (~0 elo)
    // We prunes trees that have no hope of improving our mate score (if we have one).
    // Upperbound of score for our current node: checkMateScore - ply - 1
    // Lowerbound of score for our current node: -checkMateScore + ply

    if (ply > 0){
        score_t mateUB = checkMateScore - ply - 1;
        score_t mateLB = -checkMateScore + ply;

        // We already found mate (alpha) greater than upperbound
        if (alpha >= mateUB)
            return mateUB;
            
        // Opponent found mate (beta) lower than lowerbound
        if (beta <= mateLB)
            return mateLB;
    }

    // 3) Probe the TT and initialize variables

    ttEntry tte = ttEntry();
    bool foundEntry = globalTT.probe(board.getHash(), tte, ply);

    score_t originalAlpha = alpha;
    bool inCheck = board.inCheck();

    ss->staticEval = noScore;

    if (foundEntry
        and !pvNode
        and tte.depth >= depth)
    {
        if (decodeBound(tte.ageAndBound) == boundExact 
            or (decodeBound(tte.ageAndBound) == boundLower and tte.score >= beta)
            or (decodeBound(tte.ageAndBound) == boundUpper and tte.score <= alpha))
        {
            return tte.score;
        }
    }

    // Step 4) TT move reduction (aka internal "iterative" reduction) (~5 elo)
    // If we don't have a TT move, we reduce by 1. Basically the
    // opposite of internal iterative deepening.

    // Conditions:
    // 1) Depth >= 4
    // 2) No TT move

    if (depth >= 4 and !foundEntry)
        depth--;

    // 4) Static eval and improving

    if (!inCheck){
        ss->staticEval = foundEntry ? tte.staticEval : board.eval();

        // Adjust static eval based on TT
        // if (foundEntry and tte.score != noScore){
        //     adjustEval(tte, ss->eval);
        // }
    }
    bool improving = (!inCheck and (ply >= 2 and ((ss - 2)->staticEval == noScore or ss->staticEval > (ss - 2)->staticEval)));


    // Step 5) Reverse Futility Pruning (~75 elo)
    // If the static evaluation is far above beta, we can assume that
    // it will most definitely fail high!

    // Conditions:
    // 1) Not at PV / root
    // 2) Not in check
    // 3) Enough depth left
    // 4) Static evaluation is far above beta (see code for exact details)

    if (!pvNode 
        and !inCheck
        and depth <= 8
        and ss->staticEval - 77 * std::max(depth - improving, 0) >= beta)
    {
        return ss->staticEval;
    }

    // 7) Null Move Pruning (~60 elo)
    // We evaluate the position if we skipped our turn. 
    // We check if doing so causes a beta cutoff. So we use zero window [beta - 1, beta].
    // Our score will be a lowerbound even if it does.

    // Conditions:
    // 1) Not at PV / root
    // 2) Not in check (doing no move here is illegal)
    // 3) Depth >= 3
    // 4) Static eval greater than beta minus some value
    // 5) Previous move wasn't a null move
    // 6) Have at least one major piece of our color left

    if (!pvNode 
        and !inCheck
        and depth >= 3 
        and ss->staticEval >= beta
        and (ply > 0 and (ss - 1)->move != nullOrNoMove)
        and board.hasMajorPieceLeft(board.getTurn()))
    {
        // Make move and update variables
        ss->move = nullOrNoMove;
        sd.nodes++;

        board.makeNullMove();
        
        // Search with reduced depth
        depth_t R = 3 + (depth / 3) + std::min((ss->staticEval - beta) / 200, 3);
        
        score_t score = -negamax<false>(-beta, -(beta - 1), ply + 1, depth - R, board, sd, ss + 1);

        // Undo
        board.undoNullMove();

        // See if score is above beta
        if (score >= beta){
            // Don't use mate score
            if (score >= foundMate)
                score = beta;

            return score;
        }
    }
    
    // Generate moves and check for draw/stalemate
    moveList moves, quiets, capts;
    board.genAllMoves(false, moves);
     
    if (moves.sz == 0){
        return inCheck ? -(checkMateScore - ply) : 0;
    }

    // Score moves
    scoreMoves(moves, foundEntry ? tte.bestMove : nullOrNoMove, ply, board, sd, ss);

    // Iterate over the moves
    score_t bestScore = -checkMateScore;
    move_t bestMove = 0;

    for (int i = 0; i < moves.sz; i++){
        // Bring best move up
        moves.bringBest(i);

        // Variables related
        move_t move = moves.moves[i].move;
        score_t score = checkMateScore;
        bool isQuiet = movePromo(move) == noPiece and board.moveCaptType(move) == noPiece;

        if (isQuiet)
            quiets.addMove(move);
        else if (board.moveCaptType(move) != noPiece)
            capts.addMove(move);
        
        // Step 10) Move Count Pruning (~13.8 elo)
        // Complements LMR. If we are at low depth and searched enough
        // quiet moves we can stop searching all other quiet moves

        // Conditions to not search current move:
        // 1) Not PV
        // 2) Not in check
        // 3) Current score isn't losing mate score
        // 4) Depth is low
        // 5) Quiet
        // 6) We've searched enough quiets

        if (!pvNode
            and !inCheck
            and bestScore > -foundMate 
            and depth <= 4
            and isQuiet
            and quiets.sz - 1 >= (improving ? 2 * depth * depth + 1 : 4 * depth * depth))
        {
             continue;
        }

        // Step 12) SEE Pruning (~25 elo)
        // We skip moves with a bad SEE...

        // Conditions to not search current move:
        // 1) Not root
        // 2) Current score isn't mate score
        // 3) Enough depth
        // 3) We searched >2 nodes
        // 4) Bad SEE

        if (ply > 0
            and bestScore > -foundMate 
            and depth <= 5
            and i >= 2)
        {
            score_t cutoff = isQuiet ? -60 * depth : -55 * depth;

            if (!board.seeGreater(move, cutoff)){
                continue;
            }
        }

        // Make and update
        
        ss->move = move;
        (ss->counter) = &(sd.counter[board.movePieceEnc(move)][moveTo(move)]);
        (ss->contHist) = &(sd.contHist[board.moveCaptType(move) != noPiece][board.movePieceEnc(move)][moveTo(move)]);
        sd.nodes++;

        board.makeMove(move);

        // Step 16) Late Move Reduction (~150 elo)
        // Later moves are likely to fail low so we search them at a reduced depth
        // with zero window [alpha, alpha + 1]. We do a re-search if it does not fail low.
        
        // Conditions:
        // 1) We are not in check
        // 3) Depth >= 3
        // 4) We searched enough nodes

        if (!inCheck
            and depth >= 3
            and i >= 3 + 2 * pvNode)
        {
            depth_t R = lmrReduction[depth][i];
            
            // Decrease reduction if we are currently at a PV node
            R -= pvNode;

            // Decrease reduction if non-quiet
            R -= !isQuiet;

            // Decrease reduction if special quiet
            R -= (move == sd.killers[ply][0] or move == sd.killers[ply][1]);

            // Increase reduction if we have a bad SEE
            R += !board.seeGreater(move, -50);
            
            // Increase reduction if we aren't improving
            R += !improving;

            // We don't want to directly drop into qsearch after the reduction
            R = std::min(R, static_cast<depth_t>(depth - 1));

            // Reduce if it's worthwhile
            if (R >= 2){
                score = -negamax<false>(-(alpha + 1), -alpha, ply + 1, depth - R, board, sd, ss + 1);
            }
        }

        // Step 16) Principle Variation Search
        // LMR inconclusive or we didn't do LMR (note that score is initializd to inf)

        if (score > alpha){
            // Fully evaluate node if first move
            if (!i){ 
                score = -negamax<true>(-beta, -alpha, ply + 1, depth - 1, board, sd, ss + 1);
            }
            // Otherwise do zero window search: [alpha, alpha + 1]
            else{ 
                score = -negamax<false>(-(alpha + 1), -alpha, ply + 1, depth - 1, board, sd, ss + 1); 
                        
                // Zero window inconclusive (note that its only possible to enter this if pvNode)
                if (score > alpha and score < beta){
                    score = -negamax<true>(-beta, -alpha, ply + 1, depth - 1, board, sd, ss + 1);
                }
            }
        }

        // Unmake
        board.undoLastMove();

        if (score > bestScore){
            bestScore = score;
            bestMove = move;

            // Beta cutoff

            if (score >= beta){
                updateAllHistory(move, quiets, capts, depth, ply, board, sd, ss);
                break;
            }

            // Raise alpha and update PV list

            if (score > alpha){
                alpha = score;

                // If we are here we will have a non-zero window so we should update our PV
                // This is because we already checked if score >= beta above
                
                sd.pvTable[ply][ply] = move;           
                sd.pvLength[ply] = sd.pvLength[ply + 1];

                for (depth_t nextPly = ply + 1; nextPly < sd.pvLength[ply]; nextPly++)
                     sd.pvTable[ply][nextPly] = sd.pvTable[ply + 1][nextPly];
            }
        }
    }
    // Update TT
    if (!sd.stopped){
        ttFlagAge_t bound = boundExact;

        if (bestScore <= originalAlpha)
            bound = boundUpper;
        else if (bestScore >= beta)
            bound = boundLower;

        globalTT.addToTT(board.getHash(), bestScore, ss->staticEval, bestMove, depth, ply, bound, pvNode);
    }
    return bestScore;
}

score_t aspirationWindowSearch(score_t prevEval, depth_t depth, position &board, searchData &sd){
    // Aspiration Window (~30 elo)
    // We assume our current evaluation will be very close
    // to our previous evaluation so we search with a small window around
    // our previous evaluation and expand the window if it is inconclusive.

    // Init window and stack
    int delta = 14;
    searchStack searchArr[maximumPly + 5];
    searchStack *ss = searchArr;
    
    // If our previous eval was big in magnitude, increase window since scores
    // will probably increase/decrease in larger strides. Won't make
    // that big of a difference because we will probably win/lose either way.

    if (abs(prevEval) >= 425 and abs(prevEval) <= foundMate)
        delta = 20;
    
    // Now we init alpha and beta with our window
    score_t alpha = std::max(prevEval - delta, static_cast<int>(-checkMateScore));
    score_t beta = std::min(prevEval + delta, static_cast<int>(checkMateScore));

    // If we are at low depth, we just do a single search since low depths are unstable
    if (depth <= 7){
        alpha = -checkMateScore;
        beta = checkMateScore;
    }    

    while (true){
        // Get score
        score_t score = negamax<true>(alpha, beta, 0, depth, board, sd, ss);

        // Out of time
        if (sd.stopped)
            break;
        
        // Score is an upperbound
        if (score <= alpha){
            beta = (static_cast<int>(alpha) + static_cast<int>(beta)) / 2;
            alpha = std::max(score - delta, static_cast<int>(-checkMateScore));
        }

        // Score is a lowerbound
        else if (score >= beta){
            // No alpha = (alpha + beta) / 2 because "it doesn't work" according
            // to some person on discord. After testing, I realized that they were correct...
            beta = std::min(score + delta, static_cast<int>(checkMateScore));
        }

        // Score is exact
        else 
            return score;
        
        // Snap to mate score
        if (alpha <= 2750)
            alpha = -checkMateScore;

        if (beta >= 2750)
            beta = checkMateScore;

        // Increase delta exponentially
        delta += delta / 2;
    }

    // Only reach here when out of time
    return 0;
}

void searchDriver(uint64 timeAlloted, position boardToSearch){


    
    move_t bestMove = 0;
    score_t score = 0;

    position board = boardToSearch;
    searchData sd = {};

    sd.T.beginTimer(timeAlloted);

    for (depth_t startingDepth = 1; startingDepth <= maximumPly; startingDepth++){

        // Search
        sd.selDepth = 0;


        score = aspirationWindowSearch(score, startingDepth, board, sd);
        
        if (!sd.stopped){
            bestMove = sd.pvTable[0][0];

            std::cout<<"info depth "<<int(startingDepth);
            std::cout<<" seldepth "<<int(sd.selDepth);
            
            if (abs(score) >= foundMate)
                std::cout<<" score mate "<<(checkMateScore - abs(score) + 1) * (score > 0 ? 1 : -1) / 2;
            else 
                std::cout<<" score cp " <<score;
            
            std::cout<<" nodes "    <<sd.nodes;
            std::cout<<" time "<<sd.T.timeElapsed();
            std::cout<<" nps "      <<int(sd.nodes / (sd.T.timeElapsed() / 1000.0 + 0.00001));
            std::cout<<" hashfull "<<globalTT.hashFullness();
            std::cout<<" pv ";

            for (depth_t i = 0; i < sd.pvLength[0]; i++) 
                std::cout<<moveToString(sd.pvTable[0][i])<<" ";

            std::cout<<std::endl;
        }
    }
    globalTT.incrementAge();
    std::cout<<"bestmove "<<moveToString(bestMove)<<std::endl;
    
}