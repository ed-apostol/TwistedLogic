/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/* a utility to print output into different files */
void Print(int vb, char *fmt, ...){
/*// bit 0: stdout
// bit 1: logfile
// bit 2: errfile
// bit 3: dumpfile */
  	va_list ap;
  	va_start(ap, fmt);
  	if (vb&1) {
		vprintf(fmt, ap);
		fflush(stdout);
  	}
  	va_start(ap, fmt);
   	if (logfile&&((vb>>1)&1)) {
     	vfprintf(logfile, fmt, ap);
	    fflush(logfile);
	}
  	va_start(ap, fmt);
   	if (errfile&&((vb>>2)&1)) {
     	vfprintf(errfile, fmt, ap);
	    fflush(errfile);
	}
  	va_start(ap, fmt);
   	if (dumpfile&&((vb>>3)&1)) {
     	vfprintf(dumpfile, fmt, ap);
	    fflush(dumpfile);
	} 
	va_end(ap);
}

/* a utility to print the bits in a position-like format */
void displayBit(u64 a, int x){
    int i, j; 
    for(i = 56; i >= 0; i -= 8){
        Print(x, "\n%d  ",(i / 8) + 1);
        for(j = 0; j < 8; ++j){
             Print(x, "%c ", ((a & ((u64)1 << (i + j))) ? '1' : '_'));
        }    
    }
    Print(x, "\n\n");
    Print(x, "   a b c d e f g h \n\n");
}

/* a utility to convert int move to string */
char *move2Str(int m){
    static char promstr[]="\0pnbrqk";
    static char str[6];
    sprintf(str, "%c%c%c%c%c",
        SQFILE(moveFrom(m)) + 'a',
        '1' + SQRANK(moveFrom(m)),
        SQFILE(moveTo(m)) + 'a',
        '1' + SQRANK(moveTo(m)),
        promstr[movePromote(m)]
    );
    return str;
}

/* a utility to print the position */
void displayBoard(const position_t *pos, int x){
    static char pcstr[] = ".PNBRQK.pnbrqk";
    int i, j, c, p; 
    for(i=56; i>=0; i-=8){
        Print(x, "\n%d  ",(i/8)+1);
        for(j=0;j<8;++j){
            c = getColor(pos, i+j);
            p = getPiece(pos, i+j);
            Print(x, "%c ", pcstr[p+(c?7:0)]);
        }    
    }
    Print(x, "\n\n");
    Print(x, "   a b c d e f g h \n\n");
    Print(x, "%d.%s%s ", (pos->status-&pos->undos[0])/2
        +(pos->side?1:0), pos->side?" ":" ..",
        move2Str(pos->status->lastmove));
    Print(x, "Castle = %d, ", pos->status->castle);
    Print(x, "EP = %d, ", pos->status->epsq);
    Print(x, "Fifty = %d, ", pos->status->fifty);
    Print(x, "Eval = %d, ", eval(pos));
    Print(x, "inCheck = %s\n", 
        isAtt(pos, pos->side^1, pos->kings&pos->color[pos->side])
        ? "T" : "F");
    Print(x, "%s, ", pos->side == WHITE ? "WHITE" : "BLACK");
    Print(x, "Phase = %d, ", pos->status->phase);
    Print(x, "Hash = %llu, ", pos->status->hash);
    Print(x, "PHash = %llu, ", pos->status->phash);
    Print(x, "MatHash = %llu\n\n", pos->status->mathash);
}

/* a utility to get a certain piece from a position given a square */
u32 getPiece(const position_t *pos, int sq){
    ASSERT(sq >= a1 || sq <= h8);
    u64 mask = BitMask[sq];
    if(mask & ~pos->occupied) return EMPTY;
    else if(mask & pos->pawns) return PAWN;
    else if(mask & pos->knights) return KNIGHT;
    else if(mask & pos->bishops) return BISHOP;
    else if(mask & pos->rooks) return ROOK;
    else if(mask & pos->queens) return QUEEN;
    else return KING;
}

