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

bool position::isLegal(Move move){
    // Init
    Square st = moveFrom(move);
    Square en = moveTo(move);
    Piece pieceType = movePieceType(move); 
    Color col = movePieceColor(move);
    Piece promo = movePromo(move);

    // Part 1) Our first step is to deal with all the special cases. This includes no piece / wrong color,
    // friendly piece at the destination, invalid promotion, enpassant, and king moves (including castling). 

    // We aren't moving any piece or wrong color
    if (pieceType == noPiece or col != turn){
        return false;
    }

    // Friendly piece at en
    if ((1ULL << en) & colorBB[turn]){
        return false;
    }

    // Invalid pawn promotion (not at end but promo or at end but no promo)
    if (pieceType == pawn and ((getRank(en) == (turn == white ? 7 : 0)) != (promo != noPiece))){
        return false;
    }

    // Non-pawn tries to promote
    if (pieceType != pawn and promo != noPiece){
        return false;
    }

    // En passant (pawn moves diagonally but doesn't capture anything)
    if (pieceType == pawn
        and ((1ULL << en) & pawnAttack(st, turn))
        and !((1ULL << en) & colorBB[!turn]))
    {
        // If we can't do en passant
        if (pos[stk].epFile == noEP)
            return false;

        Square epEnemyPawn = pos[stk].epFile + (turn == white ? 32 : 24);
        Square epDestination = pos[stk].epFile + (turn == white ? 40 : 16);

        // Move destination not equal to en passant destination
        if (en != epDestination)
            return false;

        Bitboard occupancyAfter = (allBB ^ (1ULL << st) ^ (1ULL << epDestination) ^ (1ULL << epEnemyPawn));
        
        // Good if no discovered attacks and not already in check. This is en passant so need to clear
        // out enemy pawn

        return !(pawnAttack(kingSq(turn), turn) & (pieceBB[pawn][!turn] & (~(1ULL << epEnemyPawn))))
                and !(knightAttack(kingSq(turn)) & pieceBB[knight][!turn])
                and !(bishopAttack(kingSq(turn), occupancyAfter) & pieceBB[bishop][!turn])
                and !(rookAttack(kingSq(turn), occupancyAfter) & pieceBB[rook][!turn])
                and !(queenAttack(kingSq(turn), occupancyAfter) & pieceBB[queen][!turn]);
    }

    // King move
    if (pieceType == king){

        // Note that we xray through our king
        Bitboard attacked = allAttack(!turn, allBB ^ pieceBB[king][turn]);
        Bitboard moves = (kingAttack(st) & (~attacked) & (~colorBB[turn]));

        if (turn == white){
            moves |= (1ULL << g1) * ((pos[stk].castleRights & castleWhiteK) and !(attacked & getLine(e1, g1)) and !(allBB & getLine(f1, g1)))
                   | (1ULL << c1) * ((pos[stk].castleRights & castleWhiteQ) and !(attacked & getLine(e1, c1)) and !(allBB & getLine(b1, d1)));
        }
        else{
            moves |= (1ULL << g8) * ((pos[stk].castleRights & castleBlackK) and !(attacked & getLine(e8, g8)) and !(allBB & getLine(f8, g8)))
                   | (1ULL << c8) * ((pos[stk].castleRights & castleBlackQ) and !(attacked & getLine(e8, c8)) and !(allBB & getLine(b8, d8)));
        }
        return static_cast<bool>((1ULL << en) & moves);
    }

    // Part 2) With the special cases out of the way, now we are left with "normal" moves. We first have
    // to check whether a piece of the given type can move from st to en (for example a rook cannot do a diagonal move).
    // Then we check whether moving the piece from st to en reveals any discovered attacks to the king

    // Pawn psuedolegalality
    if (pieceType == pawn){

        // Rank relative to stm
        Bitboard pawnPushEnd = (pawnsUp((1ULL << st), turn) & (~allBB));
        Bitboard pawnDoublePushEnd = (pawnsUp((pawnPushEnd & allInRank[turn == white ? 2 : 5]), turn) & (~allBB));
        Bitboard pawnAttackEnd = (pawnAttack(st, turn) & colorBB[!turn]);

        // Not psuedolegal
        if (!((1ULL << en) & (pawnPushEnd | pawnDoublePushEnd | pawnAttackEnd))){
            return false;
        }
    }

    // Knight psuedolegality
    if (pieceType == knight and !((1ULL << en) & knightAttack(st))){
        return false;
    }

    // Bishop psuedolegality
    if (pieceType == bishop and !((1ULL << en) & bishopAttack(st, allBB))){
        return false;
    }

    // Rook psuedolegality
    if (pieceType == rook and !((1ULL << en) & rookAttack(st, allBB))){
        return false;
    }

    // Queen psuedolegality
    if (pieceType == queen and !((1ULL << en) & queenAttack(st, allBB))){
        return false;
    }

    // Don't xor (1ULL << en) since that bit may be set if we are capturing a piece at en.
    // Also make sure to clear out the captured piece from pieceBB

    Bitboard occupancyAfter = (allBB ^ (1ULL << st)) | (1ULL << en);

    return !(pawnAttack(kingSq(turn), turn) & (pieceBB[pawn][!turn] & (~(1ULL << en))))
            and !(knightAttack(kingSq(turn)) & (pieceBB[knight][!turn] & (~(1ULL << en))))
            and !(bishopAttack(kingSq(turn), occupancyAfter) & (pieceBB[bishop][!turn] & (~(1ULL << en))))
            and !(rookAttack(kingSq(turn), occupancyAfter) & (pieceBB[rook][!turn] & (~(1ULL << en))))
            and !(queenAttack(kingSq(turn), occupancyAfter) & (pieceBB[queen][!turn] & (~(1ULL << en))));
}