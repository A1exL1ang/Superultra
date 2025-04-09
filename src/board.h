#pragma once

#include <vector>
#include "types.h"
#include "helpers.h"
#include "movepick.h"
#include "nnue.h"
#include "attacks.h"

struct BoardState{
    Piece moveCaptType;
    Move move;

    int8 castleRights; 
    File epFile;
    TTKey zhash;
    
    int halfMoveClock;
    int moveCount;

    NeuralNetwork nnue;
};

class Position{

public:
    // Move gen (fully legal)
    void genAllMoves(bool noisy, moveList &moves);

    // Legality related for staged movegen
    bool isLegal(Move move);

    // Move make/unmake
    void makeMove(Move move);
    void undoLastMove();
    void makeNullMove();
    void undoNullMove();
    
    // Eval related
    bool seeGreater(Move move, Score value);
    bool drawByRepetition(Depth searchPly);
    bool drawByInsufficientMaterial();
    bool drawByFiftyMoveRule();
    Score eval();

    // Fen and debug related
    void readFen(std::string fen);
    std::string getFen();
    void resetStack();

    // Interface and utility access
    inline Color getTurn(){
        return turn;
    }
    inline Bitboard pieceAt(Square sq){
        return board[sq];
    }
    inline Bitboard allPiece(Piece piece){
        return pieceBB[piece][BLACK] | pieceBB[piece][WHITE];
    }
    inline Square kingSq(Color col){
        return lsb(pieceBB[KING][col]);
    }
    inline TTKey getHash(){
        return pos[stk].zhash;
    }
    inline Bitboard allAttack(Color col, Bitboard occupancy){
        Bitboard attacked = 0;
        attacked |= pawnsAllAttack(pieceBB[PAWN][col], col) | kingAttack(kingSq(col));

        for (Bitboard m = pieceBB[KNIGHT][col]; m;){
            attacked |= knightAttack(poplsb(m));
        }
        for (Bitboard m = pieceBB[BISHOP][col]; m;){
            attacked |= bishopAttack(poplsb(m), occupancy);
        }
        for (Bitboard m = pieceBB[ROOK][col]; m;){
            attacked |= rookAttack(poplsb(m), occupancy);
        }
        for (Bitboard m = pieceBB[QUEEN][col]; m;){
            attacked |= queenAttack(poplsb(m), occupancy);
        }
        return attacked;
    }
    inline bool inCheck(){
        return (allAttack(!turn, allBB) & pieceBB[KING][turn]);
    }
    inline Piece movePieceEnc(Move move){
        return board[moveFrom(move)];
    }
    inline Piece movePieceType(Move move){
        return getPieceType(board[moveFrom(move)]);
    }
    inline Piece movePieceColor(Move move){
        return getPieceColor(board[moveFrom(move)]);
    }
    inline Piece moveCaptType(Move move){
        return isEP(move) ? PAWN : getPieceType(board[moveTo(move)]);
    }
    inline bool isEP(Move move){
        // Assumes 'move' is legal so don't use this for legality checks
        return getPieceType(board[moveFrom(move)]) == PAWN 
               and getFile(moveFrom(move)) != getFile(moveTo(move))
               and board[moveTo(move)] == NO_PIECE;
    }
    inline bool hasMajorPieceLeft(Color col){
        return (pieceBB[KNIGHT][col] | pieceBB[BISHOP][col] | pieceBB[ROOK][col] | pieceBB[QUEEN][col]);
    }

private:
    // Position stack (UCI doesnt support undo so we store the minimum (up to 50mr) and reset the stack when necessary)
    std::vector<BoardState> pos;
    int stk;

    // Board variables
    Piece board[64];
    Bitboard pieceBB[7][2];
    Bitboard colorBB[2];
    Bitboard allBB;
    Color turn;

    // Move gen helpers
    void calcPins(Bitboard &pinHV, Bitboard &pinDA);
    void calcAttacks(Bitboard &attacked, Bitboard &okSq);

    void genPawnMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves);
    void genKnightMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves);
    void genBishopMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves);
    void genRookMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves);
    void genQueenMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves);
    void genKingMoves(bool noisy, Bitboard attacked, moveList &moves);
    
    // Make and unmake helpers
    void addPiece(Piece pieceType, Square sq, Color col, bool updateAccum);
    void removePiece(Square sq, bool updateAccum);
    void movePiece(Piece pieceType, Square st, Square en, Color col, bool updateAccum);
};