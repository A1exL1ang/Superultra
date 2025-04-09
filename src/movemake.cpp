#include "assert.h"
#include "board.h"
#include "helpers.h"
#include "types.h"
#include "tt.h"

void Position::addPiece(Piece pieceType, Square sq, Color col, bool updateAccum){
    // Assumes position is empty
    assert(!board[sq]);

    pieceBB[pieceType][col] ^= (1ULL << sq);
    colorBB[col] ^= (1ULL << sq);
    allBB ^= (1ULL << sq);
    board[sq] = encodePiece(pieceType, col);
    pos[stk].zhash ^= ttRngPiece[pieceType][col][sq];

    if (updateAccum){
        pos[stk].nnue.addFeature(pieceType, sq, col, kingSq(WHITE), kingSq(BLACK));
    }
}

void Position::removePiece(Square sq, bool updateAccum){
    // Assumes there's is a piece at sq
    assert(board[sq]);

    Piece pieceType = getPieceType(board[sq]);
    Color col = getPieceColor(board[sq]);

    pieceBB[pieceType][col] ^= (1ULL << sq);
    colorBB[col] ^= (1ULL << sq);
    allBB ^= (1ULL << sq);
    board[sq] = NO_PIECE;
    pos[stk].zhash ^= ttRngPiece[pieceType][col][sq];

    if (updateAccum){
        pos[stk].nnue.removeFeature(pieceType, sq, col, kingSq(WHITE), kingSq(BLACK));
    }
}

void Position::movePiece(Piece piece, Square st, Square en, Color col, bool updateAccum){
    // Assumes there's a piece at st and no piece at en
    assert(board[st] and !board[en]);

    pieceBB[piece][col] ^= (1ULL << st) ^ (1ULL << en);
    colorBB[col] ^= (1ULL << st) ^ (1ULL << en);
    allBB ^= (1ULL << st) ^ (1ULL << en);
    board[st] = NO_PIECE;
    board[en] = encodePiece(piece, col);
    pos[stk].zhash ^= ttRngPiece[piece][col][st] ^ ttRngPiece[piece][col][en];
    
    if (updateAccum){ 
        pos[stk].nnue.updateMove(piece, st, en, col, kingSq(WHITE), kingSq(BLACK));
    }
}

