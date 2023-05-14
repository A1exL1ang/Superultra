#include "types.h"
#include "helpers.h"

#pragma once

// Line lookup (too big to declare in compile time)
extern bitboard_t lineBB[64][64];

// All bits in certain ranks/files
const bitboard_t allInRank[8] = {255ULL, 65280ULL, 16711680ULL, 4278190080ULL, 1095216660480ULL, 280375465082880ULL, 71776119061217280ULL, 18374686479671623680ULL};
const bitboard_t allInFile[8] = {72340172838076673ULL, 144680345676153346ULL, 289360691352306692ULL, 578721382704613384ULL, 1157442765409226768ULL, 2314885530818453536ULL, 4629771061636907072ULL, 9259542123273814144ULL};

// Basic attack masks
const bitboard_t pawnAttackArray[64][2] = {{512ULL, 0ULL}, {1280ULL, 0ULL}, {2560ULL, 0ULL}, {5120ULL, 0ULL}, {10240ULL, 0ULL}, {20480ULL, 0ULL}, {40960ULL, 0ULL}, {16384ULL, 0ULL}, {131072ULL, 2ULL}, {327680ULL, 5ULL}, {655360ULL, 10ULL}, {1310720ULL, 20ULL}, {2621440ULL, 40ULL}, {5242880ULL, 80ULL}, {10485760ULL, 160ULL}, {4194304ULL, 64ULL}, {33554432ULL, 512ULL}, {83886080ULL, 1280ULL}, {167772160ULL, 2560ULL}, {335544320ULL, 5120ULL}, {671088640ULL, 10240ULL}, {1342177280ULL, 20480ULL}, {2684354560ULL, 40960ULL}, {1073741824ULL, 16384ULL}, {8589934592ULL, 131072ULL}, {21474836480ULL, 327680ULL}, {42949672960ULL, 655360ULL}, {85899345920ULL, 1310720ULL}, {171798691840ULL, 2621440ULL}, {343597383680ULL, 5242880ULL}, {687194767360ULL, 10485760ULL}, {274877906944ULL, 4194304ULL}, {2199023255552ULL, 33554432ULL}, {5497558138880ULL, 83886080ULL}, {10995116277760ULL, 167772160ULL}, {21990232555520ULL, 335544320ULL}, {43980465111040ULL, 671088640ULL}, {87960930222080ULL, 1342177280ULL}, {175921860444160ULL, 2684354560ULL}, {70368744177664ULL, 1073741824ULL}, {562949953421312ULL, 8589934592ULL}, {1407374883553280ULL, 21474836480ULL}, {2814749767106560ULL, 42949672960ULL}, {5629499534213120ULL, 85899345920ULL}, {11258999068426240ULL, 171798691840ULL}, {22517998136852480ULL, 343597383680ULL}, {45035996273704960ULL, 687194767360ULL}, {18014398509481984ULL, 274877906944ULL}, {144115188075855872ULL, 2199023255552ULL}, {360287970189639680ULL, 5497558138880ULL}, {720575940379279360ULL, 10995116277760ULL}, {1441151880758558720ULL, 21990232555520ULL}, {2882303761517117440ULL, 43980465111040ULL}, {5764607523034234880ULL, 87960930222080ULL}, {11529215046068469760ULL, 175921860444160ULL}, {4611686018427387904ULL, 70368744177664ULL}, {0ULL, 562949953421312ULL}, {0ULL, 1407374883553280ULL}, {0ULL, 2814749767106560ULL}, {0ULL, 5629499534213120ULL}, {0ULL, 11258999068426240ULL}, {0ULL, 22517998136852480ULL}, {0ULL, 45035996273704960ULL}, {0ULL, 18014398509481984ULL}};
const bitboard_t knightAttackArray[64] = {132096ULL, 329728ULL, 659712ULL, 1319424ULL, 2638848ULL, 5277696ULL, 10489856ULL, 4202496ULL, 33816580ULL, 84410376ULL, 168886289ULL, 337772578ULL, 675545156ULL, 1351090312ULL, 2685403152ULL, 1075839008ULL, 8657044482ULL, 21609056261ULL, 43234889994ULL, 86469779988ULL, 172939559976ULL, 345879119952ULL, 687463207072ULL, 275414786112ULL, 2216203387392ULL, 5531918402816ULL, 11068131838464ULL, 22136263676928ULL, 44272527353856ULL, 88545054707712ULL, 175990581010432ULL, 70506185244672ULL, 567348067172352ULL, 1416171111120896ULL, 2833441750646784ULL, 5666883501293568ULL, 11333767002587136ULL, 22667534005174272ULL, 45053588738670592ULL, 18049583422636032ULL, 145241105196122112ULL, 362539804446949376ULL, 725361088165576704ULL, 1450722176331153408ULL, 2901444352662306816ULL, 5802888705324613632ULL, 11533718717099671552ULL, 4620693356194824192ULL, 288234782788157440ULL, 576469569871282176ULL, 1224997833292120064ULL, 2449995666584240128ULL, 4899991333168480256ULL, 9799982666336960512ULL, 1152939783987658752ULL, 2305878468463689728ULL, 1128098930098176ULL, 2257297371824128ULL, 4796069720358912ULL, 9592139440717824ULL, 19184278881435648ULL, 38368557762871296ULL, 4679521487814656ULL, 9077567998918656ULL};
const bitboard_t kingAttackArray[64] = {770ULL, 1797ULL, 3594ULL, 7188ULL, 14376ULL, 28752ULL, 57504ULL, 49216ULL, 197123ULL, 460039ULL, 920078ULL, 1840156ULL, 3680312ULL, 7360624ULL, 14721248ULL, 12599488ULL, 50463488ULL, 117769984ULL, 235539968ULL, 471079936ULL, 942159872ULL, 1884319744ULL, 3768639488ULL, 3225468928ULL, 12918652928ULL, 30149115904ULL, 60298231808ULL, 120596463616ULL, 241192927232ULL, 482385854464ULL, 964771708928ULL, 825720045568ULL, 3307175149568ULL, 7718173671424ULL, 15436347342848ULL, 30872694685696ULL, 61745389371392ULL, 123490778742784ULL, 246981557485568ULL, 211384331665408ULL, 846636838289408ULL, 1975852459884544ULL, 3951704919769088ULL, 7903409839538176ULL, 15806819679076352ULL, 31613639358152704ULL, 63227278716305408ULL, 54114388906344448ULL, 216739030602088448ULL, 505818229730443264ULL, 1011636459460886528ULL, 2023272918921773056ULL, 4046545837843546112ULL, 8093091675687092224ULL, 16186183351374184448ULL, 13853283560024178688ULL, 144959613005987840ULL, 362258295026614272ULL, 724516590053228544ULL, 1449033180106457088ULL, 2898066360212914176ULL, 5796132720425828352ULL, 11592265440851656704ULL, 4665729213955833856ULL};

