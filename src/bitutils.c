/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/* this is a utility to generate 32 bit pseudo-random numbers */
static u32 rndseed = 3;
u32 rand32(void){
    u32 r = rndseed;
    r = 1664525UL * r + 1013904223UL;
    return (rndseed = r);
}
/* this count the number bits in a given bitfield,
it is using a SWAR algorithm I think */
int bitCnt(u64 x){
    x -= (x >> 1) & 0x5555555555555555ULL;
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
    return (x * 0x0101010101010101ULL) >> 56;
}
/* returns the least significant square from the 64 bitfield */
__inline int getFirstBit(u64 b) {
    b ^= (b - 1);
    u32 folded = ((int)b) ^ ((int)(b >> 32));
    return FoldedTable[(folded * 0x78291ACF) >> 26];
}
/* returns the least significant square
 and clears it from the 64 bitfield */
__inline int popFirstBit(u64 *b) {
    u64 bb = *b ^ (*b - 1);
    u32 folded = ((int)bb ^ (int)(bb >> 32));
    *b &= (*b - 1);
    return FoldedTable[(folded * 0x78291ACF) >> 26];
}

/* returns a bitboard with all bits above b filled up (discluding b) */
u64 fillUp(u64 b){
	b |= b << 8;
	b |= b << 16;
	return (b | (b << 32)) << 8;
}

/* returns a bitboard with all bits below b filled down (discluding b) */
u64 fillDown(u64 b){
	b |= b >> 8;
	b |= b >> 16;
	return (b | (b >> 32)) >> 8;
}

/* returns a bitboard with all bits above b filled up (including b) */
u64 fillUp2(u64 b){
	b |= b << 8;
	b |= b << 16;
	return (b | (b << 32));
}

/* returns a bitboard with all bits below b filled down (including b) */
u64 fillDown2(u64 b){
	b |= b >> 8;
	b |= b >> 16;
	return (b | (b >> 32));
}
