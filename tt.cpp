#include <cstring>
#include <memory>
#include "types.h"
#include "helpers.h"
#include "tt.h"

ttStruct globalTT;

TTKey ttRngPiece[7][2][64]; 
TTKey ttRngTurn;
TTKey ttRngEnpass[16];
TTKey ttRngCastle[16];

static uint64 seed = 1928777382391231823ULL;

static inline uint64 genRand(){
    return seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
}

void initTT(){
    for (Piece pieceType : {pawn, knight, bishop, rook, queen, king}){
        for (Color col : {white, black}){
            for (Square sq = 0; sq < 64; sq++){
                ttRngPiece[pieceType][col][sq] = genRand();
            }
        }
    }

    ttRngTurn = genRand();
    
    for (File enpassFile = 0; enpassFile <= 7; enpassFile++){
        ttRngEnpass[enpassFile] = genRand();
    }   
    for (int8 castle = 0; castle < 16; castle++){
        ttRngCastle[castle] = genRand();
    }
}

void ttStruct::clearTT(){
    for (int i = 0; i < sz; i++){
        for (int j = 0; j < clusterSize; j++){
            table[i].dat[j] = ttEntry();
        }
    }
}

void ttStruct::setSize(uint64 megabytes){
    sz = 1024;
    while (2 * sz * sizeof(ttCluster) <= (megabytes << 20))
        sz *= 2;

    // (% tt size) is equivilant to (& maskMod)
    maskMod = sz - 1;

    table.reset(new ttCluster[sz]);

    clearTT();
    currentAge = 0;
}

bool ttStruct::probe(TTKey probehash, ttEntry &tte, Depth ply){
    ttEntry *bucket = &table[(probehash & maskMod)].dat[0];

    for (int i = 0; i < clusterSize; i++){
        if (bucket[i].zhash == probehash){
            // Update age of entry to be most recent
            bucket[i].ageAndBound = encodeAgeAndBound(currentAge, decodeBound(bucket[i].ageAndBound));
            
            // Init and adjust score
            tte = bucket[i];
            tte.score = scoreFromTT(tte.score, ply);

            // Found entry
            return true;
        }
    }
    // Failed to find entry
    return false;
}

void ttStruct::addToTT(TTKey zhash, Score score, Score staticEval, Move bestMove, Depth depth, Depth ply, TTboundAge bound, bool pvNode){
    ttEntry *bucket = &table[(zhash & maskMod)].dat[0];
    int replace = 0;
    
    // Find replacement entry by finding entry with same hash or the entry
    // of the worst quality if the entry of the same hash doesn't exist

    for (int i = 0; i < clusterSize; i++){
        // An entry with this hash already exists
        if (bucket[i].zhash == zhash){
            replace = i;
            break;
        }
        // See if curEntries[i] is worse
        if (quality(bucket[i], currentAge) < quality(bucket[replace], currentAge)){
            replace = i;
        }
    }

    // Replace if:
    // 1) Did not find entry of same hash and our entry is somewhat on par with worst entry in bucket
    // 2) Found entry of same hash and our entry is somewhat on par with the found entry
    
    ttEntry newEntry = ttEntry(zhash, scoreToTT(score, ply), staticEval, bestMove, depth, encodeAgeAndBound(currentAge, bound));

    if (bound == boundExact
        or (bucket[replace].zhash != zhash and quality(newEntry, currentAge) + 1 + 2 * pvNode >= quality(bucket[replace], currentAge))
        or (bucket[replace].zhash == zhash and depth + pvNode >= bucket[replace].depth))
    {
        bucket[replace] = newEntry;
    }
}

int ttStruct::hashFullness(){
    int counter = 0;
    for (int i = 0; i < 1000 / clusterSize; i++){
        for (int j = 0; j < clusterSize; j++){
            if (decodeAge(table[i].dat[j].ageAndBound) == currentAge){
                counter += (table[i].dat[j].zhash != noHash);
            }
        }
    }
    return counter;
}