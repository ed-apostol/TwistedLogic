/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void init_pst(void) {
	const int pawnfile[8] = { -3,-1,0,1,1,0,-1,-3 };
	const int pawnrank[8] = { 0,-3,-2,-1,0,1,2,0 };
	const int knightline[8] = { -4,-2,0,1,1,0,-2,-4 };
	const int knightrank[8] = { -2,-1,0,1,2,3,2,1 };
	const int bishopline[8] = { -3,-1,0,1,1,0,-1,-3 };
	const int rookfile[8] = { -2,-1,0,1,1,0,-1,-2 };
	const int queenline[8] = { -3,-1,0,1,1,0,-1,-3 };
	const int kingline[8] = { -3,-1,0,1,1,0,-1,-3 };
	const int kingfile[8] = { 3,4,2,0,0,2,4,3 };
	const int kingrank[8] = { 1,0,-2,-3,-4,-5,-6,-7 };
	int i, j, k;
	memset(PcSqTb, 0, sizeof(PcSqTb));
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 64; j++) {
			switch (i) {
			case PAWN:
				PST(0, i, j, 0) += pawnfile[SQFILE(j)] * 5;
				break;
			case KNIGHT:
				PST(0, i, j, 0) += knightline[SQFILE(j)] * 5;
				PST(0, i, j, 1) += knightline[SQFILE(j)] * 5;
				PST(0, i, j, 0) += knightline[SQRANK(j)] * 5;
				PST(0, i, j, 1) += knightline[SQRANK(j)] * 5;
				PST(0, i, j, 0) += knightrank[SQRANK(j)] * 5;
				break;
			case BISHOP:
				PST(0, i, j, 0) += bishopline[SQFILE(j)] * 2;
				PST(0, i, j, 1) += bishopline[SQFILE(j)] * 3;
				PST(0, i, j, 0) += bishopline[SQRANK(j)] * 2;
				PST(0, i, j, 1) += bishopline[SQRANK(j)] * 3;
				break;
			case ROOK:
				PST(0, i, j, 0) += rookfile[SQFILE(j)] * 3;
				break;
			case QUEEN:
				/* PST(0,i,j,0) += queenline[SQFILE(j)]*0; */
				PST(0, i, j, 1) += queenline[SQFILE(j)] * 4;
				/* PST(0,i,j,0) += queenline[SQRANK(j)]*0; */
				PST(0, i, j, 1) += queenline[SQRANK(j)] * 4;
				break;
			case KING:
				PST(0, i, j, 0) += kingfile[SQFILE(j)] * 10;
				PST(0, i, j, 1) += kingline[SQFILE(j)] * 12;
				PST(0, i, j, 0) += kingrank[SQRANK(j)] * 10;
				PST(0, i, j, 1) += kingline[SQRANK(j)] * 12;
				break;
			}
		}
	}
	PST(0, PAWN, d3, 0) += 10;
	PST(0, PAWN, e3, 0) += 10;
	PST(0, PAWN, d4, 0) += 20;
	PST(0, PAWN, e4, 0) += 20;
	PST(0, PAWN, d5, 0) += 10;
	PST(0, PAWN, e5, 0) += 10;
	for (i = a1; i <= h1; i++) PST(0, BISHOP, i, 0) -= 10;
	for (i = a1; i <= h1; i++) PST(0, QUEEN, i, 0) -= 5;
	for (i = 0; i < 8; i++) {
		j = (i * 8) + i;
		PST(0, BISHOP, j, 0) += 4;
		j = ((7 - i) * 8) + i;
		PST(0, BISHOP, j, 0) += 4;
	}
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 64; j++) {
			k = ((7 - SQRANK(j)) * 8) + SQFILE(j);
			PST(1, i, k, 0) = PST(0, i, j, 0);
			PST(1, i, k, 1) = PST(0, i, j, 1);
		}
	}
	/* char c;
	for(k = 1; k < 7; k++){
		Print(2, "pc = %d\n", k);
		for(i=56; i>=0; i-=8){
			for(j=0;j<8;++j){
				Print(2, "%d ", PST(0,k,i+j,0));
			}
			Print(2, "\n");
		}
		Print(2, "\n");
		for(i=56; i>=0; i-=8){
			for(j=0;j<8;++j){
				Print(2, "%d ", PST(0,k,i+j,1));
			}
			Print(2, "\n");
		}
		Print(2, "\n\n");
	}
	for(k = 1; k < 7; k++){
		Print(2, "pc = %d\n", k);
		for(i=56; i>=0; i-=8){
			for(j=0;j<8;++j){
				Print(2, "%d ", PST(1,k,i+j,0));
			}
			Print(2, "\n");
		}
		Print(2, "\n");
		for(i=56; i>=0; i-=8){
			for(j=0;j<8;++j){
				Print(2, "%d ", PST(1,k,i+j,1));
			}
			Print(2, "\n");
		}
		Print(2, "\n\n");
	} */
}

