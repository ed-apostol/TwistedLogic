/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void ponderHit(search_info_t *si) {
	u64 time = getTime() - si->start_time;
	if ((si->iteration >= 5 && si->legalmoves == 1) ||
		(si->mate_found >= 2) ||
		(time > si->time_limit_abs) ||
		(time > si->time_limit_max && (si->currmovenumber == 1 || (
			si->best_value - si->last_value >= -50))) ||
			(time > si->time_limit_max / 16 && si->iteration >= 5 && si->easy && !si->change))
		si->thinking_status = ABORTED;
	else si->thinking_status = THINKING;
}

void check4Input(search_info_t *si) {
	char  input[256];
	if (bioskey()) {
		if (fgets(input, 256, stdin) == NULL)
			strcpy_s(input, 5, "quit\n");
		Print(2, "%s\n", input);
		if (_strnicmp(input, "quit", 4) == 0) {
			quit();
		}
		else if (_strnicmp(input, "stop", 4) == 0) {
			si->thinking_status = ABORTED;
			return;
		}
		else if (_strnicmp(input, "ponderhit", 9) == 0) {
			ponderHit(si);
		}
		else if (_strnicmp(input, "isready", 7) == 0)
			printf("readyok\n");
		else if (_strnicmp(input, "debug", 5) == 0) {
			/* dummy for now */
		}
	}
}

void initNode(search_info_t *si) {
	u64 time;
	si->nodes++;
	si->nodes_since_poll++;
	if (si->nodes_since_poll >= si->nodes_between_polls) {
		si->nodes_since_poll = 0;
		check4Input(si);
		if (si->thinking_status == ABORTED) return;
		time = getTime();
		if (time - si->last_time > 1000) {
			si->last_time = time;
			time = time - si->start_time;
			printf("info time %llu ", time);
			printf("nodes %llu ", si->nodes);
			if (time > 0) printf("nps %llu ", (si->nodes * 1000ULL) / (time));
			printf("\n");
		}
		if (si->thinking_status == THINKING && si->time_is_limited == TRUE
			&& time > si->time_limit_max) {
			if (time < si->time_limit_abs &&
				(si->best_value != -INF && si->last_value - si->best_value >= 50)) {
				si->time_limit_max = si->time_limit_abs;
			}
			else si->thinking_status = ABORTED;
		}
		if (si->thinking_status == THINKING && si->node_is_limited == TRUE
			&& si->nodes > si->node_limit)
			si->thinking_status = ABORTED;
	}
}

move_t *getNextMove(movelist_t *ml) {
	move_t *best, *temp, *start, *end;
	start = &ml->list[ml->pos++];
	end = &ml->list[ml->size];
	if (start >= end) return NULL;
	best = start;
	for (temp = start + 1; temp < end; temp++) {
		if (temp->s > best->s) best = temp;
	}
	if (best == start) return start;
	*temp = *start;
	*start = *best;
	*best = *temp;
	return start;
}

int moveIsTactical(int m) {
	return (m & 0x01dc0000);
}

int captureIsGood(const position_t *pos, int m) {
	int prom = movePromote(m);
	int capt = moveCapture(m);
	int pc = movePiece(m);
	if (prom > EMPTY && prom < QUEEN) return FALSE;
	if (capt != EMPTY) {
		if (prom != EMPTY) return TRUE;
		if (PcVal[capt] >= PcVal[pc]) return TRUE;
	}
	return (swap(pos, m) >= 0);
}

void qpreScore(movelist_t *ml) {
	move_t *m;
	for (m = &ml->list[0]; m < &ml->list[ml->size]; m++) {
		m->s = (moveCapture(m->m) * 6) + movePromote(m->m) - movePiece(m->m);
	}
}

void preScore(const position_t *pos, movelist_t *ml, search_info_t *si) {
	move_t *m; int rank, file;
	for (m = &ml->list[0]; m < &ml->list[ml->size]; m++) {
		if (moveIsTactical(m->m)) {
			m->s = (moveCapture(m->m) * 6) + movePromote(m->m) - movePiece(m->m);
			if (captureIsGood(pos, m->m)) m->s += MAXHIST * 2;
			else m->s -= MAXHIST;
		}
		else if (m->m == si->killer1[pos->ply]) m->s = MAXHIST + 1;
		else if (m->m == si->killer2[pos->ply]) m->s = MAXHIST;
		else m->s = si->history[pos->side][movePiece(m->m)][moveTo(m->m)];
	}
}

