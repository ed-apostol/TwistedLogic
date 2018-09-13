/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/* this routines are invented by Gerd Isenberg to compute sliding attacks
the non-rotated way, he calls this the Kindergarten approach */
u64 rankAttacks(u64 occ, u32 sq) {
	u32 f = sq & 7;
	u32 r = sq & ~7;
	u32 o = (u32)(occ >> (r + 1)) & 63;
	return (u64)FirstRankAttacks[o][f] << r;
}

u64 fileAttacks(u64 occ, u32 sq) {
	u32 f = sq & 7;
	occ = 0x0101010101010101ULL & (occ >> f);
	u32 o = (0x0080402010080400ULL * occ) >> 58;
	return (AFileAttacks[o][sq >> 3]) << f;
}

u64 diagonalAttacks(u64 occ, u32 sq) {
	u32 f = sq & 7;
	occ = (DiagonalMask[sq] & occ);
	u32 o = (0x0202020202020202ULL * occ) >> 58;
	return (DiagonalMask[sq] & FillUpAttacks[o][f]);
}

u64 antiDiagAttacks(u64 occ, u32 sq) {
	u32 f = sq & 7;
	occ = (AntiDiagMask[sq] & occ);
	u32 o = (0x0202020202020202ULL * occ) >> 58;
	return (AntiDiagMask[sq] & FillUpAttacks[o][f]);
}

/* the following routines returns a 64 bit attacks of pieces
on the From square with the limiting Occupied bits */
u64 bishopAttacksBB(int from, u64 occ) {
	return (diagonalAttacks(occ, from) | antiDiagAttacks(occ, from));
}

u64 rookAttacksBB(int from, u64 occ) {
	return (fileAttacks(occ, from) | rankAttacks(occ, from));
}

u64 queenAttacksBB(int from, u64 occ) {
	return (diagonalAttacks(occ, from) | antiDiagAttacks(occ, from)
		| fileAttacks(occ, from) | rankAttacks(occ, from));
}

/* this is the attack routine, capable of multiple targets,
can be used to determine if a position is in check*/
int isAtt(const position_t *pos, int color, u64 target) {
	int from;
	while (target) {
		from = popFirstBit(&target);
		if (((pos->rooks | pos->queens) & pos->color[color])
			& rookAttacksBB(from, pos->occupied)) return TRUE;
		if (((pos->bishops | pos->queens) & pos->color[color])
			& bishopAttacksBB(from, pos->occupied)) return TRUE;
		if ((pos->knights & pos->color[color])
			& KnightMoves[from]) return TRUE;
		if (((pos->kings | pos->queens) & pos->color[color])
			& KingMoves[from]) return TRUE;
		if (((pos->pawns | pos->bishops) & pos->color[color])
			& PawnCaps[from][color ^ 1]) return TRUE;
	}
	return FALSE;
}

/* this returns the pinned pieces to the King of the side Color,
idea stolen from Glaurung */
u64 pinnedPieces(const position_t *pos, int color) {
	u64 b1, b2, pinned, pinners, sliders;
	int ksq = getFirstBit((pos->kings&pos->color[color])), temp;
	pinned = EmptyBoardBB;
	b1 = pos->occupied;
	sliders = (pos->queens | pos->rooks) & pos->color[color ^ 1];
	if (sliders) {
		b2 = rookAttacksBB(ksq, b1) & pos->color[color];
		pinners = rookAttacksBB(ksq, b1 ^ b2) & sliders;
		while (pinners) {
			temp = popFirstBit(&pinners);
			pinned |= (InBetween[temp][ksq] & b2);
		}
	}
	sliders = (pos->queens | pos->bishops) & pos->color[color ^ 1];
	if (sliders) {
		b2 = bishopAttacksBB(ksq, b1) & pos->color[color];
		pinners = bishopAttacksBB(ksq, b1 ^ b2) & sliders;
		while (pinners) {
			temp = popFirstBit(&pinners);
			pinned |= (InBetween[temp][ksq] & b2);
		}
	}
	return pinned;
}

int moveIsLegal(const position_t *pos, int move, u64 pinned) {
	int us, them, ksq, from, to, capsq;
	u64 b;
	/* If we're in check, all pseudo-legal moves are legal, because our
	// check evasion generator only generates true legal moves. */
	if (pos->status->incheck) return TRUE;
	/* Castling moves are checked for legality during move generation. */
	if (isCastle(move)) return TRUE;
	us = pos->side;
	them = us ^ 1;
	from = moveFrom(move);
	to = moveTo(move);
	ksq = getFirstBit(pos->kings&pos->color[us]);
	/* En passant captures are a tricky special case.  Because they are
	// rather uncommon, we do it simply by testing whether the king is attacked
	// after the move is made: */
	if (isEnPassant(move)) {
		capsq = (SQRANK(from) << 3) + SQFILE(to);
		b = pos->occupied;
		b &= ~BitMask[from];
		b &= ~BitMask[capsq];
		b |= BitMask[to];
		return
			(!(rookAttacksBB(ksq, b) & (pos->queens | pos->rooks) & pos->color[them]) &&
				!(bishopAttacksBB(ksq, b) & (pos->queens | pos->bishops) & pos->color[them]));
	}
	/* If the moving piece is a king, check whether the destination
	// square is attacked by the opponent. */
	if (from == ksq) return !(isAtt(pos, them, BitMask[to]));
	/* A non-king move is legal if and only if it is not pinned or it
	// is moving along the ray towards or away from the king. */
	if (!(pinned & BitMask[from])) return TRUE;
	if (Direction[from][ksq] == Direction[to][ksq]) return TRUE;
	return FALSE;
}

