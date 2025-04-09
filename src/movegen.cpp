#include "board.h"
#include "movepick.h"
#include "attacks.h"

void Position::calcPins(Bitboard &pinHV, Bitboard &pinDA){
    // Diagonal
    Bitboard maskDA = ((pieceBB[BISHOP][!turn] | pieceBB[QUEEN][!turn]) & bishopAttack(kingSq(turn), colorBB[!turn]));
    
    while (maskDA){
        Square sq = poplsb(maskDA);

        // Path to king (exclusive of both endpoints) 
        Bitboard toKing = getLine(sq, kingSq(turn)) ^ pieceBB[KING][turn] ^ (1ULL << sq); 

        // Check if they pin us
        if (countOnes(toKing & colorBB[turn]) == 1){
            pinDA |= toKing | (1ULL << sq); 
        }
    }

    // Horizontal
    Bitboard maskHV = ((pieceBB[ROOK][!turn] | pieceBB[QUEEN][!turn]) & rookAttack(kingSq(turn), colorBB[!turn]));

    while (maskHV){
        Square sq = poplsb(maskHV);

        // Path to king (exclusive of both endpoints) 
        Bitboard toKing = getLine(sq, kingSq(turn)) ^ pieceBB[KING][turn] ^ (1ULL << sq); 

        // Check if they pin us
        if (countOnes(toKing & colorBB[turn]) == 1){
            pinHV |= toKing | (1ULL << sq); 
        }
    }
}

void Position::calcAttacks(Bitboard &attacked, Bitboard &okSq){
    // Pawns
    Bitboard pawnAtt = pawnsAllAttack(pieceBB[PAWN][!turn], !turn);
    attacked |= pawnAtt;
    
    if (pawnAtt & pieceBB[KING][turn]){
        okSq &= (pieceBB[PAWN][!turn] & pawnAttack(kingSq(turn), turn));
    }

    // King
    Bitboard kingAtt = kingAttack(lsb(pieceBB[KING][!turn]));
    attacked |= kingAtt;

    // Knights
    for (Bitboard m = pieceBB[KNIGHT][!turn]; m;){
        Square sq = poplsb(m);

        Bitboard att = knightAttack(sq);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[KING][turn]){
            okSq &= (1ULL << sq);
        }
    }

    // Bishops
    for (Bitboard m = pieceBB[BISHOP][!turn]; m;){
        Square sq = poplsb(m);

        Bitboard att = bishopAttack(sq, allBB ^ pieceBB[KING][turn]);
        attacked |= att;
        
        // They hit our king
        if (att & pieceBB[KING][turn]){
            okSq &= getLine(sq, kingSq(turn));
        }
    }

    // Rooks
    for (Bitboard m = pieceBB[ROOK][!turn]; m;){
        Square sq = poplsb(m);

        Bitboard att = rookAttack(sq, allBB ^ pieceBB[KING][turn]);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[KING][turn]){
            okSq &= getLine(sq, kingSq(turn));
        }
    }

    // Queens
    for (Bitboard m = pieceBB[QUEEN][!turn]; m;){
        Square sq = poplsb(m);

        Bitboard att = queenAttack(sq, allBB ^ pieceBB[KING][turn]);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[KING][turn]){
            okSq &= getLine(sq, kingSq(turn));
        }
    }
}

