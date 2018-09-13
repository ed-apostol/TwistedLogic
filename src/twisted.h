/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/*********************************************/
/* MACROS                                    */
/*********************************************/

/* this is used on debugging, stolen from Fruit :-) */
// #define DEBUG
#ifdef DEBUG
#  define ASSERT(a) { if (!(a)) \
Print(4, "file \"%s\", line %d, assertion \"" #a "\" failed\n",__FILE__,__LINE__); }
#else
#  define ASSERT(a)
#endif

#define TRUE  1
#define FALSE  0

#define MAXPLY 64
#define MAXDATA 1024
#define MAXMOVES 256
#define INF 10000
#define MAXHIST 32767

#define WHITE  0
#define BLACK  1

#define EMPTY  0
#define PAWN  1
#define KNIGHT  2
#define BISHOP  3
#define ROOK  4
#define QUEEN  5
#define KING  6

#define CASTLE      14
#define TWOFORWARD  17
#define PROMOTE     33
#define ENPASSANT    9

#define WCKS 1
#define WCQS 2
#define BCKS 4
#define BCQS 8

#define STARTPOS  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define SQRANK(x)       ((x) >> 3)
#define SQFILE(x)       ((x) & 7)
#define MAX(a, b)       ((a)>(b)?(a):(b))
#define MIN(a, b)       ((a)<(b)?(a):(b))
#define ABS(a)          ((a)>0?(a):(-a))
#define DISTANCE(a, b)  (MAX((ABS(SQRANK(a)-SQRANK(b))),(ABS(SQFILE(a)-SQFILE(b)))))
#define PAWN_RANK(f,c)  ((c==BLACK)?(7-SQRANK(f)):SQRANK(f))

#define PST(c,p,s,l) (PcSqTb[c][p][s][l])
#define OPENING 0
#define ENDGAME 1

#define ABORTED             0
#define THINKING            1
#define PONDERING           2
#define ANALYSING           3

/*********************************************/
/* DATA STRUCTURES                           */
/*********************************************/

/* some basic definitions */
typedef unsigned long long u64;
typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef signed long long s64;
typedef signed long s32;
typedef signed short s16;
typedef signed char s8;
/* the move structure */
typedef struct {
	u32 m;
	int s;
}move_t;
/* the move list structure */
typedef struct {
	move_t list[MAXMOVES];
	u32 size;
	u32 pos;
}movelist_t;
/* the hash type structure */
typedef union {
	u64 b;
	u32 lock;
	u32 key;
}bit64_t;
/* the undo structure */
typedef struct {
	u32 lastmove;
	u32 castle;
	u32 fifty;
	u32 incheck;
	u32 phase;
	int epsq;
	int open[2];
	int end[2];
	bit64_t hash;
	bit64_t phash;
	bit64_t mathash;
}undo_t;
/* the position structure */
typedef struct {
	u64 pawns;
	u64 knights;
	u64 bishops;
	u64 rooks;
	u64 queens;
	u64 kings;
	u64 color[2];
	u64 occupied;
	u32 side;
	u32 ply;
	undo_t *status;
	undo_t undos[MAXDATA];
}position_t;

typedef struct {
	u32 thinking_status;
	u32 depth_is_limited;
	u32 depth_limit;
	u32 moves_is_limited;
	u64 time_is_limited;
	u64 time_limit_max;
	u64 time_limit_abs;
	u64 node_is_limited;
	u64 node_limit;

	u64 nodes_since_poll;
	u64 nodes_between_polls;
	u64 nodes;

	u64 start_time;
	u64 last_time;

	int last_value;
	int best_value;
	int currmovenumber;
	int change;
	int easy;
	int iteration;
	int maxplysearched;
	int legalmoves;
	int mate_found;

	int history[2][8][64];
	int killer1[MAXPLY];
	int killer2[MAXPLY];
}search_info_t;

/*********************************************/
/* CONSTANTS                                 */
/*********************************************/

/* the squares */
enum {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

/* used in routines popFirstBit and getFirstBit */
const int FoldedTable[64] = {
	63,30, 3,32,59,14,11,33,
	60,24,50, 9,55,19,21,34,
	61,29, 2,53,51,23,41,18,
	56,28, 1,43,46,27, 0,35,
	62,31,58, 4, 5,49,54, 6,
	15,52,12,40, 7,42,45,16,
	25,57,48,13,10,39, 8,44,
	20,47,38,22,17,37,36,26,
};

/* used in updating the castle status of the position */
static const int CastleMask[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	7, 15, 15, 15,  3, 15, 15, 11
};

/* the following constants are used by the Kindergarten bitboard attacks */
const u64 DiagonalMask[64] = {
0x8040201008040201ULL, 0x0080402010080402ULL, 0x0000804020100804ULL, 0x0000008040201008ULL,
0x0000000080402010ULL, 0x0000000000804020ULL, 0x0000000000008040ULL, 0x0000000000000080ULL,
0x4020100804020100ULL, 0x8040201008040201ULL, 0x0080402010080402ULL, 0x0000804020100804ULL,
0x0000008040201008ULL, 0x0000000080402010ULL, 0x0000000000804020ULL, 0x0000000000008040ULL,
0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL, 0x0080402010080402ULL,
0x0000804020100804ULL, 0x0000008040201008ULL, 0x0000000080402010ULL, 0x0000000000804020ULL,
0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
0x0080402010080402ULL, 0x0000804020100804ULL, 0x0000008040201008ULL, 0x0000000080402010ULL,
0x0804020100000000ULL, 0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL,
0x8040201008040201ULL, 0x0080402010080402ULL, 0x0000804020100804ULL, 0x0000008040201008ULL,
0x0402010000000000ULL, 0x0804020100000000ULL, 0x1008040201000000ULL, 0x2010080402010000ULL,
0x4020100804020100ULL, 0x8040201008040201ULL, 0x0080402010080402ULL, 0x0000804020100804ULL,
0x0201000000000000ULL, 0x0402010000000000ULL, 0x0804020100000000ULL, 0x1008040201000000ULL,
0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL, 0x0080402010080402ULL,
0x0100000000000000ULL, 0x0201000000000000ULL, 0x0402010000000000ULL, 0x0804020100000000ULL,
0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL
};

const u64 AntiDiagMask[64] = {
0x0000000000000001ULL, 0x0000000000000102ULL, 0x0000000000010204ULL, 0x0000000001020408ULL,
0x0000000102040810ULL, 0x0000010204081020ULL, 0x0001020408102040ULL, 0x0102040810204080ULL,
0x0000000000000102ULL, 0x0000000000010204ULL, 0x0000000001020408ULL, 0x0000000102040810ULL,
0x0000010204081020ULL, 0x0001020408102040ULL, 0x0102040810204080ULL, 0x0204081020408000ULL,
0x0000000000010204ULL, 0x0000000001020408ULL, 0x0000000102040810ULL, 0x0000010204081020ULL,
0x0001020408102040ULL, 0x0102040810204080ULL, 0x0204081020408000ULL, 0x0408102040800000ULL,
0x0000000001020408ULL, 0x0000000102040810ULL, 0x0000010204081020ULL, 0x0001020408102040ULL,
0x0102040810204080ULL, 0x0204081020408000ULL, 0x0408102040800000ULL, 0x0810204080000000ULL,
0x0000000102040810ULL, 0x0000010204081020ULL, 0x0001020408102040ULL, 0x0102040810204080ULL,
0x0204081020408000ULL, 0x0408102040800000ULL, 0x0810204080000000ULL, 0x1020408000000000ULL,
0x0000010204081020ULL, 0x0001020408102040ULL, 0x0102040810204080ULL, 0x0204081020408000ULL,
0x0408102040800000ULL, 0x0810204080000000ULL, 0x1020408000000000ULL, 0x2040800000000000ULL,
0x0001020408102040ULL, 0x0102040810204080ULL, 0x0204081020408000ULL, 0x0408102040800000ULL,
0x0810204080000000ULL, 0x1020408000000000ULL, 0x2040800000000000ULL, 0x4080000000000000ULL,
0x0102040810204080ULL, 0x0204081020408000ULL, 0x0408102040800000ULL, 0x0810204080000000ULL,
0x1020408000000000ULL, 0x2040800000000000ULL, 0x4080000000000000ULL, 0x8000000000000000ULL,
};

const u8 FirstRankAttacks[64][8] = {
{0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f}, {0x2,0xfd,0xfa,0xf6,0xee,0xde,0xbe,0x7e},
{0x6,0x5,0xfb,0xf4,0xec,0xdc,0xbc,0x7c}, {0x2,0x5,0xfa,0xf4,0xec,0xdc,0xbc,0x7c},
{0xe,0xd,0xb,0xf7,0xe8,0xd8,0xb8,0x78}, {0x2,0xd,0xa,0xf6,0xe8,0xd8,0xb8,0x78},
{0x6,0x5,0xb,0xf4,0xe8,0xd8,0xb8,0x78}, {0x2,0x5,0xa,0xf4,0xe8,0xd8,0xb8,0x78},
{0x1e,0x1d,0x1b,0x17,0xef,0xd0,0xb0,0x70}, {0x2,0x1d,0x1a,0x16,0xee,0xd0,0xb0,0x70},
{0x6,0x5,0x1b,0x14,0xec,0xd0,0xb0,0x70}, {0x2,0x5,0x1a,0x14,0xec,0xd0,0xb0,0x70},
{0xe,0xd,0xb,0x17,0xe8,0xd0,0xb0,0x70}, {0x2,0xd,0xa,0x16,0xe8,0xd0,0xb0,0x70},
{0x6,0x5,0xb,0x14,0xe8,0xd0,0xb0,0x70}, {0x2,0x5,0xa,0x14,0xe8,0xd0,0xb0,0x70},
{0x3e,0x3d,0x3b,0x37,0x2f,0xdf,0xa0,0x60}, {0x2,0x3d,0x3a,0x36,0x2e,0xde,0xa0,0x60},
{0x6,0x5,0x3b,0x34,0x2c,0xdc,0xa0,0x60}, {0x2,0x5,0x3a,0x34,0x2c,0xdc,0xa0,0x60},
{0xe,0xd,0xb,0x37,0x28,0xd8,0xa0,0x60}, {0x2,0xd,0xa,0x36,0x28,0xd8,0xa0,0x60},
{0x6,0x5,0xb,0x34,0x28,0xd8,0xa0,0x60}, {0x2,0x5,0xa,0x34,0x28,0xd8,0xa0,0x60},
{0x1e,0x1d,0x1b,0x17,0x2f,0xd0,0xa0,0x60}, {0x2,0x1d,0x1a,0x16,0x2e,0xd0,0xa0,0x60},
{0x6,0x5,0x1b,0x14,0x2c,0xd0,0xa0,0x60}, {0x2,0x5,0x1a,0x14,0x2c,0xd0,0xa0,0x60},
{0xe,0xd,0xb,0x17,0x28,0xd0,0xa0,0x60}, {0x2,0xd,0xa,0x16,0x28,0xd0,0xa0,0x60},
{0x6,0x5,0xb,0x14,0x28,0xd0,0xa0,0x60}, {0x2,0x5,0xa,0x14,0x28,0xd0,0xa0,0x60},
{0x7e,0x7d,0x7b,0x77,0x6f,0x5f,0xbf,0x40}, {0x2,0x7d,0x7a,0x76,0x6e,0x5e,0xbe,0x40},
{0x6,0x5,0x7b,0x74,0x6c,0x5c,0xbc,0x40}, {0x2,0x5,0x7a,0x74,0x6c,0x5c,0xbc,0x40},
{0xe,0xd,0xb,0x77,0x68,0x58,0xb8,0x40}, {0x2,0xd,0xa,0x76,0x68,0x58,0xb8,0x40},
{0x6,0x5,0xb,0x74,0x68,0x58,0xb8,0x40}, {0x2,0x5,0xa,0x74,0x68,0x58,0xb8,0x40},
{0x1e,0x1d,0x1b,0x17,0x6f,0x50,0xb0,0x40}, {0x2,0x1d,0x1a,0x16,0x6e,0x50,0xb0,0x40},
{0x6,0x5,0x1b,0x14,0x6c,0x50,0xb0,0x40}, {0x2,0x5,0x1a,0x14,0x6c,0x50,0xb0,0x40},
{0xe,0xd,0xb,0x17,0x68,0x50,0xb0,0x40}, {0x2,0xd,0xa,0x16,0x68,0x50,0xb0,0x40},
{0x6,0x5,0xb,0x14,0x68,0x50,0xb0,0x40}, {0x2,0x5,0xa,0x14,0x68,0x50,0xb0,0x40},
{0x3e,0x3d,0x3b,0x37,0x2f,0x5f,0xa0,0x40}, {0x2,0x3d,0x3a,0x36,0x2e,0x5e,0xa0,0x40},
{0x6,0x5,0x3b,0x34,0x2c,0x5c,0xa0,0x40}, {0x2,0x5,0x3a,0x34,0x2c,0x5c,0xa0,0x40},
{0xe,0xd,0xb,0x37,0x28,0x58,0xa0,0x40}, {0x2,0xd,0xa,0x36,0x28,0x58,0xa0,0x40},
{0x6,0x5,0xb,0x34,0x28,0x58,0xa0,0x40}, {0x2,0x5,0xa,0x34,0x28,0x58,0xa0,0x40},
{0x1e,0x1d,0x1b,0x17,0x2f,0x50,0xa0,0x40}, {0x2,0x1d,0x1a,0x16,0x2e,0x50,0xa0,0x40},
{0x6,0x5,0x1b,0x14,0x2c,0x50,0xa0,0x40}, {0x2,0x5,0x1a,0x14,0x2c,0x50,0xa0,0x40},
{0xe,0xd,0xb,0x17,0x28,0x50,0xa0,0x40}, {0x2,0xd,0xa,0x16,0x28,0x50,0xa0,0x40},
{0x6,0x5,0xb,0x14,0x28,0x50,0xa0,0x40}, {0x2,0x5,0xa,0x14,0x28,0x50,0xa0,0x40}
};

/* the bit masks of Squares */
const u64 A1 = 0x0000000000000001ULL;
const u64 B1 = 0x0000000000000002ULL;
const u64 C1 = 0x0000000000000004ULL;
const u64 D1 = 0x0000000000000008ULL;
const u64 E1 = 0x0000000000000010ULL;
const u64 F1 = 0x0000000000000020ULL;
const u64 G1 = 0x0000000000000040ULL;
const u64 H1 = 0x0000000000000080ULL;
const u64 A2 = 0x0000000000000100ULL;
const u64 B2 = 0x0000000000000200ULL;
const u64 C2 = 0x0000000000000400ULL;
const u64 D2 = 0x0000000000000800ULL;
const u64 E2 = 0x0000000000001000ULL;
const u64 F2 = 0x0000000000002000ULL;
const u64 G2 = 0x0000000000004000ULL;
const u64 H2 = 0x0000000000008000ULL;
const u64 A3 = 0x0000000000010000ULL;
const u64 B3 = 0x0000000000020000ULL;
const u64 C3 = 0x0000000000040000ULL;
const u64 D3 = 0x0000000000080000ULL;
const u64 E3 = 0x0000000000100000ULL;
const u64 F3 = 0x0000000000200000ULL;
const u64 G3 = 0x0000000000400000ULL;
const u64 H3 = 0x0000000000800000ULL;
const u64 A4 = 0x0000000001000000ULL;
const u64 B4 = 0x0000000002000000ULL;
const u64 C4 = 0x0000000004000000ULL;
const u64 D4 = 0x0000000008000000ULL;
const u64 E4 = 0x0000000010000000ULL;
const u64 F4 = 0x0000000020000000ULL;
const u64 G4 = 0x0000000040000000ULL;
const u64 H4 = 0x0000000080000000ULL;
const u64 A5 = 0x0000000100000000ULL;
const u64 B5 = 0x0000000200000000ULL;
const u64 C5 = 0x0000000400000000ULL;
const u64 D5 = 0x0000000800000000ULL;
const u64 E5 = 0x0000001000000000ULL;
const u64 F5 = 0x0000002000000000ULL;
const u64 G5 = 0x0000004000000000ULL;
const u64 H5 = 0x0000008000000000ULL;
const u64 A6 = 0x0000010000000000ULL;
const u64 B6 = 0x0000020000000000ULL;
const u64 C6 = 0x0000040000000000ULL;
const u64 D6 = 0x0000080000000000ULL;
const u64 E6 = 0x0000100000000000ULL;
const u64 F6 = 0x0000200000000000ULL;
const u64 G6 = 0x0000400000000000ULL;
const u64 H6 = 0x0000800000000000ULL;
const u64 A7 = 0x0001000000000000ULL;
const u64 B7 = 0x0002000000000000ULL;
const u64 C7 = 0x0004000000000000ULL;
const u64 D7 = 0x0008000000000000ULL;
const u64 E7 = 0x0010000000000000ULL;
const u64 F7 = 0x0020000000000000ULL;
const u64 G7 = 0x0040000000000000ULL;
const u64 H7 = 0x0080000000000000ULL;
const u64 A8 = 0x0100000000000000ULL;
const u64 B8 = 0x0200000000000000ULL;
const u64 C8 = 0x0400000000000000ULL;
const u64 D8 = 0x0800000000000000ULL;
const u64 E8 = 0x1000000000000000ULL;
const u64 F8 = 0x2000000000000000ULL;
const u64 G8 = 0x4000000000000000ULL;
const u64 H8 = 0x8000000000000000ULL;

/* the bit mask on a given square */
const u64 BitMask[64] = {
0x0000000000000001ULL, 0x0000000000000002ULL, 0x0000000000000004ULL, 0x0000000000000008ULL,
0x0000000000000010ULL, 0x0000000000000020ULL, 0x0000000000000040ULL, 0x0000000000000080ULL,
0x0000000000000100ULL, 0x0000000000000200ULL, 0x0000000000000400ULL, 0x0000000000000800ULL,
0x0000000000001000ULL, 0x0000000000002000ULL, 0x0000000000004000ULL, 0x0000000000008000ULL,
0x0000000000010000ULL, 0x0000000000020000ULL, 0x0000000000040000ULL, 0x0000000000080000ULL,
0x0000000000100000ULL, 0x0000000000200000ULL, 0x0000000000400000ULL, 0x0000000000800000ULL,
0x0000000001000000ULL, 0x0000000002000000ULL, 0x0000000004000000ULL, 0x0000000008000000ULL,
0x0000000010000000ULL, 0x0000000020000000ULL, 0x0000000040000000ULL, 0x0000000080000000ULL,
0x0000000100000000ULL, 0x0000000200000000ULL, 0x0000000400000000ULL, 0x0000000800000000ULL,
0x0000001000000000ULL, 0x0000002000000000ULL, 0x0000004000000000ULL, 0x0000008000000000ULL,
0x0000010000000000ULL, 0x0000020000000000ULL, 0x0000040000000000ULL, 0x0000080000000000ULL,
0x0000100000000000ULL, 0x0000200000000000ULL, 0x0000400000000000ULL, 0x0000800000000000ULL,
0x0001000000000000ULL, 0x0002000000000000ULL, 0x0004000000000000ULL, 0x0008000000000000ULL,
0x0010000000000000ULL, 0x0020000000000000ULL, 0x0040000000000000ULL, 0x0080000000000000ULL,
0x0100000000000000ULL, 0x0200000000000000ULL, 0x0400000000000000ULL, 0x0800000000000000ULL,
0x1000000000000000ULL, 0x2000000000000000ULL, 0x4000000000000000ULL, 0x8000000000000000ULL
};

/* the bit masks of Files */
const u64 FileABB = 0x0101010101010101ULL;
const u64 FileBBB = 0x0202020202020202ULL;
const u64 FileCBB = 0x0404040404040404ULL;
const u64 FileDBB = 0x0808080808080808ULL;
const u64 FileEBB = 0x1010101010101010ULL;
const u64 FileFBB = 0x2020202020202020ULL;
const u64 FileGBB = 0x4040404040404040ULL;
const u64 FileHBB = 0x8080808080808080ULL;

const u64 FileBB[8] = {
	0x0101010101010101ULL, 0x0202020202020202ULL, 0x0404040404040404ULL,
	0x0808080808080808ULL, 0x1010101010101010ULL, 0x2020202020202020ULL,
	0x4040404040404040ULL, 0x8080808080808080ULL
};

/* the bit masks of Ranks */
const u64 Rank1BB = 0xFFULL;
const u64 Rank2BB = 0xFF00ULL;
const u64 Rank3BB = 0xFF0000ULL;
const u64 Rank4BB = 0xFF000000ULL;
const u64 Rank5BB = 0xFF00000000ULL;
const u64 Rank6BB = 0xFF0000000000ULL;
const u64 Rank7BB = 0xFF000000000000ULL;
const u64 Rank8BB = 0xFF00000000000000ULL;

const u64 RankBB[8] = {
	0xFFULL, 0xFF00ULL, 0xFF0000ULL, 0xFF000000ULL, 0xFF00000000ULL,
	0xFF0000000000ULL, 0xFF000000000000ULL, 0xFF00000000000000ULL
};

const u64 EmptyBoardBB = 0ULL;
const u64 FullBoardBB = 0xFFFFFFFFFFFFFFFFULL;

const u64 WhiteSquaresBB = 0x55AA55AA55AA55AAULL;
const u64 BlackSquaresBB = 0xAA55AA55AA55AA55ULL;

const int FileA = 0;
const int FileB = 1;
const int FileC = 2;
const int FileD = 3;
const int FileE = 4;
const int FileF = 5;
const int FileG = 6;
const int FileH = 7;

const int Rank1 = 0;
const int Rank2 = 1;
const int Rank3 = 2;
const int Rank4 = 3;
const int Rank5 = 4;
const int Rank6 = 5;
const int Rank7 = 6;
const int Rank8 = 7;

/* init, mixed, castle, en passant, promotion, no-nothing,
	from the CRBMG of Sune Fischer */
char FenString[6][256] = {
	"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
	"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
	"r3k2r/3q4/2n1b3/7n/1bB5/2N2N2/1B2Q3/R3K2R w KQkq - 0 1",
	"rnbq1bnr/1pppkp1p/4p3/2P1P3/p5p1/8/PP1PKPPP/RNBQ1BNR w - - 0 1",
	"rn1q1bnr/1bP1kp1P/1p2p3/p7/8/8/PP1pKPpP/RNBQ1BNR w - - 0 1",
	"r6r/3qk3/2n1b3/7n/1bB5/2N2N2/1B2QK2/R6R w - - 0 1"
};

/* this is the value of pieces(middlegame and endgame) */
const int PcVal[] = { 0, 80, 310, 325, 500, 975, 0 };
const int EndPcVal[] = { 0, 100, 310, 325, 500, 975, 0 };

/*********************************************/
/* PRE-COMPUTED ARRAYS                       */
/*********************************************/

/* the precomputed static piece 64 bit attacks */
u64 KnightMoves[64];
u64 KingMoves[64];
u64 PawnCaps[64][2];
u64 PawnMoves[64][2];
u64 PawnMoves2[64][2];

/* used by the Kindergarten bitboard attacks generation */
u64 FillUpAttacks[64][8];
u64 AFileAttacks[64][8];

/* the rank and file mask indexed by square */
u64 RankMask[64];
u64 FileMask[64];

/*the 64 bit attacks on 8 different ray directions */
u64 DirA[8][64];

/* the 64 bit attacks between two squares of a valid direction */
u64 InBetween[64][64];

/* the bit mask on rank 2 and 7 according to colors */
u64 PromoteMask[2];

/* contains the delta of possible piece moves between two squares,
zero otherwise */
int Direction[64][64];

/* to be used in the move evasion generator */
int RunTo[64][64];

/* used for pre-computed piece-square table */
int PcSqTb[2][8][64][2];

/* this are used in computing hash values, inventor is someone named Zobrist */
bit64_t Zobrist[2][8][64];
bit64_t ZobMat[2][8][8];
bit64_t ZobEpsq[64];
bit64_t ZobCastle[16];
bit64_t ZobColor;

/* used in debugging, etc.. */
FILE *logfile;
FILE *errfile;
FILE *dumpfile;

/*********************************************/
/* PROTOTYPES                                */
/*********************************************/

/* utils.c */
extern void Print(int, char*, ...);
extern void displayBit(u64, int);
extern void displayBoard(const position_t*, int);
extern void displayBitPieces(const position_t*, int);
extern int getColor(const position_t*, int);
extern int getTime(void);
extern int parseMove(movelist_t*, char*);
extern char *move2Str(int);
extern u32 getPiece(const position_t*, int);
extern int bioskey(void);
extern int reps(const position_t*, int);
extern int getDirIndex(int);

/* movegen.c */
/* utilities for move generation */
extern u32 GenOneForward(u32, u32);
extern u32 GenTwoForward(u32, u32);
extern u32 GenPromote(u32, u32, u32, u32);
extern u32 GenPromoteStraight(u32, u32, u32);
extern u32 GenEnPassant(u32, u32);
extern u32 GenPawnMove(u32, u32, u32);
extern u32 GenKnightMove(u32, u32, u32);
extern u32 GenBishopMove(u32, u32, u32);
extern u32 GenRookMove(u32, u32, u32);
extern u32 GenQueenMove(u32, u32, u32);
extern u32 GenKingMove(u32, u32, u32);
extern u32 GenWhiteOO();
extern u32 GenWhiteOOO();
extern u32 GenBlackOO();
extern u32 GenBlackOOO();

extern u32 moveFrom(u32);
extern u32 moveTo(u32);
extern u32 movePiece(u32);
extern u32 moveAction(u32);
extern u32 moveCapture(u32);
extern u32 moveRemoval(u32);
extern u32 movePromote(u32);

extern u32 isCastle(u32);
extern u32 isPawn2Forward(u32);
extern u32 isPromote(u32);
extern u32 isEnPassant(u32);
extern void genMoves(const position_t*, movelist_t*);
extern void genQmoves(const position_t*, movelist_t*);
extern void genEvasions(const position_t*, movelist_t*);

/* init.c */
extern void initArr(void);

/* bitutils.c */
extern u32 rand32(void);
extern int bitCnt(u64);
extern __inline int getFirstBit(u64);
extern __inline int popFirstBit(u64*);
extern u64 fillUp(u64);
extern u64 fillDown(u64);
extern u64 fillUp2(u64);
extern u64 fillDown2(u64);

/* attacks.c */
extern u64 rankAttacks(u64, u32);
extern u64 fileAttacks(u64, u32);
extern u64 diagonalAttacks(u64, u32);
extern u64 antiDiagAttacks(u64, u32);
extern u64 bishopAttacksBB(int, u64);
extern u64 rookAttacksBB(int, u64);
extern u64 queenAttacksBB(int, u64);
extern int isAtt(const position_t*, int, u64);
extern u64 pinnedPieces(const position_t*, int);
extern int moveIsLegal(const position_t*, int, u64);
extern u64 attackingPiecesAll(const position_t*, int);
extern u64 attackingPiecesSide(const position_t*, int, int);
extern u64 behindFigure(u64, u64, u64, int, int);
extern int swap(const position_t*, int);

/* position.c */
extern void updateBoards(position_t*, int, int, int);
extern void unmakeMove(position_t*);
extern void makeMove(position_t*, int);
extern void setPosition(position_t*, char*);

/* tests.c */
extern void perft(position_t*, u32, u64[]);
extern int perftDivide(position_t*, u32, u32);
extern void runPerft(int);
extern void runPerftDivide(position_t*, u32);
extern void nonUCI(void);

/* eval.c */
extern int eval(const position_t*);

/* search.c */
extern void ponderHit(search_info_t*);
extern void check4Input(search_info_t*);
extern void initNode(search_info_t*);
extern move_t *getNextMove(movelist_t*);
extern int moveIsTactical(int);
extern int captureIsGood(const position_t*, int);
extern void qpreScore(movelist_t*);
extern void preScore(const position_t *pos, movelist_t*, search_info_t*);
extern void updatePV(int, int, int*, int*);
extern void displayPV(search_info_t*, int*, int, int);
extern int qSearch(position_t*, search_info_t*, int, int, int[]);
extern int search(position_t*, search_info_t*, int, int, int, int[]);
extern void getBestMove(position_t*, int, int, int, int, int,
	int, int, int, int, int, int, int[], int[]);

/* uci.c */
extern void uciParseSearchmoves(movelist_t*, char*, int[]);
extern void uciGo(position_t*, char*);
extern void uciStart(position_t*);
extern void uciSetPosition(position_t*, char*);

/* main.c */
extern void quit(void);
extern int main(int, char*[]);