void updatePV(int m, int ply, int *pv, int *newpv) {
	int j;
	pv[ply] = m;
	for (j = ply + 1; newpv[j]; j++) pv[j] = newpv[j];
	pv[j] = 0;
}

void displayPV(search_info_t *si, int *pv, int depth, int score) {
	u64 time; int i;
	time = getTime() - si->start_time;
	printf("info currmove %s currmovenumber %d ", move2Str(pv[0]), si->currmovenumber);
	if (ABS(score) < INF - MAXPLY) {
		printf("depth %d seldepth %d score cp %d ", depth, si->maxplysearched, score);
		printf("time %llu ", time);
		printf("nodes %llu ", si->nodes);
		/* printf(" hashfull %d", (tt.used*1000)/tt.size); */
	}
	else {
		printf("depth %d seldepth %d score mate %d ",
			depth, si->maxplysearched, (score > 0) ? (INF - score + 1) / 2 : -(INF + score) / 2);
		printf("time %llu ", time);
		printf("nodes %llu ", si->nodes);
		/* printf(" hashfull %d", (tt.used*1000)/tt.size); */
	}
	if (time > 0) printf("nps %llu ", (si->nodes * 1000ULL) / (time));
	printf("pv ");
	for (i = 0; pv[i]; i++) printf("%s ", move2Str(pv[i]));
	printf("\n");
}

int qSearch(position_t *pos, search_info_t *si, int alpha, int beta, int oldPV[]) {
	int bestvalue = -INF;
	int legal = 0;
	int newPV[MAXPLY];
	int score;
	u64 pinned;
	move_t *move;
	movelist_t ml;

	oldPV[pos->ply] = 0;

	/* check for input and time out here */
	initNode(si);
	if (si->thinking_status == ABORTED) return 0;

	/* check for maximum ply searched */
	if (pos->ply > si->maxplysearched) si->maxplysearched = pos->ply;

	/* repetition detection */
	if (reps(pos, 1) || pos->status->fifty >= 100) return 0;

	/* mate distance pruning here */
	score = INF - pos->ply - 1;
	if (beta > score) {
		beta = score;
		if (alpha >= score) return score;
	}
	score = -INF + pos->ply + 2;
	if (alpha < score) {
		alpha = score;
		if (score >= beta) return score;
	}

	/* out of bounds here */
	if ((pos->ply >= MAXPLY - 1) || ((pos->status - &pos->undos[0]) >= MAXDATA - 1))
		return eval(pos);

	if (pos->status->incheck) {
		genEvasions(pos, &ml);
		preScore(pos, &ml, si);
	}
	else {
		bestvalue = eval(pos);
		if (bestvalue >= beta) return bestvalue;
		if (bestvalue > alpha) alpha = bestvalue;
		genQmoves(pos, &ml);
		qpreScore(&ml);
	}
	pinned = pinnedPieces(pos, pos->side);
	while ((move = getNextMove(&ml)) != NULL) {
		if (!moveIsLegal(pos, move->m, pinned)) continue;
		if (!pos->status->incheck && !captureIsGood(pos, move->m)) continue;

		makeMove(pos, move->m);
		score = -qSearch(pos, si, -beta, -alpha, newPV);
		unmakeMove(pos);
		if (si->thinking_status == ABORTED) return 0;

		legal++;
		if (score > bestvalue) {
			bestvalue = score;
			updatePV(move->m, pos->ply, oldPV, newPV);
			if (score > alpha) {
				if (score >= beta) goto cut;
				alpha = score;
			}
		}
	}
	if (legal == 0) {
		if (pos->status->incheck) return -INF + pos->ply;
	}
cut:
	return bestvalue;
}

