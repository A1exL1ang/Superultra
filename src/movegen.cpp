#include "board.h"
#include "movepick.h"
#include "attacks.h"

void position::calcPins(Bitboard &pinHV, Bitboard &pinDA){
    // Diagonal
    Bitboard maskDA = ((pieceBB[bishop][!turn] | pieceBB[queen][!turn]) & bishopAttack(kingSq(turn), colorBB[!turn]));
    
    while (maskDA){
        Square sq = poplsb(maskDA);

        // Path to king (exclusive of both endpoints) 
        Bitboard toKing = getLine(sq, kingSq(turn)) ^ pieceBB[king][turn] ^ (1ULL << sq); 

        // Check if they pin us
        if (countOnes(toKing & colorBB[turn]) == 1){
            pinDA |= toKing | (1ULL << sq); 
        }
    }

    // Horizontal
    Bitboard maskHV = ((pieceBB[rook][!turn] | pieceBB[queen][!turn]) & rookAttack(kingSq(turn), colorBB[!turn]));

    while (maskHV){
        Square sq = poplsb(maskHV);

        // Path to king (exclusive of both endpoints) 
        Bitboard toKing = getLine(sq, kingSq(turn)) ^ pieceBB[king][turn] ^ (1ULL << sq); 

        // Check if they pin us
        if (countOnes(toKing & colorBB[turn]) == 1){
            pinHV |= toKing | (1ULL << sq); 
        }
    }
}

void position::calcAttacks(Bitboard &attacked, Bitboard &okSq){
    // Pawns
    Bitboard pawnAtt = pawnsAllAttack(pieceBB[pawn][!turn], !turn);
    attacked |= pawnAtt;
    
    if (pawnAtt & pieceBB[king][turn]){
        okSq &= (pieceBB[pawn][!turn] & pawnAttack(kingSq(turn), turn));
    }

    // King
    Bitboard kingAtt = kingAttack(lsb(pieceBB[king][!turn]));
    attacked |= kingAtt;

    // Knights
    for (Bitboard m = pieceBB[knight][!turn]; m;){
        Square sq = poplsb(m);

        Bitboard att = knightAttack(sq);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[king][turn]){
            okSq &= (1ULL << sq);
        }
    }

    // Bishops
    for (Bitboard m = pieceBB[bishop][!turn]; m;){
        Square sq = poplsb(m);

        Bitboard att = bishopAttack(sq, allBB ^ pieceBB[king][turn]);
        attacked |= att;
        
        // They hit our king
        if (att & pieceBB[king][turn]){
            okSq &= getLine(sq, kingSq(turn));
        }
    }

    // Rooks
    for (Bitboard m = pieceBB[rook][!turn]; m;){
        Square sq = poplsb(m);

        Bitboard att = rookAttack(sq, allBB ^ pieceBB[king][turn]);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[king][turn]){
            okSq &= getLine(sq, kingSq(turn));
        }
    }

    // Queens
    for (Bitboard m = pieceBB[queen][!turn]; m;){
        Square sq = poplsb(m);

        Bitboard att = queenAttack(sq, allBB ^ pieceBB[king][turn]);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[king][turn]){
            okSq &= getLine(sq, kingSq(turn));
        }
    }
}

void position::genPawnMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    // Ranks relative to stm
    Bitboard rank3 = allInRank[turn == white ? 2 : 5];
    Bitboard rank8 = allInRank[turn == white ? 7 : 0];

    // Pinned pawns
    Bitboard pawnNotPinned = (pieceBB[pawn][turn] & (~pinHV) & (~pinDA));
    Bitboard pawnPinnedDA = (pieceBB[pawn][turn] & pinDA);
    Bitboard pawnPinnedHV = (pieceBB[pawn][turn] & pinHV);

    // Directional captures. Either the pawn is not pinned at all or is diagonally 
    // pinned. If it is the latter, then the pawn can only capture in one direction
    
    Bitboard pawnCaptLeftEnd = (pawnsLeftAttack(pawnNotPinned, turn) | (pawnsLeftAttack(pawnPinnedDA, turn) & pinDA));
    Bitboard pawnCaptRightEnd = (pawnsRightAttack(pawnNotPinned, turn) | (pawnsRightAttack(pawnPinnedDA, turn) & pinDA));
    pawnCaptLeftEnd &= (colorBB[!turn] & okSq);
    pawnCaptRightEnd &= (colorBB[!turn] & okSq);

    // Pawn pushes. Either the pawn is not pinned at all or is vertically pinned
    Bitboard pawnPushEnd = (pawnsUp(pawnNotPinned, turn) | (pawnsUp(pawnPinnedHV, turn) & pinHV));
    Bitboard pawnDoublePushEnd = pawnsUp(((pawnPushEnd & (~allBB)) & rank3), turn);
    pawnPushEnd &= (okSq & (~allBB));
    pawnDoublePushEnd &= (okSq & (~allBB));

    // Add left captures
    while (pawnCaptLeftEnd){
        Square en = poplsb(pawnCaptLeftEnd);
        Square st = (turn == white ? en - 7 : en + 9);

        if ((1ULL << en) & rank8){
            moves.addMove(encodeMove(st, en, queen));
            moves.addMove(encodeMove(st, en, rook));
            moves.addMove(encodeMove(st, en, bishop));
            moves.addMove(encodeMove(st, en, knight));
        }
        else{
            moves.addMove(encodeMove(st, en, noPiece));
        }
    }
    
    // Add right captures
    while (pawnCaptRightEnd){
        Square en = poplsb(pawnCaptRightEnd);
        Square st = (turn == white ? en - 9 : en + 7);

        if ((1ULL << en) & rank8){
            moves.addMove(encodeMove(st, en, queen));
            moves.addMove(encodeMove(st, en, rook));
            moves.addMove(encodeMove(st, en, bishop));
            moves.addMove(encodeMove(st, en, knight));
        }
        else{
            moves.addMove(encodeMove(st, en, noPiece));
        }
    }

    // Add pushes and filter noisy data if we have that flag set
    if (noisy){
        pawnPushEnd &= rank8;
        pawnDoublePushEnd = 0;
    }

    while (pawnPushEnd){
        Square en = poplsb(pawnPushEnd);
        Square st = (turn == white ? en - 8 : en + 8);

        if ((1ULL << en) & rank8){
            moves.addMove(encodeMove(st, en, queen));
            moves.addMove(encodeMove(st, en, rook));
            moves.addMove(encodeMove(st, en, bishop));
            moves.addMove(encodeMove(st, en, knight));
        }
        else{
            moves.addMove(encodeMove(st, en, noPiece));
        }
    }
    while (pawnDoublePushEnd){
        Square en = poplsb(pawnDoublePushEnd);
        Square st = (turn == white ? en - 16 : en + 16);
        moves.addMove(encodeMove(st, en, noPiece));
    }
    
    // En passant (we can do a partial legality check which isnt costly as ep is pretty rare)
    if (pos[stk].epFile != noEP){
        Square epEnemyPawn = pos[stk].epFile + (turn == white ? 32 : 24);
        Square epDestination = pos[stk].epFile + (turn == white ? 40 : 16);
        Bitboard pawnCandidateEpStart = (pawnAttack(epDestination, !turn) & pieceBB[pawn][turn]);

        while (pawnCandidateEpStart){
            Square st = poplsb(pawnCandidateEpStart);
            Bitboard occupancyAfter = (allBB ^ (1ULL << st) ^ (1ULL << epDestination) ^ (1ULL << epEnemyPawn));
            
            if (!(bishopAttack(kingSq(turn), occupancyAfter) & pieceBB[bishop][!turn])
               and !(rookAttack(kingSq(turn), occupancyAfter) & pieceBB[rook][!turn])
               and !(queenAttack(kingSq(turn), occupancyAfter) & pieceBB[queen][!turn]))
            {
                moves.addMove(encodeMove(st, epDestination, noPiece));
            }
        }
    }
}

