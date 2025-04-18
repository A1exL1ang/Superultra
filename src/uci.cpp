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
static Position board;

char pieceToChar(Piece p){
    if (p >> 1 == PAWN){
        return ((p & 1) == WHITE ? 'P' : 'p');
    }
    if (p >> 1 == KNIGHT){
        return ((p & 1) == WHITE ? 'N' : 'n');
    }
    if (p >> 1 == BISHOP){
        return ((p & 1) == WHITE ? 'B' : 'b');
    }
    if (p >> 1 == ROOK){
        return ((p & 1) == WHITE ? 'R' : 'r');
    }
    if (p >> 1 == QUEEN){
        return ((p & 1) == WHITE ? 'Q' : 'q');
    }
    if (p >> 1 == KING){
        return ((p & 1) == WHITE ? 'K' : 'k');
    }
    return ' ';
}

Piece charToPiece(char c){
    Color col = islower(c);
    c = tolower(c);

    if (c == 'p'){
        return (PAWN << 1) + col;
    }
    if (c == 'n'){
        return (KNIGHT << 1) + col;
    }
    if (c == 'b'){
        return (BISHOP << 1) + col;
    }
    if (c == 'r'){
        return (ROOK << 1) + col;
    }
    if (c == 'q'){
        return (QUEEN << 1) + col;
    }
    if (c == 'k'){
        return (KING << 1) + col;
    }
    return NO_PIECE;
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

void Position::resetStack(){
    pos[0] = pos[stk];
    stk = 0;
}

void Position::readFen(std::string fen){
    // Step 1) We break apart the fen by seperating into multiple strings.
    std::istringstream iss(fen);
    std::string piecePosStr, playerTurnStr, castleStr, enpassTargetStr, halfMoveClockStr, currentFullMoveStr;
    iss >> piecePosStr >> playerTurnStr >> castleStr >> enpassTargetStr >> halfMoveClockStr >> currentFullMoveStr;

    // Step 2) Reset everything that must be reset
    std::fill(board, board + 64, NO_PIECE);
    memset(pieceBB, 0, sizeof(pieceBB));
    memset(colorBB, 0, sizeof(colorBB));
    allBB = 0;

    pos.resize(MAX_PLY + 105);
    stk = 0;
    
    pos[stk].move = NULL_OR_NO_MOVE;
    pos[stk].moveCaptType = NO_PIECE;
    pos[stk].castleRights = 0;
    pos[stk].epFile = NO_EP;
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
            pos[stk].castleRights |= CASTLE_WHITE_K;
        }
        if (c == 'Q'){
            pos[stk].castleRights |= CASTLE_WHITE_Q;
        }
        if (c == 'k'){
            pos[stk].castleRights |= CASTLE_BLACK_K;
        }
        if (c == 'q'){
            pos[stk].castleRights |= CASTLE_BLACK_Q;
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
    pos[stk].nnue.refresh(board, kingSq(WHITE), kingSq(BLACK));
}

std::string Position::getFen(){
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
    fen += (turn == WHITE ? "w" : "b");

    // Step 3) Castling rights
    fen += " ";

    if (pos[stk].castleRights & CASTLE_WHITE_K){
        fen += "K";
    }
    if (pos[stk].castleRights & CASTLE_WHITE_Q){
        fen += "Q";
    }
    if (pos[stk].castleRights & CASTLE_BLACK_K){
        fen += "k";
    }
    if (pos[stk].castleRights & CASTLE_BLACK_Q){
        fen += 'q';
    }
    if (!pos[stk].castleRights){
        fen += "-";
    }

    // Step 4: En passant
    fen += " ";
    if (pos[stk].epFile != NO_EP){
        fen += (pos[stk].epFile + 'a');
        fen += std::to_string(turn == WHITE ? 6 : 3);
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
    std::cout << "id name Superultra 2.1" << std::endl;
    std::cout << "id author Alexander Liang" << std::endl;
    std::cout << "option name Hash type spin default 16 min 1 max 65536" << std::endl;
    std::cout << "option name Threads type spin default 1 min 1 max 2048" << std::endl;
    std::cout << "option name Ponder type check default false" << std::endl;
    std::cout << "uciok" << std::endl;
}

static uciSearchLims proccessGo(std::istringstream &iss){
    std::string token;
    uciSearchLims lims = {};

    // End search just in case
    endSearch();
    pondering = false;

    while (iss >> token){
        // Keep searching until stop command
        if (token == "infinite"){
            lims.infinite = true;
        }
        // Time left for white
        else if (token == "wtime"){
            iss >> lims.timeLeft[WHITE];
        }
        // Time left for black
        else if (token == "btime"){
            iss >> lims.timeLeft[BLACK];
        }
        // Increment per move for white
        else if (token == "winc"){
            iss >> lims.timeIncr[WHITE];
        }
        // Increment per move for black
        else if (token == "binc"){
            iss >> lims.timeIncr[BLACK];
        }
        // Moves till next time control
        else if (token == "movestogo"){
            iss >> lims.movesToGo;
        }
        // Search for specific amount of time
        else if (token == "movetime"){
            iss >> lims.moveTime;
        }
        // Limit depth (be careful with reading it since we don't want to read it in as a char)
        else if (token == "depth"){
            int tempVal;
            iss >> tempVal;
            lims.depthLim = static_cast<Depth>(tempVal);
        }
        // Limit nodes
        else if (token == "nodes"){
            iss >> lims.nodeLim;
        }
        // Ponder (not supported)
        else if (token == "ponder"){
            pondering = true;
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
        bool fmr = board.moveCaptType(move) != NO_PIECE or board.movePieceType(move) == PAWN;

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
            clearAllSearchDataHistory();
        }
        // Say that you are ready
        else if (token == "isready"){
            std::cout << "readyok" << std::endl;
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
        // The guessed move has been played so switch from ponder search to normal 
        // search (don't reset tm). Also note that the blank gui screen is a result of
        // ponderhit resetting the screen so if you pondered for long enough then the
        // moment you finish / end the search you will print out the best move and the
        // screen will reset to pondering the next move

        else if (token == "ponderhit"){
            pondering = false;

            if (searcherThread.joinable()){
                searcherThread.join();
            }
        }
        // Stop the search
        else if (token == "stop"){
            endSearch();
            pondering = false;

            if (searcherThread.joinable()){
                searcherThread.join();
            }
        }
        // End the program
        else if (token == "quit"){
            endSearch();
            pondering = false;

            if (searcherThread.joinable()){
                searcherThread.join();
            }
            break;
        }
    }
}