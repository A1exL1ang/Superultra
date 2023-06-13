#include "assert.h"
#include "board.h"
#include "helpers.h"
#include "types.h"
#include "tt.h"

void position::addPiece(Piece pieceType, Square sq, Color col, bool updateAccum){
    // Assumes position is empty
    assert(!board[sq]);

    pieceBB[pieceType][col] ^= (1ULL << sq);
    colorBB[col] ^= (1ULL << sq);
    allBB ^= (1ULL << sq);
    board[sq] = encodePiece(pieceType, col);
    pos[stk].zhash ^= ttRngPiece[pieceType][col][sq];

    if (updateAccum){
        pos[stk].nnue.addFeature(pieceType, sq, col, kingSq(white), kingSq(black));
    }
}

void position::removePiece(Square sq, bool updateAccum){
    // Assumes there's is a piece at sq
    assert(board[sq]);

    Piece pieceType = getPieceType(board[sq]);
    Color col = getPieceColor(board[sq]);

    pieceBB[pieceType][col] ^= (1ULL << sq);
    colorBB[col] ^= (1ULL << sq);
    allBB ^= (1ULL << sq);
    board[sq] = noPiece;
    pos[stk].zhash ^= ttRngPiece[pieceType][col][sq];

    if (updateAccum){
        pos[stk].nnue.removeFeature(pieceType, sq, col, kingSq(white), kingSq(black));
    }
}

void position::movePiece(Piece piece, Square st, Square en, Color col, bool updateAccum){
    // Assumes there's a piece at st and no piece at en
    assert(board[st] and !board[en]);

    pieceBB[piece][col] ^= (1ULL << st) ^ (1ULL << en);
    colorBB[col] ^= (1ULL << st) ^ (1ULL << en);
    allBB ^= (1ULL << st) ^ (1ULL << en);
    board[st] = noPiece;
    board[en] = encodePiece(piece, col);
    pos[stk].zhash ^= ttRngPiece[piece][col][st] ^ ttRngPiece[piece][col][en];
    
    if (updateAccum){ 
        pos[stk].nnue.updateMove(piece, st, en, col, kingSq(white), kingSq(black));
    }
}

void position::makeMove(Move move){
    // Step 1) Decode and get necessary info
    Square st = moveFrom(move);
    Square en = moveTo(move);
    Piece promo = movePromo(move);
    Piece pieceType = getPieceType(board[st]);
    Piece captType = getPieceType(board[en]);
    bool refresh = (pieceType == king and kingBucketId[turn == white ? st : flip(st)] != kingBucketId[turn == white ? en : flip(en)]);

    // Step 2) Update stack (move + capt is logged in stack i and we copy board info to stack i + 1)
    stk++;

    pos[stk - 1].moveCaptType = captType;
    pos[stk - 1].move = move;

    pos[stk].castleRights = pos[stk - 1].castleRights;
    pos[stk].epFile = pos[stk - 1].epFile;
    pos[stk].zhash = pos[stk - 1].zhash;

    pos[stk].halfMoveClock = pos[stk - 1].halfMoveClock;
    pos[stk].moveCount = pos[stk - 1].moveCount;
    
    pos[stk].nnue = pos[stk - 1].nnue;

    // Step 3) En passant
    if (isEP(move)){ 
        Square enemyPawnSq = (turn == white ? en - 8 : en + 8);
        movePiece(pawn, st, en, turn, !refresh);
        removePiece(enemyPawnSq, !refresh);
    }

    // Step 4) Promotion
    else if (promo){
        if (captType != noPiece){
            removePiece(en, !refresh);
        }
        removePiece(st, !refresh);
        addPiece(promo, en, turn, !refresh);
    }

    // Step 5) Castle
    else if (pieceType == king and abs(getFile(st) - getFile(en)) == 2){
        Square stRook;
        Square enRook;
        
        // Kingside
        if (st < en){ 
            stRook = (turn == white ? h1 : h8); 
            enRook = (turn == white ? f1 : f8);
        }
        // Queenside
        else{ 
            stRook = (turn == white ? a1 : a8);
            enRook = (turn == white ? d1 : d8);
        }
        movePiece(king, st, en, turn, !refresh);
        movePiece(rook, stRook, enRook, turn, !refresh);
    }

    // Step 6) All other moves
    else{
        if (captType != noPiece){
            removePiece(en, !refresh);
        }
        movePiece(pieceType, st, en, turn, !refresh);
    }

    // Step 7) Clock
    pos[stk].halfMoveClock = (captType != noPiece or pieceType == pawn) ? 0 : pos[stk].halfMoveClock + 1;
    pos[stk].moveCount++;

    // Step 8a) Clear zhash of ep and castle rights so we can fold everything into it afterwards
    pos[stk].zhash ^= ttRngCastle[pos[stk].castleRights] ^ ttRngEnpass[pos[stk].epFile];
    
    // Step 8b) En passant rights
    pos[stk].epFile = (pieceType == pawn and abs(getRank(st) - getRank(en)) == 2) ? getFile(st) : noEP;
    
    // Step 8c) Castle rights if we move king
    if (pieceType == king){
        pos[stk].castleRights &= (turn == white ? (castleBlackK | castleBlackQ) : (castleWhiteK | castleWhiteQ));
    }
    
    // Step 8d) Castle rights if we altered any rook positions through moving or capturing 
    pos[stk].castleRights &= ((st != h1 and en != h1) * castleWhiteK)
                           | ((st != a1 and en != a1) * castleWhiteQ)
                           | ((st != h8 and en != h8) * castleBlackK)
                           | ((st != a8 and en != a8) * castleBlackQ);

    // Step 8e) Flip color
    turn ^= 1;

    // Step 8f) Update zhash
    pos[stk].zhash ^= ttRngCastle[pos[stk].castleRights] ^ ttRngEnpass[pos[stk].epFile] ^ ttRngTurn;

    // Step 9) If our king is in a different bucket, we must refresh our NNUE
    if (refresh){
        pos[stk].nnue.refresh(board, kingSq(white), kingSq(black));
    }
}

