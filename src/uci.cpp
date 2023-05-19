#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include "board.h"
#include "tt.h"
#include "uci.h"
#include "search.h"

static position board;

void position::resetStack(){
    historyArray[0] = *stk;
    stk = historyArray;
}

void position::printBoard(){
    for (square_t st = 56; st >= 0; st -= 8){
        std::cout<<st / 8 + 1<<" ";
        for (square_t sq = st; sq < st + 8; sq++)
            std::cout<<(board[sq] ? pieceToChar(board[sq]) : ' ')<<" ";
        std::cout<<"\n";
    }
    std::cout<<"  a b c d e f g h\n\n";
}

std::string squareToString(square_t sq){
    char fileLetter = (getFile(sq) + 'a');
    return fileLetter + std::to_string(getRank(sq) + 1);
}

std::string moveToString(move_t move){
    square_t st = moveFrom(move);
    square_t en = moveTo(move);
    piece_t promo = movePromo(move);

    std::string S = char(getFile(st) + 'a') 
                  + std::to_string(getRank(st) + 1) 
                  + char(getFile(en) + 'a') 
                  + std::to_string(getRank(en) + 1);
    if (promo) 
        S += pieceToChar((promo << 1) + 1);
        
    return S;
}

move_t stringToMove(std::string move){
    square_t st = (move[0] - 'a') + 8 * (move[1] - '1');
    square_t en = (move[2] - 'a') + 8 * (move[3] - '1');
    piece_t promo = (move.size() == 5) ? (charToPiece(move[4]) >> 1) : 0;
    return encodeMove(st, en, promo);
}

char pieceToChar(piece_t p){
    if (p >> 1 == pawn) return ((p & 1) == white ? 'P' : 'p');
    if (p >> 1 == knight) return ((p & 1) == white ? 'N' : 'n');
    if (p >> 1 == bishop) return ((p & 1) == white ? 'B' : 'b');
    if (p >> 1 == rook) return ((p & 1) == white ? 'R' : 'r');
    if (p >> 1 == queen) return ((p & 1) == white ? 'Q' : 'q');
    if (p >> 1 == king) return ((p & 1) == white ? 'K' : 'k');
    return ' ';
}

piece_t charToPiece(char c){
    color_t col = islower(c);
    c = tolower(c);
    if (c == 'p') return (pawn << 1) + col;
    if (c == 'n') return (knight << 1) + col;
    if (c == 'b') return (bishop << 1) + col;
    if (c == 'r') return (rook << 1) + col;
    if (c == 'q') return (queen << 1) + col;
    if (c == 'k') return (king << 1) + col;
    return 0;
}

void printMask(bitboard_t msk){
    for (square_t i = 56; i >= 0; i -= 8){
        for (square_t j = i; j < i + 8; j++){
            std::cout<<((msk & (1ULL << j)) ? 1 : 0)<<" \n"[j == i + 7];
        }
    }
    std::cout<<"\n";
}

void position::readFen(std::string fen){
    // Step 1) We break apart the fen by seperating into multiple strings.
    std::istringstream iss(fen);
    std::string piecePosStr, playerTurnStr, castleStr, enpassTargetStr, halfMoveClockStr, currentFullMoveStr;
    iss >> piecePosStr >> playerTurnStr >> castleStr >> enpassTargetStr >> halfMoveClockStr >> currentFullMoveStr;

    // Step 2) Reset everything that must be reset
    std::fill(board, board + 64, noPiece);
    memset(pieceBB, 0, sizeof(pieceBB));
    memset(colorBB, 0, sizeof(colorBB));
    allBB = 0;

    stk = historyArray;
    stk->captPieceType = noPiece;
    stk->castleRights = 0;
    stk->epFile = noEP;
    stk->halfMoveClock = 0;
    stk->moveCount = 0;
    stk->zhash = 0;
    
    // Step 3) Fill the board
    square_t sq = 56;
    for (char c : piecePosStr){
        if (c == '/') sq -= 16;
        else if (isdigit(c)) sq += c - '0';
        else{
            piece_t piece = charToPiece(c);
            addPiece(getPieceType(piece), sq, getPieceColor(piece), false);
            sq++;
        }
    }

    // Step 4) Player Turn
    turn = (playerTurnStr == "b");

    // Step 5) Castling rights
    for (char c : castleStr){
        if (c == 'K') stk->castleRights |= castleWhiteK;
        if (c == 'Q') stk->castleRights |= castleWhiteQ;
        if (c == 'k') stk->castleRights |= castleBlackK;
        if (c == 'q') stk->castleRights |= castleBlackQ;
    }

    // Step 4) En passant
    if (enpassTargetStr != "-") 
        stk->epFile = (enpassTargetStr[0] - 'a');

    // Step 5) Half move and full move
    stk->halfMoveClock = stoi(halfMoveClockStr);
    stk->moveCount = (stoi(currentFullMoveStr) - 1) * 2 + turn;

    // Step 7) Fold everything into zhash
    stk->zhash ^= ttRngCastle[stk->castleRights] ^ ttRngEnpass[stk->epFile] ^ (ttRngTurn * turn);
    
    // Step 8) Refresh NNUE
    stk->nnue.refresh(board, kingSq(white), kingSq(black));
}