/* this returns the bitboard of all pieces attacking a certain square */
u64 attackingPiecesAll(const position_t *pos, int sq) {
	u64 attackers = 0;
	attackers |= PawnCaps[sq][BLACK] & pos->pawns & pos->color[WHITE];
	attackers |= PawnCaps[sq][WHITE] & pos->pawns & pos->color[BLACK];
	attackers |= KnightMoves[sq] & pos->knights;
	attackers |= KingMoves[sq] & pos->kings;
	attackers |= bishopAttacksBB(sq, pos->occupied) & (pos->bishops | pos->queens);
	attackers |= rookAttacksBB(sq, pos->occupied) & (pos->rooks | pos->queens);
	return attackers;
}

/* this returns the bitboard of all pieces of a given side attacking a certain square */
u64 attackingPiecesSide(const position_t *pos, int sq, int side) {
	u64 attackers = 0;
	attackers |= PawnCaps[sq][side ^ 1] & pos->pawns;
	attackers |= KnightMoves[sq] & pos->knights;
	attackers |= KingMoves[sq] & pos->kings;
	attackers |= bishopAttacksBB(sq, pos->occupied) & (pos->bishops | pos->queens);
	attackers |= rookAttacksBB(sq, pos->occupied) & (pos->rooks | pos->queens);
	return (attackers & pos->color[side]);
}

/* this returns the bitboard of sliding pieces attacking in a direction
behind the piece attacker */
u64 behindFigure(u64 QR, u64 QB, u64 occupied, int from, int dir) {
	switch (dir) {
	case -9: return diagonalAttacks(occupied, from) & QB & DirA[0][from];
	case -1: return rankAttacks(occupied, from) & QR & DirA[1][from];
	case 7: return antiDiagAttacks(occupied, from) & QB & DirA[2][from];
	case 8: return fileAttacks(occupied, from) & QR & DirA[3][from];
	case 9: return diagonalAttacks(occupied, from) & QB & DirA[4][from];
	case 1: return rankAttacks(occupied, from) & QR & DirA[5][from];
	case -7: return antiDiagAttacks(occupied, from) & QB & DirA[6][from];
	case -8: return fileAttacks(occupied, from) & QR & DirA[7][from];
	default: return 0;
	}
}

/* this is the Static Exchange Evaluator */
int swap(const position_t *pos, int m) {
	static const pcval[] = { 0, 100, 310, 325, 500, 975, 10000 };
	int to = moveTo(m);
	int from = moveFrom(m);
	int pc = movePiece(m);
	int lastvalue = pcval[pc];
	int n = 1;
	int lsb;
	int dir = Direction[to][from];
	int c = pos->side ^ 1;
	int slist[32];
	int kpos[2];
	u64 attack;
	u64 pinned[2];
	kpos[WHITE] = getFirstBit(pos->kings & pos->color[WHITE]);
	kpos[BLACK] = getFirstBit(pos->kings & pos->color[BLACK]);
	attack = attackingPiecesAll(pos, to) ^ BitMask[from];
	pinned[WHITE] = pinnedPieces(pos, WHITE);
	pinned[BLACK] = pinnedPieces(pos, BLACK);
	slist[0] = pcval[moveCapture(m)];
	if (dir && pc != KING) {
		attack |= behindFigure((pos->queens | pos->rooks), (pos->queens | pos->bishops),
			pos->occupied, from, dir);
	}
	if (isEnPassant(m)) {
		attack |= behindFigure((pos->queens | pos->rooks), (pos->queens | pos->bishops),
			pos->occupied, from, Direction[to][to + (pos->side == WHITE ? -8 : 8)]);
	}
	while (attack) {
		if (attack & pos->pawns & pos->color[c]) {
			lsb = getFirstBit(attack & pos->pawns & pos->color[c]);
			pc = PAWN;
		}
		else if (attack & pos->knights & pos->color[c]) {
			lsb = getFirstBit(attack & pos->knights & pos->color[c]);
			pc = KNIGHT;
		}
		else if (attack & pos->bishops & pos->color[c]) {
			lsb = getFirstBit(attack & pos->bishops & pos->color[c]);
			pc = BISHOP;
		}
		else if (attack & pos->rooks & pos->color[c]) {
			lsb = getFirstBit(attack & pos->rooks & pos->color[c]);
			pc = ROOK;
		}
		else if (attack & pos->queens & pos->color[c]) {
			lsb = getFirstBit(attack & pos->queens & pos->color[c]);
			pc = QUEEN;
		}
		else if (attack & pos->kings & pos->color[c]) {
			lsb = getFirstBit(attack & pos->kings & pos->color[c]);
			pc = KING;
		}
		else break;
		if ((pinned[c] & BitMask[lsb]) &&
			Direction[kpos[c]][to] != Direction[kpos[c]][lsb]) {
			attack ^= BitMask[lsb];
			continue;
		}
		slist[n] = -slist[n - 1] + lastvalue;
		if (pc == PAWN && SQRANK(to) == (c ? 0 : 7))
			lastvalue = pcval[QUEEN] - pcval[PAWN];
		else lastvalue = pcval[pc];
		attack ^= BitMask[lsb];
		dir = Direction[to][lsb];
		if (dir && pc != KING)
			attack |= behindFigure((pos->queens | pos->rooks), (pos->queens | pos->bishops),
				pos->occupied, lsb, dir);
		n++;
		c ^= 1;
	}
	while (--n) {
		if (slist[n] > -slist[n - 1])
			slist[n - 1] = -slist[n];
	}
	return slist[0];
}