int search(position_t *pos, search_info_t *si, int alpha, int beta, int depth, int oldPV[]) {
	int bestvalue = -INF;
	int legal = 0;
	int score;
	int newdepth;
	int newPV[MAXPLY];
	int bestmove = 0;
	u64 pinned;
	move_t *move;
	movelist_t ml;

	oldPV[pos->ply] = 0;

	if (depth <= 0) return qSearch(pos, si, alpha, beta, oldPV);

	/* check for input and time out here */
	initNode(si);
	if (si->thinking_status == ABORTED) return 0;

	/* check for maximum ply searched */
	if (pos->ply > si->maxplysearched) si->maxplysearched = pos->ply;

	/* repetition detection */
	if (reps(pos, 1) || pos->status->fifty >= 100) return 0;

	/* mate distance pruning here */
	score = INF - pos->ply - 1;
	if (beta > score) {
		beta = score;
		if (alpha >= score) return score;
	}
	score = -INF + pos->ply + 2;
	if (alpha < score) {
		alpha = score;
		if (score >= beta) return score;
	}

	/* transposition table here */

	/* out of bounds here */
	if ((pos->ply >= MAXPLY - 1) || ((pos->status - &pos->undos[0]) >= MAXDATA - 1))
		return eval(pos);

	/* null move pruning here */

	/* internal iterative deepening here */

	/* move generation here */
	if (pos->status->incheck) genEvasions(pos, &ml);
	else genMoves(pos, &ml);

	preScore(pos, &ml, si);
	pinned = pinnedPieces(pos, pos->side);

	while ((move = getNextMove(&ml)) != NULL) {
		if (!moveIsLegal(pos, move->m, pinned)) continue;

		makeMove(pos, move->m);

		if (pos->status->incheck) newdepth = depth;
		else newdepth = depth - 1;

		if (legal == 0 || (beta == alpha + 1))
			score = -search(pos, si, -beta, -alpha, newdepth, newPV);
		else {
			score = -search(pos, si, -alpha - 1, -alpha, newdepth, newPV);
			if (si->thinking_status != ABORTED && score > alpha) {
				score = -search(pos, si, -beta, -alpha, newdepth, newPV);
			}
		}
		unmakeMove(pos);
		if (si->thinking_status == ABORTED) return 0;

		legal++;
		if (score > bestvalue) {
			bestvalue = score;
			bestmove = move->m;
			updatePV(bestmove, pos->ply, oldPV, newPV);
			if (score > alpha) {
				if (score >= beta) goto cut;
				alpha = score;
			}
		}
	}
	if (legal == 0) {
		if (pos->status->incheck) return -INF + pos->ply;
		else return 0;
	}
cut:
	if (bestmove && !moveIsTactical(bestmove)) {
		si->history[pos->side][movePiece(bestmove)][moveTo(bestmove)] += depth * depth;
		if (si->history[pos->side][movePiece(bestmove)][moveTo(bestmove)] >= MAXHIST) {
			int i, j, k;
			for (i = 0; i < 2; i++)
				for (j = 0; j < 8; j++)
					for (k = 0; k < 64; k++)
						si->history[i][j][k] = (si->history[i][j][k] + 1) / 2;
		}
		if (si->killer1[pos->ply] != bestmove) {
			si->killer2[pos->ply] = si->killer1[pos->ply];
			si->killer1[pos->ply] = bestmove;
		}
	}
	return bestvalue;
}

