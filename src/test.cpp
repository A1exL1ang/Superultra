#include "test.h"
#include "board.h"
#include "types.h"
#include "helpers.h"
#include "tt.h"

static position board;

const static testStruct testFens[] = {
    testStruct("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4, 4085603),
    testStruct("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 6, 11030083),
    testStruct("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5, 15833292),
    testStruct("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487),
    testStruct("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 4, 3894594),
    testStruct("8/8/8/8/8/8/8/R3K2k w Q - 1 1", 8, 17450766),
    testStruct("r3k2r/1p4p1/8/P6P/p6p/8/1P4P1/R3K2R w KQkq - 0 1", 5, 7856793),
    testStruct("8/8/8/8/k2p2RK/8/4P3/8 w - - 0 1", 7, 25832032),
    testStruct("4k3/8/8/8/8/8/ppp3pp/R3K2R w KQ - 0 1", 6, 18377259)
};

void position::verifyConsistency(){
    // Verify hash
    ttKey_t correctHash = ttRngCastle[stk->castleRights] 
                        ^ ttRngEnpass[stk->epFile] 
                        ^ (ttRngTurn * turn);

    for (square_t sq = 0; sq < 64; sq++)
        if (board[sq])
            correctHash ^= ttRngPiece[getPieceType(board[sq])][getPieceColor(board[sq])][sq];

    if (correctHash != stk->zhash){
        std::cout<<"Fen: "<<getFen()<<std::endl;
        std::cout<<"Got Hash: "<<stk->zhash<<", Expected hash: "<<correctHash<<std::endl;
    }

    // Verify in check
    bitboard_t attacked = 0;
    
    attacked |= pawnsAllAttack(pieceBB[pawn][!turn], !turn)
              | kingAttack(lsb(pieceBB[king][!turn]));

    for (bitboard_t m = pieceBB[knight][!turn]; m;)
        attacked |= knightAttack(poplsb(m));
    for (bitboard_t m = pieceBB[bishop][!turn]; m;)
        attacked |= bishopAttack(poplsb(m), allBB);
    for (bitboard_t m = pieceBB[rook][!turn]; m;)
        attacked |= rookAttack(poplsb(m), allBB);
    for (bitboard_t m = pieceBB[queen][!turn]; m;)
        attacked |= queenAttack(poplsb(m), allBB);
    
    bool correctInCheck = (attacked & pieceBB[king][turn]);

    if (correctInCheck != inCheck()){
        std::cout<<"Fen: "<<getFen()<<std::endl;
        std::cout<<"Got check: "<<inCheck()<<", Expected check: "<<correctInCheck<<std::endl;
    }

    // Verify that we only generated captures
    moveList moves, ourResult, expected;
    genAllMoves(false, moves);
    genAllMoves(true, ourResult);

    for (int i = 0; i < moves.sz; i++){
        // Non-quiet move if one of the following is true:
        // a) Capture
        // b) Promotion

        move_t move = moves.moves[i].move;

        if (moveCaptType(move) or movePromo(move))
            expected.addMove(move);
    }
    if (ourResult.sz != expected.sz){
        std::cout<<"Fen: "<<getFen()<<std::endl;
        std::cout<<"Got size: "<<ourResult.sz<<", Expected size: "<<expected.sz<<std::endl;
    }
}

static uint64 perft(depth_t depth, depth_t depthLim){
    moveList moves;
    board.verifyConsistency();
    board.genAllMoves(false, moves);

    // Return number of leaves without directly exploring them
    if (depth + 1 >= depthLim)
        return moves.sz;
    
    uint64 leaves = 0;
    for (int i = 0; i < moves.sz; i++){
        move_t move = moves.moves[i].move;
        board.makeMove(move);

        uint64 value = perft(depth + 1, depthLim);
        leaves += value;

        board.undoLastMove();
    }
    return leaves;
}

void testPerft(){
    for (testStruct test : testFens){
        board.readFen(test.fen);
        uint64 leaves = perft(0, test.depthLim);

        if (leaves != test.expected){
            std::cout<<"Fen: "<<test.fen<<std::endl;
            std::cout<<"Got Leaves: "<<leaves<<", Expected leaves: "<<test.expected<<std::endl;
        }
    }
}