// Slider magic numbers
const uint64 bishopMagic[64] = {4521200542023712ULL, 2891319761176635776ULL, 1249758328351883282ULL, 9224568651267451456ULL, 3534218122427893508ULL, 2306142110803492864ULL, 431118349248512ULL, 18722632208760833ULL, 4684977437115715720ULL, 4508633398265346ULL, 2468003386722226176ULL, 9799872388798627856ULL, 9367489467172257796ULL, 2742710870273493248ULL, 4647720360385471500ULL, 9223526316386033664ULL, 13586940339224864ULL, 9804339696212902156ULL, 22518006768861704ULL, 140772083122176ULL, 144396682652550673ULL, 1688851068486664ULL, 562985408143360ULL, 2306126685365862914ULL, 153712430766147588ULL, 54360409198830852ULL, 1801589393387954720ULL, 72629344446464008ULL, 216317917724188673ULL, 18157337185427584ULL, 11260098589919249ULL, 72340168559845637ULL, 2289463037330432ULL, 5838078779793100800ULL, 901287419503968385ULL, 356379340833024ULL, 297239776577266816ULL, 1267745505214594ULL, 9224524326252872768ULL, 295269475362570320ULL, 848960952468496ULL, 4617316622441394180ULL, 2306408162523350025ULL, 9511602825608562704ULL, 72062026713770496ULL, 901847476896531200ULL, 9374262450242846792ULL, 4614083507860340808ULL, 288516257802625568ULL, 1689279424364544ULL, 9224503436629770241ULL, 37189883514847424ULL, 6345571948148686854ULL, 1147959165452416ULL, 40534630046367744ULL, 3748125224466513924ULL, 38844788166004736ULL, 4913432694379649024ULL, 36046802069491712ULL, 23708257357320ULL, 4629700417206485508ULL, 6759952660302080ULL, 1465364535686540290ULL, 4961030849462352ULL};
const uint64 rookMagic[64] = {36038417747837040ULL, 18014535966261312ULL, 684582329074614792ULL, 4683753508607102978ULL, 9259409634328838784ULL, 648527421608034322ULL, 1188952500665846912ULL, 72060484550950978ULL, 72198342276285600ULL, 288300747581293632ULL, 4611826824641511552ULL, 11673471006000812032ULL, 9223653584862976000ULL, 4647855561541681280ULL, 288511864015733248ULL, 2594354863572386048ULL, 3494797983768518656ULL, 581246927492907552ULL, 1155597166689980416ULL, 4899926290452250656ULL, 576743326925982993ULL, 3603022088752874496ULL, 1155186498741800968ULL, 4613940017272800261ULL, 140739637952512ULL, 4611756389321146496ULL, 2319388994616428672ULL, 54326908184166688ULL, 866947337053274240ULL, 564126775531524ULL, 83317709798379530ULL, 5767000635861967444ULL, 70918516777217ULL, 2323998420097835012ULL, 5476518021818224641ULL, 286010613174272ULL, 2891452798040416320ULL, 2671197546172416002ULL, 4611863108586047682ULL, 36029810664801921ULL, 10412335808035438592ULL, 10394325566560935938ULL, 288529443851370624ULL, 38483175473192ULL, 326511076415963152ULL, 1153485623933403144ULL, 9513856416279167120ULL, 5270341932881936419ULL, 4574004417396864ULL, 289426646969090176ULL, 44261790843392ULL, 474991171207296ULL, 422246858621056ULL, 2306407093307183616ULL, 2458976460531565568ULL, 1153066642307629568ULL, 75868449955985ULL, 141837289390369ULL, 40867748229693721ULL, 10982203681939654821ULL, 289638069802370097ULL, 39125073302521913ULL, 10718575986562039940ULL, 9224506872458642498ULL};

