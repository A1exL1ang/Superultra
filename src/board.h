#include <vector>
#include "types.h"
#include "helpers.h"
#include "movepick.h"
#include "nnue.h"
#include "attacks.h"

#pragma once

struct boardState{
    piece_t captPieceType;
    move_t prevMove;

    int8 castleRights; 
    file_t epFile;
    ttKey_t zhash;
    
    int halfMoveClock;
    int moveCount;

    neuralNetwork nnue;
};

class position{

public:
    // Utility
    inline color_t getTurn(){
        return turn;
    }
    inline bitboard_t pieceAt(square_t sq){
        return board[sq];
    }
    inline bitboard_t allPiece(piece_t piece){
        return pieceBB[piece][black] | pieceBB[piece][white];
    }
    inline square_t kingSq(color_t col){
        return lsb(pieceBB[king][col]);
    }
    inline ttKey_t getHash(){
        return pos[stk].zhash;
    }
    inline bitboard_t allAttack(color_t col){
        bitboard_t attacked = 0;
        attacked |= pawnsAllAttack(pieceBB[pawn][col], col) | kingAttack(kingSq(col));

        for (bitboard_t m = pieceBB[knight][col]; m;)
            attacked |= knightAttack(poplsb(m));
        
        for (bitboard_t m = pieceBB[bishop][col]; m;)
            attacked |= bishopAttack(poplsb(m), allBB);

        for (bitboard_t m = pieceBB[rook][col]; m;)
            attacked |= rookAttack(poplsb(m), allBB);
        
        for (bitboard_t m = pieceBB[queen][col]; m;)
            attacked |= queenAttack(poplsb(m), allBB);
        
        return attacked;
    }
    inline bool inCheck(){
        return (allAttack(!turn) & pieceBB[king][turn]);
    }
    inline piece_t movePieceEnc(move_t move){
        return board[moveFrom(move)];
    }
    inline piece_t movePieceType(move_t move){
        return getPieceType(board[moveFrom(move)]);
    }
    inline piece_t moveCaptType(move_t move){
        return isEP(move) ? pawn : getPieceType(board[moveTo(move)]);
    }
    inline bool isEP(move_t move){
        return getPieceType(board[moveFrom(move)]) == pawn 
               and getFile(moveFrom(move)) != getFile(moveTo(move))
               and board[moveTo(move)] == noPiece;
    }
    inline bool hasMajorPieceLeft(color_t col){
        return (pieceBB[knight][col] | pieceBB[bishop][col] | pieceBB[rook][col] | pieceBB[queen][col]);
    }

    // Move gen
    void genAllMoves(bool noisy, moveList &moves);

    // Move make/unmake
    void makeMove(move_t move);
    void undoLastMove();
    void makeNullMove();
    void undoNullMove();
    
    // Eval related
    bool seeGreater(move_t move, score_t value);
    bool drawByRepetition(depth_t searchPly);
    bool drawByInsufficientMaterial();
    bool drawByFiftyMoveRule();
    score_t eval();

    // Fen and debug related
    void readFen(std::string fen);
    std::string getFen();
    std::string flippedFen();
    void resetStack();
    void printBoard();
    void verifyConsistency();

private:
    // Position stack (UCI doesnt support undo so we dont either)
    // We store the bare minimum and reset the stack when necessary
    boardState pos[maximumPly + 105];
    int stk;

    // Board variables
    piece_t board[64];
    bitboard_t pieceBB[7][2];
    bitboard_t colorBB[2];
    bitboard_t allBB;
    color_t turn;

    // Move gen helpers
    void calcPins(bitboard_t &pinHV, bitboard_t &pinDA);
    void calcAttacks(bitboard_t &attacked, bitboard_t &okSq);

    void genPawnMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves);
    void genKnightMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves);
    void genBishopMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves);
    void genRookMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves);
    void genQueenMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves);
    void genKingMoves(bool noisy, bitboard_t attacked, moveList &moves);
    
    // Make and unmake helpers
    void addPiece(piece_t pieceType, square_t sq, color_t col, bool updateAccum);
    void removePiece(square_t sq, bool updateAccum);
    void movePiece(piece_t pieceType, square_t st, square_t en, color_t col, bool updateAccum);
};