std::string position::getFen(){
    std::string fen = "";

    // Step 1) Piece positions
    for (square_t st = 56; st >= 0; st -= 8){
        for (square_t sq = st, empt = 0; sq < st + 8; sq++){
            empt += !board[sq];
            if ((board[sq] or sq == st + 7) and empt){
                fen += std::to_string(empt); 
                empt = 0;
            }
            if (board[sq]) 
                fen += pieceToChar(board[sq]);
        }
        if (st > 0) fen += "/";
    }
    
    // Step 2) Turn
    fen += " ";
    fen += (turn == white ? "w" : "b");

    // Step 3) Castling rights
    fen += " ";
    if (stk->castleRights & castleWhiteK) fen += "K";
    if (stk->castleRights & castleWhiteQ) fen += "Q";
    if (stk->castleRights & castleBlackK) fen += "k";
    if (stk->castleRights & castleBlackQ) fen += 'q';
    if (!stk->castleRights) fen += "-";

    // Step 4: En passant
    fen += " ";
    if (stk->epFile != noEP){
        fen += (stk->epFile + 'a');
        fen += std::to_string(turn == white ? 6 : 3);
    }
    else fen += "-";

    // Step 5) Half move and full move
    fen += " " + std::to_string(stk->halfMoveClock);
    fen += " " + std::to_string(stk->moveCount / 2 + 1);
    
    return fen;
}

std::string position::flippedFen(){
    // Step 0) Initalize
    std::istringstream iss(getFen());
    std::string piecePosStr, playerTurnStr, castleStr, enpassTargetStr, halfMoveClockStr, currentFullMoveStr;
    iss >> piecePosStr >> playerTurnStr >> castleStr >> enpassTargetStr >> halfMoveClockStr >> currentFullMoveStr;

    std::string newFen = "";

    // Step 1) Reverse piece positions (flip everything across ranks 4/5 and flip color)
    piece_t newBoard[64];
    for (square_t sq = 0; sq < 64; sq++)
        newBoard[flip(sq)] = !board[sq] ? 0 : board[sq] ^ 1;
    
    // Construct fen using our newBoard
    for (square_t st = 56; st >= 0; st -= 8){
        for (square_t sq = st, empt = 0; sq < st + 8; sq++){
            empt += !newBoard[sq];
            if ((newBoard[sq] or sq == st + 7) and empt){
                newFen += std::to_string(empt); 
                empt = 0;
            }
            if (newBoard[sq]) 
                newFen += pieceToChar(newBoard[sq]);
        }
        if (st > 0) newFen += "/";
    }
    
    // Step 2) Player turn
    playerTurnStr = (playerTurnStr == "w" ? 'b' : 'w');
    newFen += " " + playerTurnStr;

    // Step 3) Castling
    if (castleStr != "-")
        for (char &c : castleStr)
            c = isupper(c) ? tolower(c) : toupper(c);

    newFen += " " + castleStr;
    
    // Step 4) En passant
    if (enpassTargetStr != "-")
        enpassTargetStr[1] = enpassTargetStr[1] == '3' ? '6' : '3';

    newFen += " " + enpassTargetStr;

    // Step 5) Half move and full move
    newFen += " " + halfMoveClockStr;
    newFen += " " + currentFullMoveStr;

    return newFen;
}