void position::genKnightMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    for (Bitboard m = (pieceBB[knight][turn] & (~(pinHV | pinDA))); m;){
        Square st = poplsb(m);
        Bitboard maskMoves = (knightAttack(st) & okSq);

        if (noisy){
            maskMoves &= colorBB[!turn];
        }
        while (maskMoves){
            Square en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, noPiece));
        }
    }
}

void position::genBishopMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    for (Bitboard m = (pieceBB[bishop][turn] & (~pinHV)); m;){
        Square st = poplsb(m);

        Bitboard maskMoves = (bishopAttack(st, allBB) 
                             & okSq
                             & ((pinDA & (1ULL << st)) ? pinDA : all));

        if (noisy){
            maskMoves &= colorBB[!turn];
        }
        while (maskMoves){
            Square en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, noPiece));
        }
    }
}

void position::genRookMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    for (Bitboard m = (pieceBB[rook][turn] & (~pinDA)); m;){
        Square st = poplsb(m);

        Bitboard maskMoves = (rookAttack(st, allBB) 
                             & okSq
                             & ((pinHV & (1ULL << st)) ? pinHV : all));

        if (noisy){
            maskMoves &= colorBB[!turn];
        }
        while (maskMoves){
            Square en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, noPiece));
        }      
    }
}

void position::genQueenMoves(bool noisy, Bitboard pinHV, Bitboard pinDA, Bitboard okSq, moveList &moves){
    for (Bitboard m = (pieceBB[queen][turn] & (~(pinHV & pinDA))); m;){
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
            moves.addMove(encodeMove(sq, en, noPiece));
        }  
    }
}

void position::genKingMoves(bool noisy, Bitboard attacked, moveList &moves){
    Square sq = kingSq(turn);

    // Generate regular moves
    Bitboard maskRegularMoves = (kingAttack(sq) & (~attacked) & (~colorBB[turn]));

    if (noisy){
        maskRegularMoves &= colorBB[!turn];
    }
    while (maskRegularMoves){
        Square en = poplsb(maskRegularMoves);
        moves.addMove(encodeMove(sq, en, noPiece));
    }
    
    // Generate castle
    if (!noisy){
        Bitboard maskCastleMoves = 0;

        if (turn == white){
            maskCastleMoves = (1ULL << g1) * ((pos[stk].castleRights & castleWhiteK) and !(attacked & getLine(e1, g1)) and !(allBB & getLine(f1, g1)))
                            | (1ULL << c1) * ((pos[stk].castleRights & castleWhiteQ) and !(attacked & getLine(e1, c1)) and !(allBB & getLine(b1, d1)));
        }
        else{
            maskCastleMoves = (1ULL << g8) * ((pos[stk].castleRights & castleBlackK) and !(attacked & getLine(e8, g8)) and !(allBB & getLine(f8, g8)))
                            | (1ULL << c8) * ((pos[stk].castleRights & castleBlackQ) and !(attacked & getLine(e8, c8)) and !(allBB & getLine(b8, d8)));
        }
        while (maskCastleMoves){
            Square en = poplsb(maskCastleMoves);
            moves.addMove(encodeMove(sq, en, noPiece));
        }
    }
}

void position::genAllMoves(bool noisy, moveList &moves){
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