void getBestMove(position_t *pos, int infinite, int wtime, int btime, int winc,
	int binc, int movestogo, int mate, int ponder, int maxdepth, int nodes, int movetime,
	int moves[], int rootPV[]) {
	int id;
	int score;
	int alpha;
	int newPV[MAXPLY];
	u64 pinned;
	u64 mytime, myotim, t_inc, max_time, alloc, time;
	move_t *move;
	movelist_t ml, mlt;
	search_info_t si;

	/* initialization */
	si.thinking_status = ABORTED;
	si.depth_is_limited = FALSE;
	si.depth_limit = 0;
	si.moves_is_limited = FALSE;
	si.time_is_limited = FALSE;
	si.time_limit_max = 0;
	si.time_limit_abs = 0;
	si.node_is_limited = FALSE;
	si.node_limit = 0;
	si.nodes_since_poll = 0;
	si.nodes_between_polls = 10000;
	si.nodes = 0;
	si.start_time = si.last_time = getTime();
	si.best_value = -INF;
	si.last_value = -INF;
	si.change = FALSE;
	si.easy = FALSE;

	memset(si.history, 0, sizeof(si.history));
	memset(si.killer1, 0, sizeof(si.killer1));
	memset(si.killer2, 0, sizeof(si.killer2));

	if (infinite) {
		si.depth_is_limited = TRUE;
		si.depth_limit = MAXPLY;
		Print(3, "info string Infinite\n");
	}
	if (maxdepth > 0) {
		si.depth_is_limited = TRUE;
		si.depth_limit = maxdepth;
		Print(3, "info string Depth is limited to %d half moves\n", si.depth_limit);
	}
	if (mate > 0) {
		si.depth_is_limited = TRUE;
		si.depth_limit = mate * 2 - 1;
		Print(3, "info string Mate in %d half moves\n", si.depth_limit);
	}
	if (nodes > 0) {
		si.node_is_limited = TRUE;
		si.node_limit = nodes;
		Print(3, "info string Nodes is limited to %d positions\n", si.node_limit);
	}
	if (moves[0]) {
		si.moves_is_limited = TRUE;
		Print(3, "info string Moves is limited\n");
	}
	if (movetime > 0) {
		si.time_is_limited = TRUE;
		si.time_limit_max = si.start_time + movetime;
		si.time_limit_abs = si.start_time + movetime;
		Print(3, "info string Fixed time per move: %d ms\n", movetime);
	}
	if (pos->side == WHITE) {
		mytime = wtime;
		myotim = btime;
		t_inc = winc;
	}
	else {
		mytime = btime;
		myotim = wtime;
		t_inc = binc;
	}
	if (mytime > 0) {
		si.time_is_limited = TRUE;
		max_time = ((mytime * 95) / 100) - 1000;
		if (max_time < 0) max_time = 0;

		if (movestogo <= 0 || movestogo > 30) movestogo = 30;
		alloc = (max_time + (t_inc * (movestogo - 1))) / movestogo;
		alloc = (ponder ? (alloc * 125 / 100) : alloc);
		if (alloc > max_time) alloc = max_time;
		si.time_limit_max = alloc;

		alloc = (max_time + ((t_inc * (movestogo - 1)) / movestogo)) / 3;
		if (alloc < si.time_limit_max) alloc = si.time_limit_max;
		if (alloc > max_time) alloc = max_time;
		si.time_limit_abs = alloc;

		Print(3, "info string Time is limited: ");
		Print(3, "max = %d, ", si.time_limit_max);
		Print(3, "abs = %d\n", si.time_limit_abs);
		si.time_limit_max += si.start_time;
		si.time_limit_abs += si.start_time;
	}
	if (infinite) {
		si.thinking_status = ANALYSING;
		Print(3, "info string Search status is ANALYSING\n");
	}
	else if (ponder) {
		si.thinking_status = PONDERING;
		Print(3, "info string Search status is PONDERING\n");
	}
	else {
		si.thinking_status = THINKING;
		Print(3, "info string Search status is THINKING\n");
	}

	pos->ply = 0;
	rootPV[pos->ply] = 0;
	newPV[pos->ply + 1] = 0;

	if (si.moves_is_limited == TRUE) {
		for (ml.size = 0; moves[ml.size] != 0; ml.size++)
			ml.list[ml.size].m = moves[ml.size];
	}
	else if (pos->status->incheck) {
		genEvasions(pos, &mlt);
	}
	else {
		genMoves(pos, &mlt);
	}

	/* this returns the pinned pieces of a certain side */
	pinned = pinnedPieces(pos, pos->side);
	/* determine illegal moves and score initial root moves */
	ml.size = 0;
	for (mlt.pos = 0; mlt.pos < mlt.size; mlt.pos++) {
		if (!moveIsLegal(pos, mlt.list[mlt.pos].m, pinned)) continue;
		ml.list[ml.size] = mlt.list[mlt.pos];
		makeMove(pos, ml.list[ml.size].m);
		ml.list[ml.size].s = -qSearch(pos, &si, -INF, INF, newPV);
		unmakeMove(pos);
		ml.size++;
	}
	for (ml.pos = 0; ml.pos < ml.size; ml.pos++) {
		makeMove(pos, ml.list[ml.pos].m);
		ASSERT(!isAtt(pos, pos->side, pos->kings&pos->color[pos->side ^ 1]));
		if (isAtt(pos, pos->side, pos->kings&pos->color[pos->side ^ 1])) {
			displayBoard(pos, 8);
			Print(8, "move = %s\n", move2Str(ml.list[ml.pos].m));
		}
		unmakeMove(pos);
	}
	if (ml.size == 0) {
		Print(3, "info string No legal moves found\n");
		return;
	}
	si.legalmoves = ml.size;
	si.easy = si.change = FALSE;
	si.mate_found = 0;
	for (id = 2; id <= MAXPLY; id++) {
		printf("info depth %d\n", id);
		ml.pos = 0;
		alpha = -INF;
		si.best_value = -INF;
		si.iteration = id;
		si.maxplysearched = 0;

		while ((move = getNextMove(&ml)) != NULL) {
			si.currmovenumber = ml.pos;
			makeMove(pos, move->m);

			if (ml.pos == 1)
				score = -search(pos, &si, -INF, INF, id - 1, newPV);
			else {
				score = -search(pos, &si, -alpha - 1, -alpha, id - 1, newPV);
				if (si.thinking_status != ABORTED && score > alpha) {
					score = -search(pos, &si, -INF, -alpha, id - 1, newPV);
					si.change = TRUE;
				}
			}
			unmakeMove(pos);
			if (si.thinking_status == ABORTED) break;

			move->s = score;
			if (score > si.best_value) si.best_value = score;

			if (score > alpha) {
				alpha = score;
				updatePV(move->m, pos->ply, rootPV, newPV);
				displayPV(&si, rootPV, id, alpha);
			}
		}
		time = getTime();
		if (si.thinking_status == ABORTED || time - si.last_time > 1000) {
			si.last_time = time;
			time = si.last_time - si.start_time;
			printf("info depth %d ", id);
			printf("seldepth %d ", si.maxplysearched);
			printf("time %llu ", time);
			printf("nodes %llu ", si.nodes);
			if (time > 0) printf("nps %llu", (si.nodes * 1000ULL) / (time));
			printf("\n");
		}
		if (si.thinking_status == ABORTED) break;

		/* time management code */
		if (id >= 2 && si.legalmoves >= 2 && si.change == FALSE
			&& ml.list[0].s > ml.list[1].s + 150 && si.best_value - si.last_value >= -50) {
			si.easy = TRUE;
		}
		if (id >= 5 && abs(si.best_value) > INF - MAXPLY) {
			si.mate_found++;
		}
		if (si.thinking_status == THINKING) {
			if (si.depth_is_limited && id >= si.depth_limit) {
				si.thinking_status = ABORTED;
			}
			else if (si.mate_found >= 2) {
				si.thinking_status = ABORTED;
			}
			else if (id >= 5 && si.legalmoves == 1) {
				si.thinking_status = ABORTED;
			}
			else if ((getTime() - si.start_time >
				((si.time_limit_max - si.start_time) * 60) / 100) && si.change == FALSE
				&& (si.last_value - si.best_value) <= 50) {
				si.thinking_status = ABORTED;
			}
			else if (getTime() - si.start_time >
				((si.time_limit_max - si.start_time) * 20) / 100 && si.easy == TRUE) {
				si.thinking_status = ABORTED;
			}
		}
		if (si.thinking_status == ABORTED) break;
		si.last_value = si.best_value;
	}
	if (si.thinking_status != ABORTED) {
		Print(3, "info string Waiting for stop, quit, or ponderhit\n");
	}
	while (si.thinking_status != THINKING && si.thinking_status != ABORTED) {
		check4Input(&si);
	}
	return;
}