void position::undoLastMove(){
    // Step 1) Flip color (so our color is the color that is moving)
    turn ^= 1;

    // Step 2) Decode and get info
    Move move = pos[stk - 1].move;
    Square st = moveFrom(move);
    Square en = moveTo(move);
    Piece promo = movePromo(move);
    Piece pieceType = getPieceType(board[en]);
    Piece captType = pos[stk - 1].moveCaptType;

    // Step 3) En passant (note isEP won't work since we are working backwards)
    if (pieceType == pawn and getFile(st) != getFile(en) and captType == noPiece){
        Square captSq = (turn == white ? en - 8 : en + 8);
        movePiece(pawn, en, st, turn, false);
        addPiece(pawn, captSq, !turn, false);
    }

    // Step 4) Promotion
    else if (promo){
        removePiece(en, false);
        addPiece(pawn, st, turn, false);
        if (captType != noPiece){
            addPiece(captType, en, !turn, false);
        }
    }

    // Step 5) Castle
    else if (pieceType == king and abs(getFile(st) - getFile(en)) == 2){
        Square stRook, enRook;
        
        if (st < en){ // Kingside
            stRook = (turn == white ? h1 : h8); 
            enRook = (turn == white ? f1 : f8);
        }
        else{ // Queenside
            stRook = (turn == white ? a1 : a8);
            enRook = (turn == white ? d1 : d8);
        }
        movePiece(king, en, st, turn, false);
        movePiece(rook, enRook, stRook, turn, false);
    }

    // Step 6) All other moves
    else{
        movePiece(pieceType, en, st, turn, false);
        if (captType){
            addPiece(captType, en, !turn, false);
        }
    }

    // Step 7) Rollback stacks
    stk--;
}

void position::makeNullMove(){
    // Update all the stack stuff
    stk++;

    pos[stk - 1].move = nullOrNoMove;
    pos[stk - 1].moveCaptType = noPiece;
    
    pos[stk].castleRights = pos[stk - 1].castleRights;
    pos[stk].epFile = noEP;

    pos[stk].halfMoveClock = pos[stk - 1].halfMoveClock + 1;
    pos[stk].moveCount = pos[stk - 1].moveCount + 1;

    pos[stk].zhash = pos[stk - 1].zhash ^ ttRngTurn ^ ttRngEnpass[pos[stk - 1].epFile] ^ ttRngEnpass[pos[stk].epFile];

    pos[stk].nnue = pos[stk - 1].nnue;

    turn ^= 1;
}

void position::undoNullMove(){
    stk--;
    turn ^= 1;
}