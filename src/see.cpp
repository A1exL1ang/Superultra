#include "board.h"
#include "movepick.h"
#include "helpers.h"
#include "types.h"
#include "attacks.h"

bool Position::seeGreater(Move move, Score threshold){
    // SEE basically calculates the result of a qsearch if we only consider the
    // destination square. We return whether the result is >= capture. We 
    // assume that (initial piece color == colorToMove) and that the move is legal.

    // You can think of this as "ping pong" across the threshold. After the current player 
    // captures, their score must be greater/equal to the threshold (relative to them)

    Square st = moveFrom(move);
    Square target = moveTo(move);
    Piece pieceTypeMoved = movePieceType(move);
    Piece pieceTypeCaptured = moveCaptType(move);

    // Deal with special moves:
    // 1) Always return true for promotion
    // 2) Assume en passant is worth a free pawn (score = PAWN)
    // 3) Castling has a score of 0

    if (movePromo(move) != NO_PIECE){
        return true;
    }
    if (isEP(move)){
        return threshold <= PIECE_SCORE[PAWN];
    }
    if (pieceTypeMoved == KING and abs(getFile(st) - getFile(target)) == 2){
        return threshold <= 0;
    }
    // Perform the initial capture (note that capt == NO_PIECE may be true)
    Score score = PIECE_SCORE[pieceTypeCaptured];

    // Cur player is not above the threshold after initially capturing
    if (score < threshold){
        return false;
    }

    // pieceTypeToBeCaptured is the piece that is at the square waiting to be captured
    // col is the color that is going to capture pieceTypeToBeCaptured

    Piece pieceTypeToBeCaptured = pieceTypeMoved;
    Color col = !turn;
    Bitboard occupancy = (allBB ^ (1ULL << st)) | (1ULL << target);

    // Initialize attackers with non-sliders
    Bitboard attackers = (pawnAttack(target, BLACK) & pieceBB[PAWN][WHITE])
                         | (pawnAttack(target, WHITE) & pieceBB[PAWN][BLACK])
                         | (knightAttack(target) & allPiece(KNIGHT))
                         | (kingAttack(target) & allPiece(KING));
                         
    while (true){
        // Find new slider attacks
        attackers |= (bishopAttack(target, occupancy) & (allPiece(BISHOP) | allPiece(QUEEN)))
                   | (rookAttack(target, occupancy) & (allPiece(ROOK) | allPiece(QUEEN)));

        // Remove the used pieces
        attackers &= occupancy;

        // The least valuable attacker
        Piece lva = 0;

        for (Piece pieceType : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}){
            if (Bitboard pieceAttacker = (attackers & pieceBB[pieceType][col]); pieceAttacker){
                lva = pieceType;
                occupancy ^= (1ULL << lsb(pieceAttacker));
                break;
            }
        }

        // We fail to be above/at the threshold because we can't capture so !col wins
        if (!lva){
            return (!col == turn); 
        }

        // We capture pieceTypeToBeCaptured and update score (make everything relative to us)
        score = -score + PIECE_SCORE[pieceTypeToBeCaptured];
        threshold = -threshold;
        
        // If the opponent's last capture was with the king and we "captured" it then we win
        // Must put this before next if statement.
        
        if (pieceTypeToBeCaptured == KING){
            return (col == turn);
        }
        // We fail to be above/at the threshold after capturing so !col wins
        if (score < threshold){
            return (!col == turn);
        }
        // Flip color and set the current piece at the square to be our least valuable attacker
        pieceTypeToBeCaptured = lva;
        col ^= 1;
    }
    return false;
}