/* a utility to get a certain color from a position given a square */
int getColor(const position_t *pos, int sq){
    ASSERT(sq >= a1 || sq <= h8);
    u64 mask = BitMask[sq];
    if(mask & pos->color[WHITE]) return WHITE;
    else if(mask & pos->color[BLACK]) return BLACK;
    else{
        ASSERT(mask & ~pos->occupied);
        return WHITE;
    }    
}

/* for debugging only */
void displayBitPieces(const position_t *pos, int x){
    Print(x, "The position:\n");
    displayBoard(pos, x);
    Print(x, "White pieces:\n");
    displayBit(pos->color[WHITE], x);
    Print(x, "Black pieces:\n");
    displayBit(pos->color[BLACK], x);
    /* Print(x, "occupied bits\n");
    displayBit(pos->occupied, x);
    Print(x, "pawn pieces\n");
    displayBit(pos->pawns, x);
    Print(x, "knight pieces\n");
    displayBit(pos->knights, x);
    Print(x, "bishop pieces\n");
    displayBit(pos->bishops, x);
    Print(x, "rook pieces\n");
    displayBit(pos->rooks, x);
    Print(x, "queen pieces\n");
    displayBit(pos->queens, x);
    Print(x, "king pieces\n");
    displayBit(pos->kings, x); */    
}

/* returns time in milli-seconds */
int getTime(void){
#ifndef _WIN32
    static struct timeval tv;
    static struct timezone tz;
	gettimeofday (&tv, &tz);
	return(tv.tv_sec * 1000 + (tv.tv_usec / 1000));
#else
    static struct _timeb tv;
	_ftime(&tv);
	return(tv.time * 1000 + tv.millitm);
#endif
}

/* parse the move from string and returns a move from the
move list of generated moves if the move string matched 
one of them */
int parseMove(movelist_t *ml, char *s){
    int m, from, to, p;
    from = (s[0] - 'a') + (8 * (s[1] - '1'));
	to = (s[2] - 'a') + (8 * (s[3] - '1'));
	m = (from) | (to << 6);
  	for(ml->pos = 0; ml->pos < ml->size; ml->pos++){
        if(m == (ml->list[ml->pos].m & 0xfff)){
            p = EMPTY;
            if(movePromote(ml->list[ml->pos].m)){
                switch(s[4]){
                    case 'n': case 'N': p = KNIGHT; break;
                    case 'b': case 'B': p = BISHOP; break;
                    case 'r': case 'R': p = ROOK; break;
                    default: p = QUEEN; break;
                }
            }
            if(p == movePromote(ml->list[ml->pos].m)) return ml->list[ml->pos].m;
        }
    }
    return 0;
}

int bioskey(void){
    #ifndef _WIN32
    /* Non-windows version */
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(fileno(stdin), &readfds);
    /* Set to timeout immediately */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    select(16, &readfds, 0, 0, &timeout);
    return (FD_ISSET(fileno(stdin), &readfds));
    #else
    /* Windows-version */
    static int init = 0, pipe;
    static HANDLE inh;
    DWORD dw;
    /* If we're running under XBoard then we can't use _kbhit() as the input commands
    * are sent to us directly over the internal pipe */
    if (stdin->_cnt > 0) return stdin->_cnt;
    if (!init) {
        init = 1;
        inh = GetStdHandle(STD_INPUT_HANDLE);
        pipe = !GetConsoleMode(inh, &dw);
        if (!pipe) {
            SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
            FlushConsoleInputBuffer(inh);
        }
    }
    if(pipe){
        if(!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
        return dw;
    }else{
        GetNumberOfConsoleInputEvents(inh, &dw);
        return dw <= 1 ? 0 : dw;
    }
    #endif
}

int reps(const position_t *pos, int n){
    int i, r = 0;
    for(i = 4; i <= pos->status->fifty, pos->status - i >= &pos->undos[0]; i += 2){
        if((pos->status-i)->hash.b == pos->status->hash.b) r++;
        if(r >= n) return TRUE;
    }    
    return FALSE;
}

/* this must not be called with d == 0 */
int getDirIndex(int d){
    switch(d){
        case -9: return 0;
        case -1: return 1;
        case 7: return 2;
        case 8: return 3;
        case 9: return 4;
        case 1: return 5;
        case -7: return 6;
        case -8: return 7;
        default: ASSERT(FALSE); return 0; 
    }
}