void Position::genPawnMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    // Ranks relative to stm
    Bitboard RANK_3 = ALL_IN_RANK[turn == WHITE ? 2 : 5];
    Bitboard RANK_8 = ALL_IN_RANK[turn == WHITE ? 7 : 0];

    // Pinned pawns
    Bitboard pawnNotPinned = (pieceBB[PAWN][turn] & (~pinHV) & (~pinDA));
    Bitboard pawnPinnedDA = (pieceBB[PAWN][turn] & pinDA);
    Bitboard pawnPinnedHV = (pieceBB[PAWN][turn] & pinHV);

    // Directional captures. Either the pawn is not pinned at all or is diagonally 
    // pinned. If it is the latter, then the pawn can only capture in one direction
    
    Bitboard pawnCaptLeftEnd = (pawnsLeftAttack(pawnNotPinned, turn) | (pawnsLeftAttack(pawnPinnedDA, turn) & pinDA));
    Bitboard pawnCaptRightEnd = (pawnsRightAttack(pawnNotPinned, turn) | (pawnsRightAttack(pawnPinnedDA, turn) & pinDA));
    pawnCaptLeftEnd &= (colorBB[!turn] & okSq);
    pawnCaptRightEnd &= (colorBB[!turn] & okSq);

    // Pawn pushes. Either the pawn is not pinned at all or is vertically pinned
    Bitboard pawnPushEnd = (pawnsUp(pawnNotPinned, turn) | (pawnsUp(pawnPinnedHV, turn) & pinHV));
    Bitboard pawnDoublePushEnd = pawnsUp(((pawnPushEnd & (~allBB)) & RANK_3), turn);
    pawnPushEnd &= (okSq & (~allBB));
    pawnDoublePushEnd &= (okSq & (~allBB));

    // Add left captures
    while (pawnCaptLeftEnd){
        Square en = poplsb(pawnCaptLeftEnd);
        Square st = (turn == WHITE ? en - 7 : en + 9);

        if ((1ULL << en) & RANK_8){
            moves.addMove(encodeMove(st, en, QUEEN));
            moves.addMove(encodeMove(st, en, ROOK));
            moves.addMove(encodeMove(st, en, BISHOP));
            moves.addMove(encodeMove(st, en, KNIGHT));
        }
        else{
            moves.addMove(encodeMove(st, en, NO_PIECE));
        }
    }
    
    // Add right captures
    while (pawnCaptRightEnd){
        Square en = poplsb(pawnCaptRightEnd);
        Square st = (turn == WHITE ? en - 9 : en + 7);

        if ((1ULL << en) & RANK_8){
            moves.addMove(encodeMove(st, en, QUEEN));
            moves.addMove(encodeMove(st, en, ROOK));
            moves.addMove(encodeMove(st, en, BISHOP));
            moves.addMove(encodeMove(st, en, KNIGHT));
        }
        else{
            moves.addMove(encodeMove(st, en, NO_PIECE));
        }
    }

    // Add pushes and filter noisy data if we have that flag set
    if (noisy){
        pawnPushEnd &= RANK_8;
        pawnDoublePushEnd = 0;
    }

    while (pawnPushEnd){
        Square en = poplsb(pawnPushEnd);
        Square st = (turn == WHITE ? en - 8 : en + 8);

        if ((1ULL << en) & RANK_8){
            moves.addMove(encodeMove(st, en, QUEEN));
            moves.addMove(encodeMove(st, en, ROOK));
            moves.addMove(encodeMove(st, en, BISHOP));
            moves.addMove(encodeMove(st, en, KNIGHT));
        }
        else{
            moves.addMove(encodeMove(st, en, NO_PIECE));
        }
    }
    while (pawnDoublePushEnd){
        Square en = poplsb(pawnDoublePushEnd);
        Square st = (turn == WHITE ? en - 16 : en + 16);
        moves.addMove(encodeMove(st, en, NO_PIECE));
    }
    
    // En passant (we can do a partial legality check which isnt costly as ep is pretty rare)
    if (pos[stk].epFile != NO_EP){
        Square epEnemyPawn = pos[stk].epFile + (turn == WHITE ? 32 : 24);
        Square epDestination = pos[stk].epFile + (turn == WHITE ? 40 : 16);
        Bitboard pawnCandidateEpStart = (pawnAttack(epDestination, !turn) & pieceBB[PAWN][turn]);

        while (pawnCandidateEpStart){
            Square st = poplsb(pawnCandidateEpStart);
            Bitboard occupancyAfter = (allBB ^ (1ULL << st) ^ (1ULL << epDestination) ^ (1ULL << epEnemyPawn));
            
            if (!(bishopAttack(kingSq(turn), occupancyAfter) & pieceBB[BISHOP][!turn])
               and !(rookAttack(kingSq(turn), occupancyAfter) & pieceBB[ROOK][!turn])
               and !(queenAttack(kingSq(turn), occupancyAfter) & pieceBB[QUEEN][!turn]))
            {
                moves.addMove(encodeMove(st, epDestination, NO_PIECE));
            }
        }
    }
}

void Position::genKnightMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    for (Bitboard m = (pieceBB[KNIGHT][turn] & (~(pinHV | pinDA))); m;){
        Square st = poplsb(m);
        Bitboard maskMoves = (knightAttack(st) & okSq);

        if (noisy){
            maskMoves &= colorBB[!turn];
        }
        while (maskMoves){
            Square en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, NO_PIECE));
        }
    }
}

