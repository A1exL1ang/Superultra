#include "board.h"
#include "movepick.h"
#include "attacks.h"

void position::calcPins(bitboard_t &pinHV, bitboard_t &pinDA){
    // Diagonal
    bitboard_t maskDA = ((pieceBB[bishop][!turn] | pieceBB[queen][!turn]) & bishopAttack(kingSq(turn), colorBB[!turn]));
    
    while (maskDA){
        square_t sq = poplsb(maskDA);

        // Path to king (exclusive of both endpoints) 
        bitboard_t toKing = between(sq, kingSq(turn)) ^ pieceBB[king][turn] ^ (1ULL << sq); 

        // Check if they pin us
        if (countOnes(toKing & colorBB[turn]) == 1)
            pinDA |= toKing | (1ULL << sq); 
    }

    // Horizontal
    bitboard_t maskHV = ((pieceBB[rook][!turn] | pieceBB[queen][!turn]) & rookAttack(kingSq(turn), colorBB[!turn]));

    while (maskHV){
        square_t sq = poplsb(maskHV);

        // Path to king (exclusive of both endpoints) 
        bitboard_t toKing = between(sq, kingSq(turn)) ^ pieceBB[king][turn] ^ (1ULL << sq); 

        // Check if they pin us
        if (countOnes(toKing & colorBB[turn]) == 1)
            pinHV |= toKing | (1ULL << sq); 
    }
}

void position::calcAttacks(bitboard_t &attacked, bitboard_t &okSq){
    // Pawns
    bitboard_t pawnAtt = pawnsAllAttack(pieceBB[pawn][!turn], !turn);
    attacked |= pawnAtt;
    
    if (pawnAtt & pieceBB[king][turn])
        okSq &= (pieceBB[pawn][!turn] & pawnAttack(kingSq(turn), turn));

    // King
    bitboard_t kingAtt = kingAttack(lsb(pieceBB[king][!turn]));
    attacked |= kingAtt;

    // Knights
    for (bitboard_t m = pieceBB[knight][!turn]; m;){
        square_t sq = poplsb(m);

        bitboard_t att = knightAttack(sq);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[king][turn])
            okSq &= (1ULL << sq);
    }

    // Bishops
    for (bitboard_t m = pieceBB[bishop][!turn]; m;){
        square_t sq = poplsb(m);

        bitboard_t att = bishopAttack(sq, allBB ^ pieceBB[king][turn]);
        attacked |= att;
        
        // They hit our king
        if (att & pieceBB[king][turn])
            okSq &= lineBB[sq][kingSq(turn)];
    }

    // Rooks
    for (bitboard_t m = pieceBB[rook][!turn]; m;){
        square_t sq = poplsb(m);

        bitboard_t att = rookAttack(sq, allBB ^ pieceBB[king][turn]);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[king][turn])
            okSq &= lineBB[sq][kingSq(turn)];
    }

    // Queens
    for (bitboard_t m = pieceBB[queen][!turn]; m;){
        square_t sq = poplsb(m);

        bitboard_t att = queenAttack(sq, allBB ^ pieceBB[king][turn]);
        attacked |= att;

        // They hit our king
        if (att & pieceBB[king][turn])
            okSq &= lineBB[sq][kingSq(turn)];
    }
}

void position::genPawnMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves){
    // Ranks relative to stm
    bitboard_t rank3 = allInRank[turn == white ? 2 : 5];
    bitboard_t rank8 = allInRank[turn == white ? 7 : 0];

    // Pinned pawns
    bitboard_t pawnNotPinned = (pieceBB[pawn][turn] & (~pinHV) & (~pinDA));
    bitboard_t pawnPinnedDA = (pieceBB[pawn][turn] & pinDA);
    bitboard_t pawnPinnedHV = (pieceBB[pawn][turn] & pinHV);

    // Directional captures. Either the pawn is not pinned at all or is diagonally 
    // pinned. If it is the latter, then the pawn can only capture in one direction.
    bitboard_t pawnCaptLeftEnd = (pawnsLeftAttack(pawnNotPinned, turn) | (pawnsLeftAttack(pawnPinnedDA, turn) & pinDA));
    bitboard_t pawnCaptRightEnd = (pawnsRightAttack(pawnNotPinned, turn) | (pawnsRightAttack(pawnPinnedDA, turn) & pinDA));
    pawnCaptLeftEnd &= (colorBB[!turn] & okSq);
    pawnCaptRightEnd &= (colorBB[!turn] & okSq);

    // Pawn pushes. Either the pawn is not pinned at all or is vertically pinned.
    bitboard_t pawnPushEnd = (pawnsUp(pawnNotPinned, turn) | (pawnsUp(pawnPinnedHV, turn) & pinHV));
    bitboard_t pawnDoublePushEnd = pawnsUp(((pawnPushEnd & (~allBB)) & rank3), turn);
    pawnPushEnd &= (okSq & (~allBB));
    pawnDoublePushEnd &= (okSq & (~allBB));

    // Add left captures
    while (pawnCaptLeftEnd){
        square_t en = poplsb(pawnCaptLeftEnd);
        square_t st = (turn == white ? en - 7 : en + 9);

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
        square_t en = poplsb(pawnCaptRightEnd);
        square_t st = (turn == white ? en - 9 : en + 7);

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
        square_t en = poplsb(pawnPushEnd);
        square_t st = (turn == white ? en - 8 : en + 8);

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
        square_t en = poplsb(pawnDoublePushEnd);
        square_t st = (turn == white ? en - 16 : en + 16);
        moves.addMove(encodeMove(st, en, noPiece));
    }
    
    // En passant (we can do a partial legality check which isnt costly as ep is pretty rare)
    if (stk->epFile != noEP){
        square_t epEnemyPawn = stk->epFile + (turn == white ? 32 : 24);
        square_t epDestination = stk->epFile + (turn == white ? 40 : 16);
        bitboard_t pawnCandidateEpStart = (pawnAttack(epDestination, !turn) & pieceBB[pawn][turn]);

        while (pawnCandidateEpStart){
            square_t st = poplsb(pawnCandidateEpStart);
            bitboard_t occupancyAfter = (allBB ^ (1ULL << st) ^ (1ULL << epDestination) ^ (1ULL << epEnemyPawn));
            
            if (!(bishopAttack(kingSq(turn), occupancyAfter) & pieceBB[bishop][!turn])
               and !(rookAttack(kingSq(turn), occupancyAfter) & pieceBB[rook][!turn])
               and !(queenAttack(kingSq(turn), occupancyAfter) & pieceBB[queen][!turn]))
            {
                moves.addMove(encodeMove(st, epDestination, noPiece));
            }
        }
    }
}

