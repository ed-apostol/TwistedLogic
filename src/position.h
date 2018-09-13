/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void updateBoards(position_t *pos, int m, int side, int xside) {
	int rook_from, rook_to, epsq, from, to;
	from = moveFrom(m);
	to = moveTo(m);
	switch (moveAction(m)) {
	case PAWN:
		pos->pawns ^= (BitMask[from] | BitMask[to]);
		break;
	case KNIGHT:
		pos->knights ^= (BitMask[from] | BitMask[to]);
		break;
	case BISHOP:
		pos->bishops ^= (BitMask[from] | BitMask[to]);
		break;
	case ROOK:
		pos->rooks ^= (BitMask[from] | BitMask[to]);
		break;
	case QUEEN:
		pos->queens ^= (BitMask[from] | BitMask[to]);
		break;
	case KING:
		pos->kings ^= (BitMask[from] | BitMask[to]);
		break;
	case CASTLE:
		pos->kings ^= (BitMask[from] | BitMask[to]);
		switch (to) {
		case c1: rook_from = a1; rook_to = d1; break;
		case g1: rook_from = h1; rook_to = f1; break;
		case c8: rook_from = a8; rook_to = d8; break;
		case g8: rook_from = h8; rook_to = f8; break;
		}
		pos->rooks ^= (BitMask[rook_from] | BitMask[rook_to]);
		pos->color[side] ^= (BitMask[rook_from] | BitMask[rook_to]);
		break;
	case TWOFORWARD:
		pos->pawns ^= (BitMask[from] | BitMask[to]);
		break;
	case PROMOTE:
		pos->pawns ^= BitMask[from];
		switch (movePromote(m)) {
		case KNIGHT:
			pos->knights ^= BitMask[to];
			break;
		case BISHOP:
			pos->bishops ^= BitMask[to];
			break;
		case ROOK:
			pos->rooks ^= BitMask[to];
			break;
		case QUEEN:
			pos->queens ^= BitMask[to];
			break;
		}
		break;
	}
	switch (moveRemoval(m)) {
	case PAWN:
		pos->pawns ^= BitMask[to];
		pos->color[xside] ^= BitMask[to];
		break;
	case KNIGHT:
		pos->knights ^= BitMask[to];
		pos->color[xside] ^= BitMask[to];
		break;
	case BISHOP:
		pos->bishops ^= BitMask[to];
		pos->color[xside] ^= BitMask[to];
		break;
	case ROOK:
		pos->rooks ^= BitMask[to];
		pos->color[xside] ^= BitMask[to];
		break;
	case QUEEN:
		pos->queens ^= BitMask[to];
		pos->color[xside] ^= BitMask[to];
		break;
	case ENPASSANT:
		switch (side) {
		case WHITE: epsq = to - 8; break;
		case BLACK: epsq = to + 8; break;
		}
		pos->pawns ^= BitMask[epsq];
		pos->color[xside] ^= BitMask[epsq];
		break;
	}
	pos->color[side] ^= (BitMask[from] | BitMask[to]);
	pos->occupied = pos->color[side] | pos->color[xside];
}

/* this undos the move done */
void unmakeMove(position_t *pos) {
	int side, xside, m;
	m = pos->status->lastmove;
	pos->status->lastmove = 0;
	--pos->status;
	--pos->ply;
	xside = pos->side;
	side = xside ^ 1;
	pos->side = side;
	updateBoards(pos, m, side, xside);
	ASSERT(!(pos->color[BLACK] & pos->color[WHITE]));
	ASSERT(pos->side == WHITE || pos->side == BLACK);
	ASSERT(pos->status->castle >= 0 || pos->status->castle <= 15);
	ASSERT(bitCnt(pos->occupied) >= 2 || bitCnt(pos->occupied) <= 32);
	ASSERT(bitCnt(pos->color[WHITE]) >= 1 || bitCnt(pos->color[WHITE]) <= 16);
	ASSERT(bitCnt(pos->color[BLACK]) >= 1 || bitCnt(pos->color[BLACK]) <= 16);
	ASSERT(bitCnt(pos->pawns) >= 0 || bitCnt(pos->pawns) <= 16);
	ASSERT(bitCnt(pos->kings) == 2);
	if (pos->status->epsq != -1)
		ASSERT(pos->status->epsq >= h3 || pos->status->epsq <= h6);
}