/* this initializes the pseudo-constant variables used in the program */
void initArr(void) {
	int i, j, m, k, n;
	u64 bit, bit2;
	const int kingd[] = { -9,-1,7,8,9,1,-7,-8 };
	const int knightd[] = { -17,-10,6,15,17,10,-6,-15 };
	const int wpawnd[] = { 8 };
	const int bpawnd[] = { -8 };
	const int wpawnc[] = { 7,9 };
	const int bpawnc[] = { -7,-9 };
	const int wpawn2mov[] = { 16 };
	const int bpawn2mov[] = { -16 };
	memset(KnightMoves, 0, sizeof(KnightMoves));
	memset(KingMoves, 0, sizeof(KingMoves));
	memset(PawnMoves, 0, sizeof(PawnMoves));
	memset(PawnCaps, 0, sizeof(PawnCaps));
	memset(PawnMoves2, 0, sizeof(PawnMoves2));
	memset(Zobrist, 0, sizeof(Zobrist));
	memset(ZobCastle, 0, sizeof(ZobCastle));
	memset(ZobMat, 0, sizeof(ZobMat));
	memset(ZobEpsq, 0, sizeof(ZobEpsq));
	memset(FillUpAttacks, 0, sizeof(FillUpAttacks));
	memset(AFileAttacks, 0, sizeof(AFileAttacks));
	memset(RankMask, 0, sizeof(RankMask));
	memset(FileMask, 0, sizeof(FileMask));
	memset(Direction, 0, sizeof(Direction));
	memset(DirA, 0, sizeof(DirA));
	memset(InBetween, 0, sizeof(InBetween));
	memset(PromoteMask, 0, sizeof(PromoteMask));
	init_pst();
	ZobColor.key = rand32();
	ZobColor.lock = rand32();
	for (i = 0; i < 16; i++) {
		ZobCastle[i].key = rand32();
		ZobCastle[i].lock = rand32();
	}
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 8; j++) {
			for (k = 0; k < 8; k++) {
				ZobMat[i][j][k].key = rand32();
				ZobMat[i][j][k].lock = rand32();
			}
		}
	}
	for (i = 0; i < 64; i++) {
		ZobEpsq[i].key = rand32();
		ZobEpsq[i].lock = rand32();
	}
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 64; j++) {
			RunTo[i][j] = -1;
		}
	}
	for (i = 0; i < 0x40; i++) {
		for (j = 0; j < 2; j++)
			for (k = 0; k < 7; k++) {
				Zobrist[j][k][i].key = rand32();
				Zobrist[j][k][i].lock = rand32();
			}
		for (j = 0; j < 0x40; j++) {
			if (SQRANK(i) == SQRANK(j)) RankMask[i] |= BitMask[j];
			if (SQFILE(i) == SQFILE(j)) FileMask[i] |= BitMask[j];
		}

		for (j = 0; j < 8; j++) {
			n = i + knightd[j];
			if (n < 64 && n >= 0 && ABS(((n & 7) - (i & 7))) <= 2)
				KnightMoves[i] |= BitMask[n];
		}
		for (j = 0; j < 8; j++) {
			n = i + kingd[j];
			if (n < 64 && n >= 0 && ABS(((n & 7) - (i & 7))) <= 2)
				KingMoves[i] |= BitMask[n];
		}
		for (j = 0; j < 1; j++) {
			n = i + wpawnd[j];
			if (n < 64 && n >= 0 && ABS(((n & 7) - (i & 7))) <= 2)
				PawnMoves[i][WHITE] |= BitMask[n];
		}
		for (j = 0; j < 1; j++) {
			n = i + bpawnd[j];
			if (n < 64 && n >= 0 && ABS(((n & 7) - (i & 7))) <= 2)
				PawnMoves[i][BLACK] |= BitMask[n];
		}
		for (j = 0; j < 2; j++) {
			n = i + wpawnc[j];
			if (n < 64 && n >= 0 && ABS(((n & 7) - (i & 7))) <= 2)
				PawnCaps[i][WHITE] |= BitMask[n];
		}
		for (j = 0; j < 2; j++) {
			n = i + bpawnc[j];
			if (n < 64 && n >= 0 && ABS(((n & 7) - (i & 7))) <= 2)
				PawnCaps[i][BLACK] |= BitMask[n];
		}
		for (j = 0; j < 1; j++) {
			n = i + wpawn2mov[j];
			if (n < 64 && n >= 0 && ABS(((n & 7) - (i & 7))) <= 2)
				PawnMoves2[i][WHITE] |= BitMask[n];
		}
		for (j = 0; j < 1; j++) {
			n = i + bpawn2mov[j];
			if (n < 64 && n >= 0 && ABS(((n & 7) - (i & 7))) <= 2)
				PawnMoves2[i][BLACK] |= BitMask[n];
		}

		if (SQRANK(i) == 6) {
			PromoteMask[WHITE] |= BitMask[i];
		}
		if (SQRANK(i) == 1) {
			PromoteMask[BLACK] |= BitMask[i];
		}
	}
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 64; j++) {
			FillUpAttacks[j][i] = 0x0101010101010101ULL * FirstRankAttacks[j][i];
			AFileAttacks[j][i ^ 7] = ((0x8040201008040201ULL
				* FirstRankAttacks[j][i]) & 0x8080808080808080ULL) >> 7;
		}
	}
	for (i = 0; i < 64; i++) {
		for (k = 0; k < 8; k++) {
			DirA[k][i] = 0;
			for (m = -1, j = i;;) {
				n = j + kingd[k];
				if (n < 0 || n > 63 || (j % 8 == 0 && n % 8 == 7)
					|| (j % 8 == 7 && n % 8 == 0))
					break;
				Direction[i][n] = kingd[k];
				DirA[k][i] |= BitMask[n];
				RunTo[i][n] = m;
				m = j = n;
			}
		}
	}
	for (i = 0; i < 64; i++) {
		for (j = 0; j < 64; j++) {
			k = Direction[i][j];
			if (k != 0) {
				m = getDirIndex(k);
				n = getDirIndex(-k);
				InBetween[i][j] = DirA[m][i] & DirA[n][j];
			}
		}
	}
	/* for(i = 0; i < 64; i++){
		for(j = 0; j < 64; j++){
			k = Direction[i][j];
			if(k != 0){
				displayBit(InBetween[i][j], 8);
				Print(8, "i = %d, j = %d, k = %d\n", i, j, k);
			}
		}
	}  */
}