// Slider magic shifts
const int8 bishopMagicShift[64] = {6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};
const int8 rookMagicShift[64] = {12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

// Slider magic masks (attack on an empty board excluding border cells and itself)
const bitboard_t bishopMagicMask[64] = {18049651735527936ULL, 70506452091904ULL, 275415828992ULL, 1075975168ULL, 38021120ULL, 8657588224ULL, 2216338399232ULL, 567382630219776ULL, 9024825867763712ULL, 18049651735527424ULL, 70506452221952ULL, 275449643008ULL, 9733406720ULL, 2216342585344ULL, 567382630203392ULL, 1134765260406784ULL, 4512412933816832ULL, 9024825867633664ULL, 18049651768822272ULL, 70515108615168ULL, 2491752130560ULL, 567383701868544ULL, 1134765256220672ULL, 2269530512441344ULL, 2256206450263040ULL, 4512412900526080ULL, 9024834391117824ULL, 18051867805491712ULL, 637888545440768ULL, 1135039602493440ULL, 2269529440784384ULL, 4539058881568768ULL, 1128098963916800ULL, 2256197927833600ULL, 4514594912477184ULL, 9592139778506752ULL, 19184279556981248ULL, 2339762086609920ULL, 4538784537380864ULL, 9077569074761728ULL, 562958610993152ULL, 1125917221986304ULL, 2814792987328512ULL, 5629586008178688ULL, 11259172008099840ULL, 22518341868716544ULL, 9007336962655232ULL, 18014673925310464ULL, 2216338399232ULL, 4432676798464ULL, 11064376819712ULL, 22137335185408ULL, 44272556441600ULL, 87995357200384ULL, 35253226045952ULL, 70506452091904ULL, 567382630219776ULL, 1134765260406784ULL, 2832480465846272ULL, 5667157807464448ULL, 11333774449049600ULL, 22526811443298304ULL, 9024825867763712ULL, 18049651735527936ULL};
const bitboard_t rookMagicMask[64] = {282578800148862ULL, 565157600297596ULL, 1130315200595066ULL, 2260630401190006ULL, 4521260802379886ULL, 9042521604759646ULL, 18085043209519166ULL, 36170086419038334ULL, 282578800180736ULL, 565157600328704ULL, 1130315200625152ULL, 2260630401218048ULL, 4521260802403840ULL, 9042521604775424ULL, 18085043209518592ULL, 36170086419037696ULL, 282578808340736ULL, 565157608292864ULL, 1130315208328192ULL, 2260630408398848ULL, 4521260808540160ULL, 9042521608822784ULL, 18085043209388032ULL, 36170086418907136ULL, 282580897300736ULL, 565159647117824ULL, 1130317180306432ULL, 2260632246683648ULL, 4521262379438080ULL, 9042522644946944ULL, 18085043175964672ULL, 36170086385483776ULL, 283115671060736ULL, 565681586307584ULL, 1130822006735872ULL, 2261102847592448ULL, 4521664529305600ULL, 9042787892731904ULL, 18085034619584512ULL, 36170077829103616ULL, 420017753620736ULL, 699298018886144ULL, 1260057572672512ULL, 2381576680245248ULL, 4624614895390720ULL, 9110691325681664ULL, 18082844186263552ULL, 36167887395782656ULL, 35466950888980736ULL, 34905104758997504ULL, 34344362452452352ULL, 33222877839362048ULL, 30979908613181440ULL, 26493970160820224ULL, 17522093256097792ULL, 35607136465616896ULL, 9079539427579068672ULL, 8935706818303361536ULL, 8792156787827803136ULL, 8505056726876686336ULL, 7930856604974452736ULL, 6782456361169985536ULL, 4485655873561051136ULL, 9115426935197958144ULL};

// Slider magic lookup (too big to declare in compile time)
extern bitboard_t bishopMagicCache[64][512];
extern bitboard_t rookMagicCache[64][4096];

// We create an "interface" of inlined functions (to be consistent)
inline bitboard_t between(square_t sq1, square_t sq2){
    return lineBB[sq1][sq2];
}

inline bitboard_t knightAttack(square_t sq){
    return knightAttackArray[sq];
}

inline bitboard_t bishopAttack(square_t sq, bitboard_t occupancy){
    return bishopMagicCache[sq][((occupancy & bishopMagicMask[sq]) * bishopMagic[sq]) >> (64 - bishopMagicShift[sq])];
}

inline bitboard_t rookAttack(square_t sq, bitboard_t occupancy){
    return rookMagicCache[sq][((occupancy & rookMagicMask[sq]) * rookMagic[sq]) >> (64 - rookMagicShift[sq])];
}

inline bitboard_t queenAttack(square_t sq, bitboard_t occupancy){
    return bishopAttack(sq, occupancy) | rookAttack(sq, occupancy);
}

inline bitboard_t pawnAttack(square_t sq, color_t col){
    return pawnAttackArray[sq][col];
}

inline bitboard_t pawnsLeftAttack(bitboard_t pawnMask, color_t col){ 
    return col == white ? ((pawnMask & (~allInFile[fileA])) << 7) : ((pawnMask & (~allInFile[fileA])) >> 9);
}

inline bitboard_t pawnsRightAttack(bitboard_t pawnMask, color_t col){
    return col == white ? ((pawnMask & (~allInFile[fileH])) << 9) : ((pawnMask & (~allInFile[fileH])) >> 7);
}

inline bitboard_t pawnsAllAttack(bitboard_t pawnMask, color_t col){
    return (pawnsLeftAttack(pawnMask, col) | pawnsRightAttack(pawnMask, col));
}

inline bitboard_t pawnsUp(bitboard_t pawnMask, color_t col){
    return col == white ? (pawnMask << 8) : (pawnMask >> 8);
}

inline bitboard_t kingAttack(square_t sq){
    return kingAttackArray[sq];
}

// Initalize the big arrays during runtime
void initLineBB();
void initMagicCache();