/* this updates the position structure from the move being played */
void makeMove(position_t *pos, int m) {
	int rook_from, rook_to, epsq;
	int from = moveFrom(m);
	int to = moveTo(m);
	int side = pos->side;
	int xside = side ^ 1;
	int pc = moveAction(m);
	int capt = moveRemoval(m);
	undo_t *temp;
	temp = ++pos->status;
	temp->lastmove = m;
	updateBoards(pos, m, side, xside);
	++pos->ply;
	pos->side = xside;

	temp->epsq = -1;
	temp->castle = (temp - 1)->castle & CastleMask[from] & CastleMask[to];
	temp->fifty = (temp - 1)->fifty + 1;
	temp->phase = (temp - 1)->phase;
	temp->open[WHITE] = (temp - 1)->open[WHITE];
	temp->open[BLACK] = (temp - 1)->open[BLACK];
	temp->end[WHITE] = (temp - 1)->end[WHITE];
	temp->end[BLACK] = (temp - 1)->end[BLACK];
	temp->hash.b = (temp - 1)->hash.b;
	temp->phash.b = (temp - 1)->phash.b;
	temp->mathash.b = (temp - 1)->mathash.b;
	if ((temp - 1)->epsq != -1) temp->hash.b ^= ZobEpsq[(temp - 1)->epsq].b;
	temp->hash.b ^= ZobCastle[(temp - 1)->castle].b;
	temp->hash.b ^= ZobCastle[temp->castle].b;
	temp->hash.b ^= ZobColor.b;
	switch (pc) {
	case PAWN:
		temp->hash.b ^= Zobrist[side][PAWN][from].b;
		temp->hash.b ^= Zobrist[side][PAWN][to].b;
		temp->phash.b ^= Zobrist[side][PAWN][from].b;
		temp->phash.b ^= Zobrist[side][PAWN][to].b;
		temp->open[side] += PST(side, PAWN, to, OPENING) - PST(side, PAWN, from, OPENING);
		temp->end[side] += PST(side, PAWN, to, ENDGAME) - PST(side, PAWN, from, ENDGAME);
		break;
	case TWOFORWARD:
		switch (side) {
		case WHITE: temp->epsq = to - 8; break;
		case BLACK: temp->epsq = to + 8; break;
		}
		temp->hash.b ^= ZobEpsq[temp->epsq].b;
		temp->hash.b ^= Zobrist[side][PAWN][from].b;
		temp->hash.b ^= Zobrist[side][PAWN][to].b;
		temp->phash.b ^= Zobrist[side][PAWN][from].b;
		temp->phash.b ^= Zobrist[side][PAWN][to].b;
		temp->open[side] += PST(side, PAWN, to, OPENING) - PST(side, PAWN, from, OPENING);
		temp->end[side] += PST(side, PAWN, to, ENDGAME) - PST(side, PAWN, from, ENDGAME);
		break;
	case CASTLE:
		switch (to) {
		case c1: rook_from = a1; rook_to = d1; break;
		case g1: rook_from = h1; rook_to = f1; break;
		case c8: rook_from = a8; rook_to = d8; break;
		case g8: rook_from = h8; rook_to = f8; break;
		}
		temp->hash.b ^= Zobrist[side][KING][from].b;
		temp->hash.b ^= Zobrist[side][KING][to].b;
		temp->hash.b ^= Zobrist[side][ROOK][rook_from].b;
		temp->hash.b ^= Zobrist[side][ROOK][rook_to].b;
		temp->open[side] += PST(side, KING, to, OPENING) - PST(side, KING, from, OPENING);
		temp->end[side] += PST(side, KING, to, ENDGAME) - PST(side, KING, from, ENDGAME);
		temp->open[side] += PST(side, ROOK, rook_to, OPENING) - PST(side, ROOK, rook_from, OPENING);
		temp->end[side] += PST(side, ROOK, rook_to, ENDGAME) - PST(side, ROOK, rook_from, ENDGAME);
		break;
	case PROMOTE:
		temp->hash.b ^= Zobrist[side][PAWN][from].b;
		temp->phash.b ^= Zobrist[side][PAWN][from].b;
		temp->mathash.b ^= ZobMat[side][PAWN][bitCnt(pos->color[side] & pos->pawns) + 1].b;
		temp->mathash.b ^= ZobMat[side][PAWN][bitCnt(pos->color[side] & pos->pawns)].b;
		temp->open[side] += PST(side, movePromote(m), to, OPENING) - PST(side, PAWN, from, OPENING);
		temp->end[side] += PST(side, movePromote(m), to, ENDGAME) - PST(side, PAWN, from, ENDGAME);
		temp->open[side] += PcVal[movePromote(m)] - PcVal[PAWN];
		temp->end[side] += EndPcVal[movePromote(m)] - EndPcVal[PAWN];
		switch (movePromote(m)) {
		case KNIGHT:
			temp->hash.b ^= Zobrist[side][KNIGHT][to].b;
			temp->mathash.b ^= ZobMat[side][KNIGHT][bitCnt(pos->color[side] & pos->knights) - 1].b;
			temp->mathash.b ^= ZobMat[side][KNIGHT][bitCnt(pos->color[side] & pos->knights)].b;
			break;
		case BISHOP:
			temp->hash.b ^= Zobrist[side][BISHOP][to].b;
			temp->mathash.b ^= ZobMat[side][BISHOP][bitCnt(pos->color[side] & pos->bishops) - 1].b;
			temp->mathash.b ^= ZobMat[side][BISHOP][bitCnt(pos->color[side] & pos->bishops)].b;
			break;
		case ROOK:
			temp->hash.b ^= Zobrist[side][ROOK][to].b;
			temp->mathash.b ^= ZobMat[side][ROOK][bitCnt(pos->color[side] & pos->rooks) - 1].b;
			temp->mathash.b ^= ZobMat[side][ROOK][bitCnt(pos->color[side] & pos->rooks)].b;
			break;
		case QUEEN:
			temp->hash.b ^= Zobrist[side][QUEEN][to].b;
			temp->mathash.b ^= ZobMat[side][QUEEN][bitCnt(pos->color[side] & pos->queens) - 1].b;
			temp->mathash.b ^= ZobMat[side][QUEEN][bitCnt(pos->color[side] & pos->queens)].b;
			break;
		}
		break;
	default:
		temp->hash.b ^= Zobrist[side][pc][from].b;
		temp->hash.b ^= Zobrist[side][pc][to].b;
		temp->open[side] += PST(side, pc, to, OPENING) - PST(side, pc, from, OPENING);
		temp->end[side] += PST(side, pc, to, ENDGAME) - PST(side, pc, from, ENDGAME);
		break;
	}

	switch (capt) {
	case EMPTY: break;
	case PAWN:
		temp->hash.b ^= Zobrist[xside][PAWN][to].b;
		temp->phash.b ^= Zobrist[side][PAWN][from].b;
		temp->mathash.b ^= ZobMat[xside][PAWN][bitCnt(pos->color[xside] & pos->pawns) + 1].b;
		temp->mathash.b ^= ZobMat[xside][PAWN][bitCnt(pos->color[xside] & pos->pawns)].b;
		temp->fifty = 0;
		temp->open[xside] -= PcVal[PAWN];
		temp->end[xside] -= EndPcVal[PAWN];
		temp->open[xside] -= PST(side, PAWN, to, OPENING);
		temp->end[xside] -= PST(side, PAWN, to, ENDGAME);
		break;
	case ENPASSANT:
		switch (side) {
		case WHITE: epsq = to - 8; break;
		case BLACK: epsq = to + 8; break;
		}
		ASSERT(pos->status->epsq >= h3 || pos->status->epsq <= h6);
		temp->hash.b ^= Zobrist[xside][PAWN][epsq].b;
		temp->phash.b ^= Zobrist[xside][PAWN][epsq].b;
		temp->mathash.b ^= ZobMat[xside][PAWN][bitCnt(pos->color[xside] & pos->pawns) + 1].b;
		temp->mathash.b ^= ZobMat[xside][PAWN][bitCnt(pos->color[xside] & pos->pawns)].b;
		temp->fifty = 0;
		temp->open[xside] -= PcVal[PAWN];
		temp->end[xside] -= EndPcVal[PAWN];
		temp->open[xside] -= PST(xside, PAWN, epsq, OPENING);
		temp->end[xside] -= PST(xside, PAWN, epsq, ENDGAME);
		break;
	case KNIGHT:
		temp->hash.b ^= Zobrist[xside][KNIGHT][to].b;
		temp->mathash.b ^= ZobMat[xside][KNIGHT][bitCnt(pos->color[xside] & pos->knights) + 1].b;
		temp->mathash.b ^= ZobMat[xside][KNIGHT][bitCnt(pos->color[xside] & pos->knights)].b;
		temp->fifty = 0;
		temp->phase -= 1;
		temp->open[xside] -= PcVal[KNIGHT];
		temp->end[xside] -= EndPcVal[KNIGHT];
		temp->open[xside] -= PST(xside, KNIGHT, to, OPENING);
		temp->end[xside] -= PST(xside, KNIGHT, to, ENDGAME);
		break;
	case BISHOP:
		temp->hash.b ^= Zobrist[xside][BISHOP][to].b;
		temp->mathash.b ^= ZobMat[xside][BISHOP][bitCnt(pos->color[xside] & pos->bishops) + 1].b;
		temp->mathash.b ^= ZobMat[xside][BISHOP][bitCnt(pos->color[xside] & pos->bishops)].b;
		temp->fifty = 0;
		temp->phase -= 1;
		temp->open[xside] -= PcVal[BISHOP];
		temp->end[xside] -= EndPcVal[BISHOP];
		temp->open[xside] -= PST(xside, BISHOP, to, OPENING);
		temp->end[xside] -= PST(xside, BISHOP, to, ENDGAME);
		break;
	case ROOK:
		temp->hash.b ^= Zobrist[xside][ROOK][to].b;
		temp->mathash.b ^= ZobMat[xside][ROOK][bitCnt(pos->color[xside] & pos->rooks) + 1].b;
		temp->mathash.b ^= ZobMat[xside][ROOK][bitCnt(pos->color[xside] & pos->rooks)].b;
		temp->fifty = 0;
		temp->phase -= 2;
		temp->open[xside] -= PcVal[ROOK];
		temp->end[xside] -= EndPcVal[ROOK];
		temp->open[xside] -= PST(xside, ROOK, to, OPENING);
		temp->end[xside] -= PST(xside, ROOK, to, ENDGAME);
		break;
	case QUEEN:
		temp->hash.b ^= Zobrist[xside][QUEEN][to].b;
		temp->mathash.b ^= ZobMat[xside][QUEEN][bitCnt(pos->color[xside] & pos->queens) + 1].b;
		temp->mathash.b ^= ZobMat[xside][QUEEN][bitCnt(pos->color[xside] & pos->queens)].b;
		temp->fifty = 0;
		temp->phase -= 4;
		temp->open[xside] -= PcVal[QUEEN];
		temp->end[xside] -= EndPcVal[QUEEN];
		temp->open[xside] -= PST(xside, QUEEN, to, OPENING);
		temp->end[xside] -= PST(xside, QUEEN, to, ENDGAME);
		break;
	}
	temp->incheck = isAtt(pos, pos->side ^ 1, (pos->kings & pos->color[pos->side]));
	ASSERT(!(pos->color[BLACK] & pos->color[WHITE]));
	ASSERT(pos->side == WHITE || pos->side == BLACK);
	ASSERT(pos->status->castle >= 0 || pos->status->castle <= 15);
	ASSERT(bitCnt(pos->occupied) >= 2 || bitCnt(pos->occupied) <= 32);
	ASSERT(bitCnt(pos->color[WHITE]) >= 1 || bitCnt(pos->color[WHITE]) <= 16);
	ASSERT(bitCnt(pos->color[BLACK]) >= 1 || bitCnt(pos->color[BLACK]) <= 16);
	ASSERT(bitCnt(pos->pawns) >= 0 || bitCnt(pos->pawns) <= 16);
	ASSERT(bitCnt(pos->kings) == 2);
	if (pos->status->epsq != -1)
		ASSERT(pos->status->epsq >= h3 || pos->status->epsq <= h6);
}

