#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <thread>
#include "board.h"
#include "tt.h"
#include "uci.h"
#include "search.h"

int threadCount;
static position board;

char pieceToChar(Piece p){
    if (p >> 1 == pawn){
        return ((p & 1) == white ? 'P' : 'p');
    }
    if (p >> 1 == knight){
        return ((p & 1) == white ? 'N' : 'n');
    }
    if (p >> 1 == bishop){
        return ((p & 1) == white ? 'B' : 'b');
    }
    if (p >> 1 == rook){
        return ((p & 1) == white ? 'R' : 'r');
    }
    if (p >> 1 == queen){
        return ((p & 1) == white ? 'Q' : 'q');
    }
    if (p >> 1 == king){
        return ((p & 1) == white ? 'K' : 'k');
    }
    return ' ';
}

Piece charToPiece(char c){
    Color col = islower(c);
    c = tolower(c);

    if (c == 'p'){
        return (pawn << 1) + col;
    }
    if (c == 'n'){
        return (knight << 1) + col;
    }
    if (c == 'b'){
        return (bishop << 1) + col;
    }
    if (c == 'r'){
        return (rook << 1) + col;
    }
    if (c == 'q'){
        return (queen << 1) + col;
    }
    if (c == 'k'){
        return (king << 1) + col;
    }
    return noPiece;
}

std::string squareToString(Square sq){
    char fileLetter = (getFile(sq) + 'a');
    return fileLetter + std::to_string(getRank(sq) + 1);
}

std::string moveToString(Move move){
    Square st = moveFrom(move);
    Square en = moveTo(move);
    Piece promo = movePromo(move);

    std::string S = char(getFile(st) + 'a') 
                  + std::to_string(getRank(st) + 1) 
                  + char(getFile(en) + 'a') 
                  + std::to_string(getRank(en) + 1);
    if (promo){
        S += pieceToChar((promo << 1) + 1);
    }   
    return S;
}

Move stringToMove(std::string move){
    Square st = (move[0] - 'a') + 8 * (move[1] - '1');
    Square en = (move[2] - 'a') + 8 * (move[3] - '1');
    Piece promo = (move.size() == 5) ? (charToPiece(move[4]) >> 1) : 0;
    return encodeMove(st, en, promo);
}

void position::resetStack(){
    pos[0] = pos[stk];
    stk = 0;
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

    stk = 0;
    pos[stk].move = nullOrNoMove;
    pos[stk].moveCaptType = noPiece;
    pos[stk].castleRights = 0;
    pos[stk].epFile = noEP;
    pos[stk].halfMoveClock = 0;
    pos[stk].moveCount = 0;
    pos[stk].zhash = 0;
    
    // Step 3) Fill the board
    Square sq = 56;
    for (char c : piecePosStr){
        if (c == '/'){
            sq -= 16;
        }
        else if (isdigit(c)){
            sq += c - '0';
        }
        else{
            Piece piece = charToPiece(c);
            addPiece(getPieceType(piece), sq, getPieceColor(piece), false);
            sq++;
        }
    }

    // Step 4) Player Turn
    turn = (playerTurnStr == "b");

    // Step 5) Castling rights
    for (char c : castleStr){
        if (c == 'K'){
            pos[stk].castleRights |= castleWhiteK;
        }
        if (c == 'Q'){
            pos[stk].castleRights |= castleWhiteQ;
        }
        if (c == 'k'){
            pos[stk].castleRights |= castleBlackK;
        }
        if (c == 'q'){
            pos[stk].castleRights |= castleBlackQ;
        }
    }

    // Step 4) En passant
    if (enpassTargetStr != "-"){
        pos[stk].epFile = (enpassTargetStr[0] - 'a');
    }

    // Step 5) Half move and full move
    pos[stk].halfMoveClock = stoi(halfMoveClockStr);
    pos[stk].moveCount = (stoi(currentFullMoveStr) - 1) * 2 + turn;

    // Step 7) Fold everything into zhash
    pos[stk].zhash ^= ttRngCastle[pos[stk].castleRights] ^ ttRngEnpass[pos[stk].epFile] ^ (ttRngTurn * turn);
    
    // Step 8) Refresh NNUE
    pos[stk].nnue.refresh(board, kingSq(white), kingSq(black));
}

