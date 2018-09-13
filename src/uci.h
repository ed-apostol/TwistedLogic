/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void uciParseSearchmoves(movelist_t *ml, char *str, int moves[]) {
	char *c, movestr[10];
	int i; int m;
	int *move = moves;
	c = str;
	while (isspace(*c)) c++;
	while (*c != '\0') {
		i = 0;
		while (*c != '\0' && !isspace(*c) && i < 9) movestr[i++] = *c++;
		if (i >= 4 && 'a' <= movestr[0] && movestr[0] <= 'h' &&
			'1' <= movestr[1] && movestr[1] <= '8' &&
			'a' <= movestr[2] && movestr[2] <= 'h' &&
			'1' <= movestr[3] && movestr[3] <= '8') {
			m = parseMove(ml, movestr);
			if (m) *move++ = m;
		}
		else break;
		while (isspace(*c)) c++;
	}
	*move = 0;
}

void uciGo(position_t *pos, char *options) {
	char *ponder, *infinite;
	char *c;
	int wtime = 0,
		btime = 0,
		winc = 0,
		binc = 0,
		movestogo = 0,
		depth = 0,
		nodes = 0,
		mate = 0,
		movetime = 0;
	int moves[MAXMOVES];
	int RootPV[MAXPLY];
	movelist_t ml;
	RootPV[0] = 0;
	moves[0] = 0;
	infinite = strstr(options, "infinite");
	ponder = strstr(options, "ponder");
	c = strstr(options, "wtime");
	if (c != NULL) sscanf_s(c + 6, "%d", &wtime);
	c = strstr(options, "btime");
	if (c != NULL) sscanf_s(c + 6, "%d", &btime);
	c = strstr(options, "winc");
	if (c != NULL) sscanf_s(c + 5, "%d", &winc);
	c = strstr(options, "binc");
	if (c != NULL) sscanf_s(c + 5, "%d", &binc);
	c = strstr(options, "movestogo");
	if (c != NULL) sscanf_s(c + 10, "%d", &movestogo);
	c = strstr(options, "depth");
	if (c != NULL) sscanf_s(c + 6, "%d", &depth);
	c = strstr(options, "nodes");
	if (c != NULL) sscanf_s(c + 6, "%d", &nodes);
	c = strstr(options, "mate");
	if (c != NULL) sscanf_s(c + 5, "%d", &mate);
	c = strstr(options, "movetime");
	if (c != NULL) sscanf_s(c + 9, "%d", &movetime);
	c = strstr(options, "searchmoves");
	if (c != NULL) {
		genMoves(pos, &ml);
		uciParseSearchmoves(&ml, c + 12, moves);
	}
	getBestMove(pos, infinite != NULL, wtime, btime, winc, binc, movestogo, mate,
		ponder != NULL, depth, nodes, movetime, moves, RootPV);
	if (!RootPV[0]) {
		Print(3, "info string No legal move found. Start a new game.\n");
		return;
	}
	else {
		Print(3, "bestmove %s", move2Str(RootPV[0]));
		if (RootPV[1]) Print(3, " ponder %s", move2Str(RootPV[1]));
		Print(3, "\n");
	}
	return;
}

void uciStart(position_t *pos) {
	setPosition(pos, FenString[0]);
	/* trans_clear(); */
	Print(3, "id name Twisted Logic alpha\n");
	Print(3, "id author Edsel Apostol\n");
	Print(3, "option name Hash type spin default 1 min 1 max 4096\n");
	Print(3, "option name Ponder type check default false\n");
	Print(3, "uciok\n");
}

void uciSetPosition(position_t *pos, char *str) {
	char *c = str, *m;
	char movestr[10];
	int move, i;
	movelist_t ml;
	while (isspace(*c)) c++;
	if (_strnicmp(c, "startpos", 8) == 0) {
		c += 8; while (isspace(*c)) c++;
		setPosition(pos, FenString[0]);
	}
	else if (_strnicmp(c, "fen", 3) == 0) {
		c += 3; while (isspace(*c)) c++;
		setPosition(pos, c);
		while (*c != '\0' && _strnicmp(c, "moves", 5) != 0) c++;
	}
	while (isspace(*c)) c++;
	if (_strnicmp(c, "moves", 5) == 0) {
		c += 5; while (isspace(*c)) c++;
		while (*c != '\0') {
			m = movestr;
			while (*c != '\0' && !isspace(*c)) *m++ = *c++;
			*m = '\0';
			if (pos->status->incheck) genEvasions(pos, &ml);
			else genMoves(pos, &ml);
			move = parseMove(&ml, movestr);
			if (!move) {
				Print(3, "info string Illegal move: %s\n", movestr);
				return;
			}
			else makeMove(pos, move);
			while (isspace(*c)) c++;
		}
	}
}