void Position::genBishopMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    for (Bitboard m = (pieceBB[BISHOP][turn] & (~pinHV)); m;){
        Square st = poplsb(m);

        Bitboard maskMoves = (bishopAttack(st, allBB) 
                             & okSq
                             & ((pinDA & (1ULL << st)) ? pinDA : ALL));

        if (noisy){
            maskMoves &= colorBB[!turn];
        }
        while (maskMoves){
            Square en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, NO_PIECE));
        }
    }
}

void Position::genRookMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    for (Bitboard m = (pieceBB[ROOK][turn] & (~pinDA)); m;){
        Square st = poplsb(m);

        Bitboard maskMoves = (rookAttack(st, allBB) 
                             & okSq
                             & ((pinHV & (1ULL << st)) ? pinHV : ALL));

        if (noisy){
            maskMoves &= colorBB[!turn];
        }
        while (maskMoves){
            Square en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, NO_PIECE));
        }      
    }
}

void Position::genQueenMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    for (Bitboard m = (pieceBB[QUEEN][turn] & (~(pinHV & pinDA))); m;){
        Square sq = poplsb(m);

        Bitboard bishopMoves = (bishopAttack(sq, allBB) & okSq);
        Bitboard rookMoves = (rookAttack(sq, allBB) & okSq);
        Bitboard maskMoves = bishopMoves | rookMoves;

        // Pinned diagonally
        if ((1ULL << sq) & pinDA){
            maskMoves &= (bishopMoves & pinDA);
        }
        // Pinned horizontally
        if ((1ULL << sq) & pinHV){
            maskMoves &= (rookMoves & pinHV);
        }
        if (noisy){
            maskMoves &= colorBB[!turn];
        }
        while (maskMoves){
            Square en = poplsb(maskMoves);
            moves.addMove(encodeMove(sq, en, NO_PIECE));
        }  
    }
}

void Position::genKingMoves(bool noisy, Bitboard attacked, moveList &moves){
    Square sq = kingSq(turn);

    // Generate regular moves
    Bitboard maskRegularMoves = (kingAttack(sq) & (~attacked) & (~colorBB[turn]));

    if (noisy){
        maskRegularMoves &= colorBB[!turn];
    }
    while (maskRegularMoves){
        Square en = poplsb(maskRegularMoves);
        moves.addMove(encodeMove(sq, en, NO_PIECE));
    }
    
    // Generate castle
    if (!noisy){
        Bitboard maskCastleMoves = 0;

        if (turn == WHITE){
            maskCastleMoves = (1ULL << SQ_G1) * ((pos[stk].castleRights & CASTLE_WHITE_K) and !(attacked & getLine(SQ_E1, SQ_G1)) and !(allBB & getLine(SQ_F1, SQ_G1)))
                            | (1ULL << SQ_C1) * ((pos[stk].castleRights & CASTLE_WHITE_Q) and !(attacked & getLine(SQ_E1, SQ_C1)) and !(allBB & getLine(SQ_B1, SQ_D1)));
        }
        else{
            maskCastleMoves = (1ULL << SQ_G8) * ((pos[stk].castleRights & CASTLE_BLACK_K) and !(attacked & getLine(SQ_E8, SQ_G8)) and !(allBB & getLine(SQ_F8, SQ_G8)))
                            | (1ULL << SQ_C8) * ((pos[stk].castleRights & CASTLE_BLACK_Q) and !(attacked & getLine(SQ_E8, SQ_C8)) and !(allBB & getLine(SQ_B8, SQ_D8)));
        }
        while (maskCastleMoves){
            Square en = poplsb(maskCastleMoves);
            moves.addMove(encodeMove(sq, en, NO_PIECE));
        }
    }
}

void Position::genAllMoves(bool noisy, moveList &moves){
    // Init
    Bitboard pinHV = 0;
    Bitboard pinDA = 0;
    Bitboard attacked = 0;
    Bitboard okSq = ~colorBB[turn];

    // Calculate variables
    calcPins(pinHV, pinDA);
    calcAttacks(attacked, okSq);

    // If in double check only generate king moves
    if (countOnes(okSq) == 0){
        genKingMoves(noisy, attacked, moves);
        return;
    }

    // Generate moves
    genPawnMoves(noisy, pinHV, pinDA, okSq, moves);
    genKnightMoves(noisy, pinHV, pinDA, okSq, moves);
    genBishopMoves(noisy, pinHV, pinDA, okSq, moves);
    genRookMoves(noisy, pinHV, pinDA, okSq, moves);
    genQueenMoves(noisy, pinHV, pinDA, okSq, moves);
    genKingMoves(noisy, attacked, moves);
}