std::string position::getFen(){
    std::string fen = "";

    // Step 1) Piece positions
    for (Square st = 56; st >= 0; st -= 8){
        for (Square sq = st, empt = 0; sq < st + 8; sq++){
            empt += !board[sq];
            if ((board[sq] or sq == st + 7) and empt){
                fen += std::to_string(empt); 
                empt = 0;
            }
            if (board[sq]){
                fen += pieceToChar(board[sq]);
            }
        }
        if (st > 0){
            fen += "/";
        }
    }
    
    // Step 2) Turn
    fen += " ";
    fen += (turn == white ? "w" : "b");

    // Step 3) Castling rights
    fen += " ";

    if (pos[stk].castleRights & castleWhiteK){
        fen += "K";
    }
    if (pos[stk].castleRights & castleWhiteQ){
        fen += "Q";
    }
    if (pos[stk].castleRights & castleBlackK){
        fen += "k";
    }
    if (pos[stk].castleRights & castleBlackQ){
        fen += 'q';
    }
    if (!pos[stk].castleRights){
        fen += "-";
    }

    // Step 4: En passant
    fen += " ";
    if (pos[stk].epFile != noEP){
        fen += (pos[stk].epFile + 'a');
        fen += std::to_string(turn == white ? 6 : 3);
    }
    else{
        fen += "-";
    }

    // Step 5) Half move and full move
    fen += " " + std::to_string(pos[stk].halfMoveClock);
    fen += " " + std::to_string(pos[stk].moveCount / 2 + 1);
    
    return fen;
}

static void printInfo(){
    std::cout<<"id name Superultra"<<std::endl;
    std::cout<<"id author Alexander Liang"<<std::endl;
    std::cout<<"option name Hash type spin default 16 min 1 max 2048"<<std::endl;
    std::cout<<"option name Threads type spin default 1 min 1 max 256"<<std::endl;
    std::cout<<"uciok"<<std::endl;
}

static uciSearchLims proccessGo(std::istringstream &iss){
    std::string token;
    uciSearchLims lims = {};

    while (iss >> token){
        // Only searches the moves give; must be the last command
        if (token == "searchmoves"){
            while (iss >> token){

            }
        }
        // Keep searching until stop command
        else if (token == "infinite"){

        }
        // Time left for white
        else if (token == "wtime"){
            iss >> lims.timeLeft[white];
        }
        // Time left for black
        else if (token == "btime"){
            iss >> lims.timeLeft[black];
        }
        // Increment per move for white
        else if (token == "winc"){
            iss >> lims.timeIncr[white];
        }
        // Increment per move for black
        else if (token == "binc"){
            iss >> lims.timeIncr[black];
        }
        // Moves till next time control
        else if (token == "movestogo"){
            iss >> lims.movesToGo;
        }
        // Limit depth
        else if (token == "depth"){
            
        }
        // Limit nodes
        else if (token == "nodes"){
            
        }
        // Ponder
        else if (token == "ponder"){
            
        }
    }
    return lims;
}

static void setOption(std::istringstream &iss){
    // setoption name option [value ...]
    std::string token, optionName;

    // Read the "name" token
    iss >> token; 

    // Read the full name of the option
    while ((iss >> token) and token != "value"){
        optionName += (optionName.empty() ? "" : " ") + token;
    }
    // TT table size
    if (optionName == "Hash"){
        iss >> token;
        globalTT.setSize(stoi(token));
    }
    // Thread couunt
    if (optionName == "Threads"){
        iss >> token;
        setThreadCount(stoi(token));
    }
}

static void setPos(std::istringstream &iss){
    // Command: position [fen | startpos] moves ...
    std::string token;
    iss >> token;

    // First, read the fen and set up the board. Also read the "moves" token
    if (token == "startpos"){
        board.readFen(startPosFen);

        // Read "moves" token
        iss >> token; 
    }
    else{
        std::string fen = "";

        while (iss >> token and token != "moves"){
            fen += token + " ";
        }   
        board.readFen(fen);
    }
    // Now play the remaining moves
    while (iss >> token){
        Move move = stringToMove(token);
        bool fmr = board.moveCaptType(move) != noPiece or board.movePieceType(move) == pawn;

        board.makeMove(move);

        // Reset the stack if we have a fifty move rule reset
        if (fmr){
            board.resetStack();
        }
    }
}

void doLoop(){
    std::thread searcherThread;

    while (1){
        std::string cmd, token; 
        getline(std::cin, cmd);

        std::istringstream iss(cmd);
        iss >> token;

        // Print basic info, commands, and say that you are uci compliant
        if (token == "uci"){
            printInfo();
        }
        // Start a new game (we should also clear TT and history)
        else if (token == "ucinewgame"){
            board.readFen(startPosFen);
            globalTT.clearTT(); 
            clearHistory();
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
            if (searcherThread.joinable()){
                searcherThread.join();
            }
            searcherThread = std::thread(beginSearch, board, proccessGo(iss));
        }
        // Stop the search
        else if (token == "stop"){
            endSearch();

            if (searcherThread.joinable()){
                searcherThread.join();
            }
        }
        // End the program
        else if (token == "quit"){
            endSearch();
            
            if (searcherThread.joinable()){
                searcherThread.join();
            }
            break;
        }
    }
}