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


uint64 perft(depth_t depth, depth_t depthLim){
    moveList moves;
    perftBoard.genAllMoves(false, moves);

    // Return number of leaves without directly exploring them
    if (depth + 1 >= depthLim)
        return moves.sz;
    
    uint64 leaves = 0;
    for (int i = 0; i < moves.sz; i++){
        move_t move = moves.moves[i].move;

        uint64 st = getTime();
        perftBoard.makeMove(move);
        totalTime += getTime() - st;
        nodes++;

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
                    uciParams uci;
                    uci.timeIncr[board.getTurn()] = 0;
                    uci.movesToGo = 1;
                    uci.timeLeft[board.getTurn()] = 333;

                    if (board.getTurn() == white)
                        beginSearch(board, uci);

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
    uci.timeLeft[board.getTurn()] = 2000;

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
    if (false){
        doLoop();
        return 0;
    }

    
    threadCount = 2;
    selectFromPGN();
    return 0;
    

    // perftBoard.readFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    // std::cout<<perftBoard.eval()<<std::endl;
    // return 0;
    // perft(0, 5);
    // std::cout<<std::setprecision(15)<<totalTime / (nodes + 0.0)<<std::endl;
    // return 0;

    /*
    CHANGES:
    1) QS SEE Pruning -- remove movesSeen > 1 condition
    2) MCP in main search -- remove !pvNode condition
    3) SEE pruning in main search -- remove !root condition
    4) HIstory pruning -- add !inCheck condition
    */

    // TODO: 
    // TT Prefetch
    // Allocate less time for timeman
    // SIMD
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


    board.readFen("1r6/7k/5P2/2R4B/6KP/6P1/8/8 w - - 3 80");

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
-engine conf="E82_Debug6" `
-engine conf="E67_AWupdate" `
-each tc=6+0.06 timemargin=200 `
-openings file="C:\Program Files\Cute Chess\Chess Openings\openings-8ply-10k.pgn" `
-pgnout "C:\Program Files\Cute Chess\Games\Games1.txt.txt" `
-games 2 `
-rounds 25000 `
-repeat 2 `
-recover `
-concurrency 5 `
-ratinginterval 10
*/

/*
-sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 `
*/

/*
[Event "My Tournament"]
[Site "?"]
[Date "2023.06.07"]
[Round "56"]
[White "Ethereal-avx"]
[Black "E80_Debug4"]
[Result "1-0"]
[ECO "C63"]
[GameDuration "00:00:33"]
[GameEndTime "2023-06-07T17:40:09.436 Pacific Daylight Time"]
[GameStartTime "2023-06-07T17:39:35.947 Pacific Daylight Time"]
[Opening "Ruy Lopez"]
[PlyCount "151"]
[Termination "abandoned"]
[TimeControl "40/6+0.06"]
[Variation "Schliemann defense"]

1. e4 {book} e5 {book} 2. Nf3 {book} Nc6 {book} 3. Bb5 {book} f5 {book}
4. Bxc6 {book} dxc6 {book} 5. Nc3 {+0.42/18 0.62s} fxe4 {-0.03/16 0.33s}
6. Nxe4 {+0.10/17 0.17s} Nf6 {-0.21/15 0.14s} 7. Qe2 {+0.12/19 0.41s}
Bg4 {-0.17/15 0.28s} 8. d3 {+0.10/17 0.23s} Bb4+ {-0.05/16 0.30s}
9. Bd2 {+0.35/17 0.16s} Bxf3 {-0.08/15 0.16s} 10. Qxf3 {+0.44/17 0.27s}
Bxd2+ {-0.06/15 0.13s} 11. Nxd2 {+0.41/17 0.17s} Qd4 {-0.16/16 0.23s}
12. O-O-O {+0.60/18 0.28s} O-O {+0.25/14 0.16s} 13. Rhf1 {+0.50/17 0.17s}
Qd5 {-0.11/15 0.37s} 14. a3 {+0.70/18 0.33s} Rae8 {-0.29/15 0.54s}
15. Qg3 {+0.60/17 0.31s} a5 {-0.10/17 0.32s} 16. f3 {+0.78/19 0.96s}
b5 {-0.08/14 0.25s} 17. Kb1 {+0.62/18 0.21s} Nh5 {-0.30/14 0.38s}
18. Qf2 {+0.67/17 0.20s} Nf4 {-0.53/14 0.21s} 19. Rfe1 {+0.67/18 0.27s}
Qd6 {-0.62/14 0.28s} 20. Re4 {+1.16/16 0.20s} Nd5 {-0.79/15 0.39s}
21. Rde1 {+0.94/17 0.24s} a4 {-0.70/15 0.21s} 22. Rg4 {+0.88/17 0.11s}
Nf6 {-0.45/15 0.46s} 23. Rh4 {+0.98/19 0.30s} Nd7 {-0.75/14 0.24s}
24. Qg3 {+1.23/18 0.49s} c5 {-0.85/13 0.23s} 25. Ne4 {+1.38/18 0.33s}
Qe7 {-0.79/12 0.12s} 26. Qg5 {+0.92/17 0.28s} Qe6 {-0.69/14 0.28s}
27. Rg4 {+1.18/16 0.22s} Rf7 {-0.54/13 0.21s} 28. Qe3 {+1.18/15 0.14s}
Qb6 {-0.50/14 0.20s} 29. c3 {+0.88/14 0.12s} h6 {-0.18/14 0.21s}
30. h4 {+1.00/13 0.048s} Re6 {0.00/13 0.11s} 31. Qd2 {+0.69/15 0.074s}
Rc6 {-0.02/13 0.13s} 32. Rh1 {+0.82/15 0.059s} Qb7 {-0.10/14 0.21s}
33. h5 {+0.84/13 0.057s} Qa6 {0.00/14 0.12s} 34. Rd1 {+0.91/15 0.061s}
Qb6 {0.00/15 0.26s} 35. Qe1 {+0.87/14 0.055s} Nf8 {-0.11/14 0.30s}
36. Ng3 {+0.73/15 0.057s} Nd7 {-0.15/13 0.064s} 37. Ne4 {+0.92/17 0.073s}
Nf8 {0.00/12 0.12s} 38. Qd2 {+0.99/14 0.062s} Nd7 {-0.21/11 0.073s}
39. Qf2 {+0.85/14 0.050s} Re6 {0.00/11 0.061s} 40. Qh4 {+0.85/14 0.060s}
Kh7 {0.00/13 0.060s} 41. Qf2 {+0.77/18 0.71s} Kg8 {-0.20/14 0.29s}
42. Rd2 {+0.73/18 0.47s} Kh7 {-0.26/14 0.22s} 43. Qe1 {+0.92/18 0.40s}
Qa6 {-0.26/15 0.38s} 44. Rc2 {+0.92/19 0.44s} Rf8 {-0.13/15 0.47s}
45. d4 {+1.85/18 0.61s} exd4 {-0.51/12 0.10s} 46. cxd4 {+1.26/18 0.19s}
Kh8 {-0.66/14 0.36s} 47. d5 {+1.53/17 0.40s} Re5 {-0.64/14 0.10s}
48. Qd1 {+0.62/18 0.56s} Qa7 {0.00/14 0.15s} 49. Rg6 {+1.10/18 0.51s}
b4 {+0.16/15 0.14s} 50. Re6 {+1.17/16 0.13s} bxa3 {+0.19/16 0.22s}
51. Nxc5 {+0.01/20 0.28s} Rxe6 {-0.26/16 0.16s} 52. dxe6 {+0.01/21 0.32s}
Nxc5 {-0.31/18 0.13s} 53. e7 {+0.31/21 0.13s} Re8 {-0.65/19 0.24s}
54. Qd8 {+0.14/20 0.11s} Qb8 {-0.47/20 0.18s} 55. Qxb8 {+0.37/22 0.19s}
Rxb8 {-0.63/18 0.11s} 56. Rxc5 {+0.35/22 0.16s} Rxb2+ {-0.71/20 0.37s}
57. Ka1 {+0.17/21 0.10s} Re2 {-0.60/18 0.12s} 58. Rxc7 {+0.15/24 0.21s}
Kg8 {-0.64/18 0.14s} 59. f4 {+0.11/22 0.17s} Kf7 {-0.39/18 0.11s}
60. e8=Q+ {+0.01/20 0.12s} Kxe8 {-0.63/19 0.14s} 61. Rxg7 {-0.01/20 0.12s}
Rf2 {-0.24/18 0.13s} 62. g3 {+0.01/21 0.095s} Kf8 {-0.34/20 0.19s}
63. Rg6 {+0.02/19 0.11s} Rf3 {-0.22/19 0.40s} 64. Kb1 {+0.01/19 0.13s}
a2+ {-0.33/18 0.16s} 65. Kxa2 {+0.17/18 0.30s} Kf7 {-0.16/19 0.50s}
66. Ka1 {+0.28/20 0.16s} a3 {-0.19/16 0.15s} 67. Ka2 {+0.01/19 0.18s}
Ke7 {-0.15/16 0.12s} 68. Rxh6 {+0.08/17 0.20s} Rxg3 {-0.15/14 0.10s}
69. Rb6 {+0.01/20 0.12s} Rf3 {-0.03/15 0.18s} 70. h6 {+0.01/18 0.11s}
Rxf4 {-0.05/15 0.21s} 71. Rb8 {+0.01/18 0.10s} Rf8 {0.00/16 0.14s}
72. Rb7+ {+0.01/20 0.060s} Kf6 {0.00/16 0.10s} 73. h7 {+0.01/21 0.053s}
Kg6 {0.00/20 0.14s} 74. Kxa3 {+0.01/22 0.074s} Rh8 {0.00/24 0.23s}
75. Kb4 {+0.01/20 0.059s} Rxh7 {0.00/33 0.74s}
76. Rxh7 {+0.01/34 0.055s, Black disconnects} 1-0
*/

/*
Score of E73_LazySMP2 vs E67_AWupdate: 297 - 197 - 590  [0.546] 1084
...      E73_LazySMP2 playing White: 192 - 60 - 290  [0.622] 542
...      E73_LazySMP2 playing Black: 105 - 137 - 300  [0.470] 542
...      White vs Black: 329 - 165 - 590  [0.576] 1084
Elo difference: 32.1 +/- 13.9, LOS: 100.0 %, DrawRatio: 54.4 %
SPRT: llr 2.96 (100.4%), lbound -2.94, ubound 2.94 - H1 was accepted

Player: E73_LazySMP2
   "Draw by 3-fold repetition": 429
   "Draw by fifty moves rule": 72
   "Draw by insufficient mating material": 85
   "Draw by stalemate": 4
   "Loss: Black disconnects": 34
   "Loss: Black mates": 32
   "Loss: White disconnects": 28
   "Loss: White mates": 103
   "No result": 4
   "Win: Black disconnects": 1
   "Win: Black mates": 105
   "Win: White mates": 191
Player: E67_AWupdate
   "Draw by 3-fold repetition": 429
   "Draw by fifty moves rule": 72
   "Draw by insufficient mating material": 85
   "Draw by stalemate": 4
   "Loss: Black disconnects": 1
   "Loss: Black mates": 105
   "Loss: White mates": 191
   "No result": 4
   "Win: Black disconnects": 34
   "Win: Black mates": 32
   "Win: White disconnects": 28
   "Win: White mates": 103
Finished match
*/


/*
Score of E81_Debug5 vs E67_AWupdate: 3040 - 3194 - 11355  [0.496] 17589
...      E81_Debug5 playing White: 1970 - 1099 - 5725  [0.550] 8794
...      E81_Debug5 playing Black: 1070 - 2095 - 5630  [0.442] 8795
...      White vs Black: 4065 - 2169 - 11355  [0.554] 17589
Elo difference: -3.0 +/- 3.1, LOS: 2.6 %, DrawRatio: 64.6 %

Player: E81_Debug5
   "Draw by 3-fold repetition": 7930
   "Draw by fifty moves rule": 1345
   "Draw by insufficient mating material": 2050
   "Draw by stalemate": 30
   "Loss: Black mates": 1099
   "Loss: White mates": 2095
   "No result": 8
   "Win: Black disconnects": 2
   "Win: Black mates": 1070
   "Win: White mates": 1968
Player: E67_AWupdate
   "Draw by 3-fold repetition": 7930
   "Draw by fifty moves rule": 1345
   "Draw by insufficient mating material": 2050
   "Draw by stalemate": 30
   "Loss: Black disconnects": 2
   "Loss: Black mates": 1070
   "Loss: White mates": 1968
   "No result": 8
   "Win: Black mates": 1099
   "Win: White mates": 2095
Finished match
*/
