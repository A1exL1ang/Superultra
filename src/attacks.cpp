#include <vector>
#include "types.h"
#include "helpers.h"
#include "attacks.h"

Bitboard lineBB[64][64];
Bitboard bishopMagicCache[64][512];
Bitboard rookMagicCache[64][4096];

void initLineBB(){
    for (Square sq = 0; sq < 64; sq++){
        for (int8 di : {-1, 0, 1}){
            for (int8 dj : {-1, 0, 1}){
                if (!di and !dj){
                    continue;
                }
                Rank i = getRank(sq);
                File j = getFile(sq);
                Bitboard pathMask = 0; 

                for (; isInGrid(i, j); i += di, j += dj){
                    int8 curSq = posToSquare(i, j);
                    pathMask |= (1ULL << curSq);
                    lineBB[sq][curSq] = pathMask; 
                }
            }
        }
    }
}

static void initSingleMagic(Square sq, bool isBishop){
    // Generate all non-end-of-path positions for blockers
    uint64 maskAllPos = isBishop ? bishopMagicMask[sq] : rookMagicMask[sq]; 
    std::vector<Square> pos; 

    for (Bitboard m = maskAllPos; m;){
        pos.push_back(poplsb(m));
    }

    // Init variables
    std::vector<uint64> blocker((1 << pos.size()), 0);
    std::vector<Bitboard> attack((1 << pos.size()), 0);

    // Generate all blocker masks and their corresponding attack masks
    for (uint64 blkMask = 0; blkMask < static_cast<uint64>(1 << pos.size()); blkMask++){
        for (uint64 m = blkMask; m;){
            blocker[blkMask] |= (1ULL << pos[poplsb(m)]);
        }
        for (int8 di : {-1, 0, 1}){
            for (int8 dj : {-1, 0, 1}){
                if ((!di and !dj)
                    or (isBishop and abs(di) != abs(dj))
                    or (!isBishop and abs(di) == abs(dj)))
                {
                    continue;
                }
                Rank i = getRank(sq);
                File j = getFile(sq);

                for (i += di, j += dj; isInGrid(i, j); i += di, j += dj){
                    Square curSq = posToSquare(i, j);
                    attack[blkMask] |= (1ULL << curSq);

                    // We stop if we hit blocker but our mask is still inclusive of it
                    if (blocker[blkMask] & (1ULL << curSq)){
                        break; 
                    }
                }
            }
        }
    }
    uint64 magic = isBishop ? bishopMagic[sq] : rookMagic[sq];
    Bitboard *cache = isBishop ? bishopMagicCache[sq] : rookMagicCache[sq];

    for (uint64 blkMask = 0; blkMask < static_cast<uint64>(1 << pos.size()); blkMask++){
        uint64 hashIdx = (blocker[blkMask] * magic) >> (64 - pos.size());
        cache[hashIdx] = attack[blkMask];
    }
}

void initMagicCache(){
    for (Square sq = 0; sq < 64; sq++){
        initSingleMagic(sq, true);
        initSingleMagic(sq, false);
    }
}