void Position::makeMove(Move move){
    // Step 1) Decode and get necessary info
    Square st = moveFrom(move);
    Square en = moveTo(move);
    Piece promo = movePromo(move);
    Piece pieceType = getPieceType(board[st]);
    Piece captType = getPieceType(board[en]);
    bool refresh = (pieceType == KING and KING_BUCKET_ID[turn == WHITE ? st : flip(st)] != KING_BUCKET_ID[turn == WHITE ? en : flip(en)]);

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
        Square enemyPawnSq = (turn == WHITE ? en - 8 : en + 8);
        movePiece(PAWN, st, en, turn, !refresh);
        removePiece(enemyPawnSq, !refresh);
    }

    // Step 4) Promotion
    else if (promo){
        if (captType != NO_PIECE){
            removePiece(en, !refresh);
        }
        removePiece(st, !refresh);
        addPiece(promo, en, turn, !refresh);
    }

    // Step 5) Castle
    else if (pieceType == KING and abs(getFile(st) - getFile(en)) == 2){
        Square stRook;
        Square enRook;
        
        // Kingside
        if (st < en){ 
            stRook = (turn == WHITE ? SQ_H1 : SQ_H8); 
            enRook = (turn == WHITE ? SQ_F1 : SQ_F8);
        }
        // Queenside
        else{ 
            stRook = (turn == WHITE ? SQ_A1 : SQ_A8);
            enRook = (turn == WHITE ? SQ_D1 : SQ_D8);
        }
        movePiece(KING, st, en, turn, !refresh);
        movePiece(ROOK, stRook, enRook, turn, !refresh);
    }

    // Step 6) All other moves
    else{
        if (captType != NO_PIECE){
            removePiece(en, !refresh);
        }
        movePiece(pieceType, st, en, turn, !refresh);
    }

    // Step 7) Clock
    pos[stk].halfMoveClock = (captType != NO_PIECE or pieceType == PAWN) ? 0 : pos[stk].halfMoveClock + 1;
    pos[stk].moveCount++;

    // Step 8a) Clear zhash of ep and castle rights so we can fold everything into it afterwards
    pos[stk].zhash ^= ttRngCastle[pos[stk].castleRights] ^ ttRngEnpass[pos[stk].epFile];
    
    // Step 8b) En passant rights
    pos[stk].epFile = (pieceType == PAWN and abs(getRank(st) - getRank(en)) == 2) ? getFile(st) : NO_EP;
    
    // Step 8c) Castle rights if we move king
    if (pieceType == KING){
        pos[stk].castleRights &= (turn == WHITE ? (CASTLE_BLACK_K | CASTLE_BLACK_Q) : (CASTLE_WHITE_K | CASTLE_WHITE_Q));
    }
    
    // Step 8d) Castle rights if we altered any rook positions through moving or capturing 
    pos[stk].castleRights &= ((st != SQ_H1 and en != SQ_H1) * CASTLE_WHITE_K)
                           | ((st != SQ_A1 and en != SQ_A1) * CASTLE_WHITE_Q)
                           | ((st != SQ_H8 and en != SQ_H8) * CASTLE_BLACK_K)
                           | ((st != SQ_A8 and en != SQ_A8) * CASTLE_BLACK_Q);

    // Step 8e) Flip color
    turn ^= 1;

    // Step 8f) Update zhash
    pos[stk].zhash ^= ttRngCastle[pos[stk].castleRights] ^ ttRngEnpass[pos[stk].epFile] ^ ttRngTurn;

    // Step 9) If our king is in a different bucket, we must refresh our NNUE
    if (refresh){
        pos[stk].nnue.refresh(board, kingSq(WHITE), kingSq(BLACK));
    }
}

void Position::undoLastMove(){
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
    if (pieceType == PAWN and getFile(st) != getFile(en) and captType == NO_PIECE){
        Square captSq = (turn == WHITE ? en - 8 : en + 8);
        movePiece(PAWN, en, st, turn, false);
        addPiece(PAWN, captSq, !turn, false);
    }

    // Step 4) Promotion
    else if (promo){
        removePiece(en, false);
        addPiece(PAWN, st, turn, false);
        if (captType != NO_PIECE){
            addPiece(captType, en, !turn, false);
        }
    }

    // Step 5) Castle
    else if (pieceType == KING and abs(getFile(st) - getFile(en)) == 2){
        Square stRook, enRook;
        
        if (st < en){ // Kingside
            stRook = (turn == WHITE ? SQ_H1 : SQ_H8); 
            enRook = (turn == WHITE ? SQ_F1 : SQ_F8);
        }
        else{ // Queenside
            stRook = (turn == WHITE ? SQ_A1 : SQ_A8);
            enRook = (turn == WHITE ? SQ_D1 : SQ_D8);
        }
        movePiece(KING, en, st, turn, false);
        movePiece(ROOK, enRook, stRook, turn, false);
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

void Position::makeNullMove(){
    // Update all the stack stuff
    stk++;

    pos[stk - 1].move = NULL_OR_NO_MOVE;
    pos[stk - 1].moveCaptType = NO_PIECE;
    
    pos[stk].castleRights = pos[stk - 1].castleRights;
    pos[stk].epFile = NO_EP;

    pos[stk].halfMoveClock = pos[stk - 1].halfMoveClock + 1;
    pos[stk].moveCount = pos[stk - 1].moveCount + 1;

    pos[stk].zhash = pos[stk - 1].zhash ^ ttRngTurn ^ ttRngEnpass[pos[stk - 1].epFile] ^ ttRngEnpass[pos[stk].epFile];

    pos[stk].nnue = pos[stk - 1].nnue;

    turn ^= 1;
}

void Position::undoNullMove(){
    stk--;
    turn ^= 1;
}