/* sets position from a FEN string*/
void setPosition(position_t *pos, char *fen) {
	int rank = 7, file = 0, pc = 0, color = 0, count = 0, i, sq;
	undo_t *temp;
	pos->pawns = EmptyBoardBB;
	pos->knights = EmptyBoardBB;
	pos->bishops = EmptyBoardBB;
	pos->rooks = EmptyBoardBB;
	pos->queens = EmptyBoardBB;
	pos->kings = EmptyBoardBB;
	pos->occupied = EmptyBoardBB;
	pos->color[WHITE] = EmptyBoardBB;
	pos->color[BLACK] = EmptyBoardBB;
	pos->side = WHITE;
	pos->ply = 0;
	for (temp = &pos->undos[0]; temp < &pos->undos[MAXDATA]; temp++) {
		temp->lastmove = EMPTY;
		temp->epsq = -1;
		temp->castle = 0;
		temp->fifty = 0;
		temp->incheck = 0;
		temp->phase = 0;
		temp->open[WHITE] = 0;
		temp->open[BLACK] = 0;
		temp->end[WHITE] = 0;
		temp->end[BLACK] = 0;
		temp->hash.b = EmptyBoardBB;
		temp->phash.b = EmptyBoardBB;
		temp->mathash.b = EmptyBoardBB;
	}
	pos->status = &pos->undos[0];
	while ((rank >= 0) && *fen) {
		count = 1; pc = EMPTY;
		switch (*fen) {
		case 'K': pc = KING; color = WHITE; break;
		case 'k': pc = KING; color = BLACK; break;
		case 'Q': pc = QUEEN; color = WHITE; break;
		case 'q': pc = QUEEN; color = BLACK; break;
		case 'R': pc = ROOK; color = WHITE; break;
		case 'r': pc = ROOK; color = BLACK; break;
		case 'B': pc = BISHOP; color = WHITE; break;
		case 'b': pc = BISHOP; color = BLACK; break;
		case 'N': pc = KNIGHT; color = WHITE; break;
		case 'n': pc = KNIGHT; color = BLACK; break;
		case 'P': pc = PAWN; color = WHITE; break;
		case 'p': pc = PAWN; color = BLACK; break;
		case '/': case ' ': rank--; file = 0; fen++; continue;
		case '1': case '2': case '3': case '4': case '5': case '6':
		case '7': case '8': count = *fen - '0'; break;
		default: Print(3, "info string FEN Error 1!"); return;
		}
		for (i = 0; i < count; i++, file++) {
			sq = (rank * 8) + file;
			ASSERT(pc >= EMPTY && pc <= KING);
			ASSERT(sq >= a1 && sq <= h8);
			ASSERT(color == BLACK || color == WHITE);
			if (pc != EMPTY) {
				switch (pc) {
				case PAWN:
					pos->pawns ^= BitMask[sq];
					break;
				case KNIGHT:
					pos->knights ^= BitMask[sq];
					pos->status->phase += 1;
					break;
				case BISHOP:
					pos->bishops ^= BitMask[sq];
					pos->status->phase += 1;
					break;
				case ROOK:
					pos->rooks ^= BitMask[sq];
					pos->status->phase += 2;
					break;
				case QUEEN:
					pos->queens ^= BitMask[sq];
					pos->status->phase += 4;
					break;
				case KING: pos->kings ^= BitMask[sq]; break;
				}
				pos->status->open[color] += PcVal[pc];
				pos->status->end[color] += EndPcVal[pc];
				pos->status->open[color] += PST(color, pc, sq, OPENING);
				pos->status->end[color] += PST(color, pc, sq, ENDGAME);
				pos->color[color] ^= BitMask[sq];
				pos->occupied ^= BitMask[sq];
				pos->status->hash.b ^= Zobrist[color][pc][sq].b;
				if (pc == PAWN) pos->status->phash.b ^= Zobrist[color][pc][sq].b;
			}
		}
		fen++;
	}
	while (isspace(*fen)) fen++;
	switch (tolower(*fen)) {
	case 'w': pos->side = WHITE; break;
	case 'b': pos->side = BLACK; break;
	default: Print(3, "info string FEN Error 2!\n"); return;
	}
	do { fen++; } while (isspace(*fen));
	while (*fen != '\0' && !isspace(*fen)) {
		if (*fen == 'K') pos->status->castle |= WCKS;
		else if (*fen == 'Q') pos->status->castle |= WCQS;
		else if (*fen == 'k') pos->status->castle |= BCKS;
		else if (*fen == 'q') pos->status->castle |= BCQS;
		fen++;
	}
	while (isspace(*fen)) fen++;
	if (*fen != '\0') {
		if (*fen != '-') {
			if (fen[0] >= 'a' && fen[0] <= 'h' && fen[1] >= '1' && fen[1] <= '8')
				pos->status->epsq = fen[0] - 'a' + (fen[1] - '1') * 8;
			do { fen++; } while (!isspace(*fen));
		}
		do { fen++; } while (isspace(*fen));
		if (isdigit(*fen)) sscanf_s(fen, "%d", &pos->status->fifty);
	}
	pos->status->mathash.b ^= ZobMat[WHITE][PAWN][bitCnt(pos->color[WHITE] & pos->pawns)].b;
	pos->status->mathash.b ^= ZobMat[WHITE][KNIGHT][bitCnt(pos->color[WHITE] & pos->knights)].b;
	pos->status->mathash.b ^= ZobMat[WHITE][BISHOP][bitCnt(pos->color[WHITE] & pos->bishops)].b;
	pos->status->mathash.b ^= ZobMat[WHITE][ROOK][bitCnt(pos->color[WHITE] & pos->rooks)].b;
	pos->status->mathash.b ^= ZobMat[WHITE][QUEEN][bitCnt(pos->color[WHITE] & pos->queens)].b;
	pos->status->mathash.b ^= ZobMat[BLACK][PAWN][bitCnt(pos->color[BLACK] & pos->pawns)].b;
	pos->status->mathash.b ^= ZobMat[BLACK][KNIGHT][bitCnt(pos->color[BLACK] & pos->knights)].b;
	pos->status->mathash.b ^= ZobMat[BLACK][BISHOP][bitCnt(pos->color[BLACK] & pos->bishops)].b;
	pos->status->mathash.b ^= ZobMat[BLACK][ROOK][bitCnt(pos->color[BLACK] & pos->rooks)].b;
	pos->status->mathash.b ^= ZobMat[BLACK][QUEEN][bitCnt(pos->color[BLACK] & pos->queens)].b;
	pos->status->hash.b ^= ZobCastle[pos->status->castle].b;
	if (pos->status->epsq != -1) pos->status->hash.b ^= ZobEpsq[pos->status->epsq].b;
	if (pos->side == BLACK) pos->status->hash.b ^= ZobColor.b;
	pos->status->incheck = isAtt(pos, pos->side ^ 1, (pos->kings & pos->color[pos->side]));

	ASSERT(!(pos->color[BLACK] & pos->color[WHITE]));
	ASSERT(pos->side == WHITE || pos->side == BLACK);
	ASSERT(pos->status->castle >= 0 || pos->status->castle <= 15);
	ASSERT(bitCnt(pos->occupied) >= 2 || bitCnt(pos->occupied) <= 32);
	ASSERT(bitCnt(pos->color[WHITE]) >= 1 || bitCnt(pos->color[WHITE]) <= 16);
	ASSERT(bitCnt(pos->color[BLACK]) >= 1 || bitCnt(pos->color[BLACK]) <= 16);
	ASSERT(bitCnt(pos->pawns) >= 0 || bitCnt(pos->pawns) <= 16);
	ASSERT(bitCnt(pos->kings) == 2);
	if (pos->status->epsq != -1)
		ASSERT(pos->status->epsq >= h3 || pos->status->epsq <= h6);
}