void printInfo(){
    std::cout<<"id name Superultra"<<std::endl;
    std::cout<<"id author Alex Liang"<<std::endl;
    std::cout<<"option name Hash type spin default 16 min 1 max 2048"<<std::endl;
    std::cout<<"uciok"<<std::endl;
}

void proccessGo(std::istringstream &iss){
    std::string token;
    uciParams uci;

    while (iss >> token){
        // Only searches the moves give; must be the last command
        if (token == "searchmoves"){
            while (iss >> token){

            }
        }
        // Keep searching until stop command
        else if (token == "infinite"){
            uci.infiniteSearch = 1;
        }
        // Time left for white
        else if (token == "wtime"){
            iss >> uci.timeLeft[white];
        }
        // TIme left for black
        else if (token == "btime"){
            iss >> uci.timeLeft[black];
        }
        // Increment per move for white
        else if (token == "winc"){
            iss >> uci.timeIncr[white];
        }
        // Increment per move for black
        else if (token == "binc"){
            iss >> uci.timeIncr[black];
        }
        // Moves till next time control
        else if (token == "movestogo"){
            iss >> uci.movesToGo;
        }
        // Limit depth
        else if (token == "depth"){

        }
        // Limit nodes
        else if (token == "nodes"){
            iss >> uci.nodeLim;
        }
        // Ponder
        else if (token == "ponder"){
            
        }
    }
    
    // Calculate time and search
    uint64 searchTime = uci.infiniteSearch ? uint64(1e18) : findOptimalTime(board.getTurn(), uci);
    searchDriver(searchTime, board);
}

void setOption(std::istringstream &iss){
    // setoption name option [value ...]
    std::string token, optionName;
    iss >> token; // Read the "name" token

    // Read the full name of the option
    while ((iss >> token) and token != "value"){
        optionName += (optionName.empty() ? "" : " ") + token;
    }

    // New hash size
    if (optionName == "Hash"){
        iss >> token;
        globalTT.setSize(stoi(token));
    }
}

void setPos(std::istringstream &iss){
    // Command: position [fen | startpos] moves ...
    std::string token;
    iss >> token;

    // First, read the fen and set up the board. Also read the "moves" token
    if (token == "startpos"){
        board.readFen(startPosFen);
        iss >> token; // Read "moves" token
    }
    else{
        std::string fen = "";
        while (iss >> token and token != "moves")
            fen += token + " ";
        board.readFen(fen);
    }
    // Now play the remaining moves
    while (iss >> token){
        // Play the moves while resetting our stack if it is a move that resets fmr
        move_t move = stringToMove(token);
        bool fmr = board.moveCaptType(move) != noPiece or board.movePieceType(move) == pawn;

        board.makeMove(move);

        if (fmr){
            board.resetStack();
        }
    }
}

void doLoop(){
    while (1){
        std::string cmd, token; 
        getline(std::cin, cmd);

        std::istringstream iss(cmd);
        iss >> token;

        // Print basic info, commands, and say that you are uci compliant
        if (token == "uci"){
            printInfo();
        }
        // Start a new game (clear TT to make it deterministic)
        else if (token == "ucinewgame"){
            board.readFen(startPosFen);
            globalTT.clearTT(); 
        }
        // Say that you are ready
        else if (token == "isready"){
            std::cout<<"readyok"<<std::endl;
        }
        // Sets the position through base fen and sequence of moves
        else if (token == "position"){
            setPos(iss);
        }
        // Sets a single option
        else if (token == "setoption"){
            setOption(iss);
        }
        // Search the position
        else if (token == "go"){
            proccessGo(iss);
        }
        // Stop the search
        else if (token == "stop"){
            
        }
        // End the program
        else if (token == "quit"){
            break;
        }
    }
}

