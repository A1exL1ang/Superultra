#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <iomanip>  
#include <chrono>
#include "types.h"
#include "helpers.h"
#include "board.h"
#include "uci.h"
#include "timecontrol.h"
#include "attacks.h"
#include "movepick.h"
#include "search.h"
#include "test.h"


position perftBoard;
uint64 totalTime, nodes;

timePoint_t getTimeNS(){
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64 sum = 0;

uint64 perft(depth_t depth, depth_t depthLim){
    moveList moves;
    perftBoard.genAllMoves(false, moves);

    sum += perftBoard.eval();
    

    // Return number of leaves without directly exploring them
    if (depth + 1 >= depthLim)
        return moves.sz;
    
    uint64 leaves = 0;
    for (int i = 0; i < moves.sz; i++){
        move_t move = moves.moves[i].move;

        
        perftBoard.makeMove(move);
        

        uint64 value = perft(depth + 1, depthLim);
        leaves += value;

        perftBoard.undoLastMove();
    }
    return leaves;
}

void selectFromPGN(){
    
    // List of PGN --> list of FEN
    // Select random subset

    std::ifstream inputFile("zinput.txt");
    position board;

    while (!inputFile.eof()){

        // Result is relative to current player (which is initially white)
        std::string S;
        double result;

        // First we read the tags
        while (std::getline(inputFile, S)){
            // We finished reading the tags
            if (S.empty()) 
                break;
            if (S.find("Result") != std::string::npos){
                if (S.find("1-0") != std::string::npos)
                    result = 1;
                if (S.find("0-1") != std::string::npos)
                    result = 0;
                if (S.find("1/2-1/2") != std::string::npos)
                    result = 0.5;
            }
        }

        // Now we read the content and extract the fen
        board.readFen(startPosFen);

        // Example PGN (note that it can and will be split across many lines):
        // 1. d4 {book} Nf6 {book} 2. Nf3 {book} e6 {book} 3. c4 {+0.16/7 0.065s}
        // Nc6 {-0.23/7 0.065s} 4. Nc3 {+0.25/8 0.064s} d5 {-0.16/8 0.064s}
        // 5. e3 {+0.16/7 0.066s} Ne4 {-0.22/7 0.066s} 6. a3 {+0.34/8 0.066s}

        bool inComment = 0;

        while (!inputFile.eof() and std::getline(inputFile, S)){
            // We finished the game
            if (S.empty())
                break;            

            for (int i = 0; i < S.size(); i++){
                // See whether we are in comment or not
                if (S[i] == '{')
                    inComment = 1;
                else if (S[i] == '}')
                    inComment = 0;

                // We found a move
                if (!inComment and isalpha(S[i])){

                    /*
                    uciParams uci;
                    uci.timeIncr[board.getTurn()] = 0;
                    uci.movesToGo = 1;
                    uci.timeLeft[board.getTurn()] = 280;

                    if (board.getTurn() == black)
                        beginSearch(board, uci);
                    */

                    // Generate all moves 
                    moveList allMoves;
                    board.genAllMoves(false, allMoves);

                    // Extract the move (we assume move isn't broken by line)
                    std::string curMove;
                    while (i < S.size() and S[i] != ' ') 
                        curMove += S[i++];
                    
                    // Remove the + or # from the end
                    if (curMove.back() == '+' or curMove.back() == '#')
                        curMove.pop_back();

                    // Castle
                    if (curMove == "O-O"){
                        board.makeMove(stringToMove(board.getTurn() == white ? "e1g1" : "e8g8"));
                    }
                    else if (curMove == "O-O-O"){
                        board.makeMove(stringToMove(board.getTurn() == white ? "e1c1" : "e8c8"));
                    }

                    // Other moves (ex: d4, dxe5, c8=Q, dxe5=Q, Nxf1e3, N1g3, Nde7)
                    else{
                        // Determine piece by looking at start
                        piece_t piece = islower(curMove[0]) ? pawn : charToPiece(curMove[0]) >> 1;
                        piece_t promo = 0;

                        // Promotion (ex: c8=Q, dxe5=Q)
                        // Delete the promotion portion after proccessing
                        if (isupper(curMove.back())){
                            promo = charToPiece(curMove.back()) >> 1;
                            curMove = curMove.substr(0, curMove.size() - 2);
                        }
                        
                        // End is always the last 2 characters after we delete stuff from the end
                        std::string en = curMove.substr(curMove.size() - 2);
                        file_t stFile = 8; 
                        rank_t stRank = 8;

                        // Pawn capture -- you are given starting file (ex: axb5)
                        if (islower(curMove[0]) and curMove[1] == 'x'){
                            stFile = curMove[0] - 'a';
                        }
                        
                        // Now we look at whether other file/ranks are specified (ex: Nxf1e3, N1g3, Nde7)
                        for (int pos = 1; pos < curMove.size() - 2; pos++){
                            if (curMove[pos] == 'x')
                                continue;
                            if (isalpha(curMove[pos]))
                                stFile = curMove[pos] - 'a';
                            else 
                                stRank = curMove[pos] - '0' - 1;
                        }

                        // Now that we have this information, we match it with one of our moves
                        for (int mv = 0; mv < allMoves.sz; mv++){
                            move_t testMove = allMoves.moves[mv].move; 
                            square_t testSt = moveFrom(testMove);
                            square_t testEn = moveTo(testMove);
                            piece_t testPromo = movePromo(testMove);
                            piece_t testPiece = board.movePieceType(testMove);

                            if (testPiece == piece
                                and squareToString(testEn) == en 
                                and !(stFile < 8 and getFile(testSt) != stFile)
                                and !(stRank < 8 and getRank(testSt) != stRank)
                                and !(promo > 0 and testPromo != promo))
                            {
                                bool fmr = board.moveCaptType(testMove) != noPiece or board.movePieceType(testMove) == pawn;

                                board.makeMove(testMove);

                                if (fmr)
                                    board.resetStack();
                            }
                        }
                    }
                    // Flip the result so that it's relative to side to move
                    result = 1.0 - result;
                }
            }
        }
    }
    std::cout<<board.getFen()<<std::endl;

    uciParams uci;
    uci.timeIncr[board.getTurn()] = 0;
    uci.movesToGo = 1;
    uci.timeLeft[board.getTurn()] = 200000;

    beginSearch(board, uci);
    std::cout<<"DONE"<<std::endl;
}

int main(){
    initLineBB();
    initMagicCache();
    initNNUEWeights();
    initLMR();
    initTT();

    // Default settings
    globalTT.setSize(16);
    threadCount = 1;

    // Recent loss: 0.004985
    if (true){
        doLoop();
        return 0;
    }

    // REMOVE RETURN FROM PREFETCH

    // threadCount = 4;
    // selectFromPGN();
    // return 0;

    

    
    /*
    perftBoard.readFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout<<perftBoard.eval()<<std::endl;
    perft(0, 6);
    std::cout<<std::setprecision(15)<<totalTime / (nodes + 0.0)<<std::endl;
    std::cout<<sum<<std::endl;
    return 0;
    */

    /*
    CHANGES:
    1) QS SEE Pruning -- remove movesSeen > 1 condition
    2) MCP in main search -- remove !pvNode condition
    3) SEE pruning in main search -- remove !root condition
    4) HIstory pruning -- add !inCheck condition
    */

    // TODO: 
    // SIMD
    // Fix bad draw detection
    // Bad capture 1, bad capture 2
    // EGTB
    // Thinking on opponent time
    // Other UCI stuffs
    // Make neat and then release

    // testPerft();

      
    position board;

    threadCount = 1;
    
    /*
    {
        board.readFen(startPosFen);

        while (true){
            std::cout<<board.getFen()<<std::endl;
            uciParams uci;
            uci.timeIncr[board.getTurn()] = 0;
            uci.movesToGo = 1;
            uci.timeLeft[board.getTurn()] = 1000;

            moveList moves;
            board.genAllMoves(false, moves);

            if (moves.sz == 0)
                break;

            beginSearch(board, uci);

            

            move_t move = moves.moves[0].move;
            bool fmr = board.moveCaptType(move) != noPiece or board.movePieceType(move) == pawn;

            board.makeMove(move);

            if (fmr){
                board.resetStack();
            }
        }
        std::cout<<"DONE"<<std::endl;
        return 0;
    }
    */
    // info depth 18 seldepth 31 score cp 39 nodes 1781651 time 700 nps 2545179 hashfull 369 pv e2e4 e7e5 g1f3 b8c6 d2d4 e5d4 f3d4 f8c5 d4c6 b7c6 f1d3 d7d6 e1g1 g8e7 b1d2 e8g8 f1e1 e7g6 d2f3 f8e8 
    board.readFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    uciParams uci;
    uci.timeIncr[board.getTurn()] = 0;
    uci.movesToGo = 1;
    uci.timeLeft[board.getTurn()] = 20000;

    beginSearch(board, uci);
    std::cout<<"DONE"<<std::endl;
    return 0;
    
    board.makeMove(stringToMove("e2e4"));

    uci.timeIncr[board.getTurn()] = 0;
    uci.movesToGo = 1;
    uci.timeLeft[board.getTurn()] = 10000;

    beginSearch(board, uci);
}
/*
.\cutechess-cli `
-engine conf="E90_RandomizeDraw" `
-engine conf="E88_SIMD2" `
-each tc=6+0.06 timemargin=200 `
-openings file="C:\Program Files\Cute Chess\Chess Openings\openings-8ply-10k.pgn" `
-pgnout "C:\Program Files\Cute Chess\Games\Games1.txt.txt" `
-games 2 `
-rounds 25000 `
-repeat 2 `
-recover `
-concurrency 8 `
-sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 `
-ratinginterval 10
*/

// -sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 `

/*
    for (int i = 0; i < hiddenHalf; i += 16){
        // Load 16 elements
        __m256i vectorAccum = _mm256_load_si256(reinterpret_cast<__m256i*>(&accum[botIdx + i]));
        __m256i vectorW2 = _mm256_load_si256(reinterpret_cast<__m256i*>(&W2[hiddenHalf + i]));

        // Clamp
        vectorAccum = _mm256_max_epi16(vectorAccum, vectorCreluMin);
        vectorAccum = _mm256_min_epi16(vectorAccum, vectorCreluMax);

        vectorEval = _mm256_add_epi32(vectorEval, _mm256_madd_epi16(vectorAccum, vectorW2));
    }
*/