void position::genKnightMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves){
    for (bitboard_t m = (pieceBB[knight][turn] & (~(pinHV | pinDA))); m;){
        square_t st = poplsb(m);
        bitboard_t maskMoves = (knightAttack(st) & okSq);

        if (noisy)
            maskMoves &= colorBB[!turn];

        while (maskMoves){
            square_t en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, noPiece));
        }
    }
}

void position::genBishopMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves){
    for (bitboard_t m = (pieceBB[bishop][turn] & (~pinHV)); m;){
        square_t st = poplsb(m);

        bitboard_t maskMoves = (bishopAttack(st, allBB) 
                             & okSq
                             & ((pinDA & (1ULL << st)) ? pinDA : all));

        if (noisy)
            maskMoves &= colorBB[!turn];

        while (maskMoves){
            square_t en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, noPiece));
        }
    }
}

void position::genRookMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves){
    for (bitboard_t m = (pieceBB[rook][turn] & (~pinDA)); m;){
        square_t st = poplsb(m);

        bitboard_t maskMoves = (rookAttack(st, allBB) 
                             & okSq
                             & ((pinHV & (1ULL << st)) ? pinHV : all));

        if (noisy)
            maskMoves &= colorBB[!turn];

        while (maskMoves){
            square_t en = poplsb(maskMoves);
            moves.addMove(encodeMove(st, en, noPiece));
        }      
    }
}

void position::genQueenMoves(bool noisy, bitboard_t pinHV, bitboard_t pinDA, bitboard_t okSq, moveList &moves){
    for (bitboard_t m = (pieceBB[queen][turn] & (~(pinHV & pinDA))); m;){
        square_t sq = poplsb(m);

        bitboard_t bishopMoves = (bishopAttack(sq, allBB) & okSq);
        bitboard_t rookMoves = (rookAttack(sq, allBB) & okSq);
        bitboard_t maskMoves = bishopMoves | rookMoves;

        // Pinned diagonally
        if ((1ULL << sq) & pinDA)
            maskMoves &= (bishopMoves & pinDA);
        
        // Pinned horizontally
        if ((1ULL << sq) & pinHV)
            maskMoves &= (rookMoves & pinHV);

        if (noisy)
            maskMoves &= colorBB[!turn];

        while (maskMoves){
            square_t en = poplsb(maskMoves);
            moves.addMove(encodeMove(sq, en, noPiece));
        }  
    }
}

void position::genKingMoves(bool noisy, bitboard_t attacked, moveList &moves){
    square_t sq = kingSq(turn);

    // Generate regular moves
    bitboard_t maskRegularMoves = (kingAttack(sq) & (~attacked) & (~colorBB[turn]));

    if (noisy)
        maskRegularMoves &= colorBB[!turn];

    while (maskRegularMoves){
        square_t en = poplsb(maskRegularMoves);
        moves.addMove(encodeMove(sq, en, noPiece));
    }
    
    // Generate castle
    if (!noisy){
        bitboard_t maskCastleMoves = 0;

        if (turn == white){
            maskCastleMoves = (1ULL << g1) * ((stk->castleRights & castleWhiteK) and !(attacked & lineBB[e1][g1]) and !(allBB & lineBB[f1][g1]))
                            | (1ULL << c1) * ((stk->castleRights & castleWhiteQ) and !(attacked & lineBB[e1][c1]) and !(allBB & lineBB[b1][d1]));
        }
        else{
            maskCastleMoves = (1ULL << g8) * (turn == black and (stk->castleRights & castleBlackK) and !(attacked & lineBB[e8][g8]) and !(allBB & lineBB[f8][g8]))
                            | (1ULL << c8) * (turn == black and (stk->castleRights & castleBlackQ) and !(attacked & lineBB[e8][c8]) and !(allBB & lineBB[b8][d8]));
        }
        while (maskCastleMoves){
            square_t en = poplsb(maskCastleMoves);
            moves.addMove(encodeMove(sq, en, noPiece));
        }
    }
}

void position::genAllMoves(bool noisy, moveList &moves){
    // Init
    bitboard_t pinHV = 0;
    bitboard_t pinDA = 0;
    bitboard_t attacked = 0;
    bitboard_t okSq = ~colorBB[turn];

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