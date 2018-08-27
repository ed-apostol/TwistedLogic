/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/* utilities for move generation */
u32 GenOneForward(u32 f,u32 t)             
    {return (f | (t<<6) | (PAWN<<12))   ;}
u32 GenTwoForward(u32 f,u32 t)             
    {return (f | (t<<6) | (PAWN<<12)   | (1<<16))  ;}
u32 GenPromote(u32 f,u32 t,u32 r,u32 c)  
    {return (f | (t<<6) | (PAWN<<12)   | (c<<18)  | (r<<22) | (1<<17)) ;}
u32 GenPromoteStraight(u32 f,u32 t,u32 r) 
    {return (f | (t<<6) | (PAWN<<12)   | (r<<22)  | (1<<17))   ;}
u32 GenEnPassant(u32 f,u32 t)              
    {return (f | (t<<6) | (PAWN<<12)   | (PAWN<<18) | (1<<21)) ;}
u32 GenPawnMove(u32 f,u32 t,u32 c)        
    {return (f | (t<<6) | (PAWN<<12)   | (c<<18)) ;}
u32 GenKnightMove(u32 f,u32 t,u32 c)      
    {return (f | (t<<6) | (KNIGHT<<12) | (c<<18)) ;}
u32 GenBishopMove(u32 f,u32 t,u32 c)      
    {return (f | (t<<6) | (BISHOP<<12) | (c<<18)) ;}
u32 GenRookMove(u32 f,u32 t,u32 c)        
    {return (f | (t<<6) | (ROOK<<12)   | (c<<18)) ;}
u32 GenQueenMove(u32 f,u32 t,u32 c)       
    {return (f | (t<<6) | (QUEEN<<12)  | (c<<18)) ;}
u32 GenKingMove(u32 f,u32 t,u32 c)        
    {return (f | (t<<6) | (KING<<12)   | (c<<18)) ;}
u32 GenWhiteOO()                             
    {return (e1 | (g1<<6) | (KING<<12)  | (1<<15)) ;}
u32 GenWhiteOOO()                            
    {return (e1 | (c1<<6) | (KING<<12)  | (1<<15)) ;}
u32 GenBlackOO()                             
    {return (e8 | (g8<<6) | (KING<<12)  | (1<<15)) ;}
u32 GenBlackOOO()                            
    {return (e8 | (c8<<6) | (KING<<12)  | (1<<15)) ;}
    
u32 moveFrom(u32 m)      
    {return (63&(m))     ;}        /* Get from square */
u32 moveTo(u32 m)        
    {return (63&(m>>6))  ;}        /* Get to square */
u32 movePiece(u32 m)     
    {return  (7&(m>>12)) ;}        /* Get the piece moving */
u32 moveAction(u32 m)    
    {return (63&(m>>12)) ;}        /* Get action to do */
u32 moveCapture(u32 m)   
    {return  (7&(m>>18)) ;}        /* Get the capture piece */
u32 moveRemoval(u32 m)   
    {return (15&(m>>18)) ;}        /* Get removal to be done */
u32 movePromote(u32 m)   
    {return  (7&(m>>22)) ;}        /* Get promote value */

u32 isCastle(u32 m){ return ((m>>15)&1) ;}
u32 isPawn2Forward(u32 m){ return ((m>>16)&1) ;}
u32 isPromote(u32 m){ return ((m>>17)&1) ;}  
u32 isEnPassant(u32 m){ return ((m>>21)&1) ;}  

/* the move generator, this generates all pseudo-legal moves,
castling is generated legally */
void genMoves(const position_t *pos, movelist_t *ml){
    int from, to;
    u64 pc_bits, mv_bits;
    u64 occupied = pos->occupied;
    u64 allies = pos->color[pos->side];
    u64 mask = ~allies;
    ml->size = 0;    
    ml->pos = 0;
    pc_bits = pos->pawns & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        /* pawn move 1 forward */
        mv_bits = PawnMoves[from][pos->side] & ~occupied;  
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            if((PromoteMask[pos->side] & BitMask[from])){
                ml->list[ml->size++].m = GenPromoteStraight(from, to, KNIGHT);
                ml->list[ml->size++].m = GenPromoteStraight(from, to, BISHOP);
                ml->list[ml->size++].m = GenPromoteStraight(from, to, ROOK);
                ml->list[ml->size++].m = GenPromoteStraight(from, to, QUEEN);
            }else
                ml->list[ml->size++].m = GenOneForward(from, to);
        }
        /* pawn captures */
        mv_bits = PawnCaps[from][pos->side] & pos->color[pos->side^1];    
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            if((PromoteMask[pos->side] & BitMask[from])){
                ml->list[ml->size++].m = GenPromote(from, to, KNIGHT, getPiece(pos, to));
                ml->list[ml->size++].m = GenPromote(from, to, BISHOP, getPiece(pos, to));
                ml->list[ml->size++].m = GenPromote(from, to, ROOK, getPiece(pos, to));
                ml->list[ml->size++].m = GenPromote(from, to, QUEEN, getPiece(pos, to));
            }else
                ml->list[ml->size++].m = GenPawnMove(from, to, getPiece(pos, to));
        }
        /* pawn moves 2 forward */
        mv_bits = 0;
        if((PawnMoves[from][pos->side] & ~occupied) && (PromoteMask[pos->side^1] & BitMask[from])){
            mv_bits = PawnMoves2[from][pos->side] & ~occupied;
        } 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenTwoForward(from, to);
        }
    }
    pc_bits = pos->knights & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = KnightMoves[from] & mask;
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenKnightMove(from, to, getPiece(pos, to));  
        }
    }
    pc_bits = pos->bishops & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = bishopAttacksBB(from, occupied) & mask; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenBishopMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->rooks & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = rookAttacksBB(from, occupied) & mask; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenRookMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->queens & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = queenAttacksBB(from, occupied) & mask; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenQueenMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->kings & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = KingMoves[from] & mask; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenKingMove(from, to, getPiece(pos, to));
        }
    }
    if(pos->side == BLACK){
        if((pos->status->castle&BCKS) && (!(occupied&(F8|G8)))){
            if(!isAtt(pos, pos->side^1, E8) && !isAtt(pos, pos->side^1, F8)
            && !isAtt(pos, pos->side^1, G8))
             ml->list[ml->size++].m = GenBlackOO();
        }
        if((pos->status->castle&BCQS) && (!(occupied&(B8|C8|D8)))){
            if(!isAtt(pos, pos->side^1, E8) && !isAtt(pos, pos->side^1, D8)
            && !isAtt(pos, pos->side^1, C8))
            ml->list[ml->size++].m = GenBlackOOO();
        }
    }else{    
        if((pos->status->castle&WCKS) && (!(occupied&(F1|G1)))){
            if(!isAtt(pos, pos->side^1, E1) && !isAtt(pos, pos->side^1, F1)
            && !isAtt(pos, pos->side^1, G1))
            ml->list[ml->size++].m = GenWhiteOO();
        }
        if((pos->status->castle&WCQS) && (!(occupied&(B1|C1|D1)))){
            if(!isAtt(pos, pos->side^1, E1) && !isAtt(pos, pos->side^1, D1)
            && !isAtt(pos, pos->side^1, C1))
            ml->list[ml->size++].m = GenWhiteOOO();
        }
    }
    if((pos->status->epsq != -1)){
        mv_bits = pos->pawns & pos->color[pos->side] 
        & PawnCaps[pos->status->epsq][pos->side^1];
        while(mv_bits){
            from = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenEnPassant(from, pos->status->epsq);
        }
    }    
}

/* this generate captures including en-passant captures, and promotions*/
void genQmoves(const position_t *pos, movelist_t *ml){
    int from, to;
    u64 pc_bits, mv_bits;
    u64 occupied = pos->occupied;
    u64 allies = pos->color[pos->side];
    u64 mask = pos->color[pos->side^1];
    ml->size = 0;    
    ml->pos = 0;
    pc_bits = pos->pawns & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        /* pawn move 1 forward - promotions only*/
        mv_bits = 0;
        if((PromoteMask[pos->side] & BitMask[from]))
            mv_bits = PawnMoves[from][pos->side] & ~occupied;  
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenPromoteStraight(from, to, KNIGHT);
            ml->list[ml->size++].m = GenPromoteStraight(from, to, BISHOP);
            ml->list[ml->size++].m = GenPromoteStraight(from, to, ROOK);
            ml->list[ml->size++].m = GenPromoteStraight(from, to, QUEEN);
        }
        /* pawn captures */
        mv_bits = PawnCaps[from][pos->side] & pos->color[pos->side^1];    
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            if((PromoteMask[pos->side] & BitMask[from])){
                ml->list[ml->size++].m = GenPromote(from, to, KNIGHT, getPiece(pos, to));
                ml->list[ml->size++].m = GenPromote(from, to, BISHOP, getPiece(pos, to));
                ml->list[ml->size++].m = GenPromote(from, to, ROOK, getPiece(pos, to));
                ml->list[ml->size++].m = GenPromote(from, to, QUEEN, getPiece(pos, to));
            }else
                ml->list[ml->size++].m = GenPawnMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->knights & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = KnightMoves[from] & mask;
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenKnightMove(from, to, getPiece(pos, to));  
        }
    }
    pc_bits = pos->bishops & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = bishopAttacksBB(from, occupied) & mask; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenBishopMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->rooks & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = rookAttacksBB(from, occupied) & mask; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenRookMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->queens & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = queenAttacksBB(from, occupied) & mask; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenQueenMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->kings & allies;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = KingMoves[from] & mask; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenKingMove(from, to, getPiece(pos, to));
        }
    }
    if((pos->status->epsq != -1)){
        mv_bits = pos->pawns & pos->color[pos->side] 
        & PawnCaps[pos->status->epsq][pos->side^1];
        while(mv_bits){
            from = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenEnPassant(from, pos->status->epsq);
        }
    }    
}

/* this generate legal moves when in check */
void genEvasions(const position_t *pos, movelist_t *ml){
    int sqchecker, from, to, ksq, side, xside;
    u64  pc_bits, mv_bits, enemies, friends, temp, checkers, pinned;
    ml->size = 0;    
    ml->pos = 0;
    side = pos->side;
    xside = side ^ 1;
    friends = pos->color[side];
    enemies = pos->color[xside];
    ksq = getFirstBit(pos->kings&pos->color[side]);
    checkers = attackingPiecesSide(pos, ksq, xside);
    mv_bits = KingMoves[ksq] & ~pos->color[side];
    while(mv_bits){
        to = popFirstBit(&mv_bits);   
        temp = pos->occupied ^ BitMask[ksq] ^ BitMask[to]; 
        if(((PawnCaps[to][side] & pos->pawns & enemies) == EmptyBoardBB) &&
        ((KnightMoves[to] & pos->knights & enemies) == EmptyBoardBB) &&
        ((KingMoves[to] & pos->kings & enemies) == EmptyBoardBB) &&
        ((bishopAttacksBB(to, temp) & pos->bishops & enemies) == EmptyBoardBB) &&
        ((rookAttacksBB(to, temp) & pos->rooks & enemies) == EmptyBoardBB) &&
        ((queenAttacksBB(to, temp) & pos->queens & enemies) == EmptyBoardBB))
            ml->list[ml->size++].m = GenKingMove(ksq, to, getPiece(pos, to));   
    }
    if(checkers & (checkers - 1)) return;
    pinned = pinnedPieces(pos, side);
    sqchecker = getFirstBit(checkers);
    mv_bits = PawnCaps[sqchecker][xside] & pos->pawns & friends & ~pinned;
    while(mv_bits){
        from = popFirstBit(&mv_bits);
        if((PromoteMask[side] & BitMask[from])){
            ml->list[ml->size++].m = GenPromote(from, sqchecker, KNIGHT, getPiece(pos, sqchecker));
            ml->list[ml->size++].m = GenPromote(from, sqchecker, BISHOP, getPiece(pos, sqchecker));
            ml->list[ml->size++].m = GenPromote(from, sqchecker, ROOK, getPiece(pos, sqchecker));
            ml->list[ml->size++].m = GenPromote(from, sqchecker, QUEEN, getPiece(pos, sqchecker));
        }else
            ml->list[ml->size++].m = GenPawnMove(from, sqchecker, getPiece(pos, sqchecker));
    }
    mv_bits = KnightMoves[sqchecker] & pos->knights & friends & ~pinned;
    while(mv_bits){
        from = popFirstBit(&mv_bits);
        ml->list[ml->size++].m = GenKnightMove(from, sqchecker, getPiece(pos, sqchecker));  
    }
    mv_bits = bishopAttacksBB(sqchecker, pos->occupied) & pos->bishops & friends & ~pinned;
    while(mv_bits){
        from = popFirstBit(&mv_bits);
        ml->list[ml->size++].m = GenBishopMove(from, sqchecker, getPiece(pos, sqchecker));  
    }
    mv_bits = rookAttacksBB(sqchecker, pos->occupied) & pos->rooks & friends & ~pinned;
    while(mv_bits){
        from = popFirstBit(&mv_bits);
        ml->list[ml->size++].m = GenRookMove(from, sqchecker, getPiece(pos, sqchecker));  
    }
    mv_bits = queenAttacksBB(sqchecker, pos->occupied) & pos->queens & friends & ~pinned;
    while(mv_bits){
        from = popFirstBit(&mv_bits);
        ml->list[ml->size++].m = GenQueenMove(from, sqchecker, getPiece(pos, sqchecker));  
    }
    if(!(checkers & (pos->queens|pos->rooks|pos->bishops) & pos->color[xside])) return;
    temp = InBetween[sqchecker][ksq];
    pc_bits = pos->pawns & pos->color[side] & ~pinned;
    if(side == WHITE) mv_bits = (pc_bits << 8) & temp;
    else mv_bits = (pc_bits >> 8) & temp;  
    while(mv_bits){
        to = popFirstBit(&mv_bits);
        if(side == WHITE) from = to - 8;
        else from = to + 8;
        if((PromoteMask[side] & BitMask[from])){
            ml->list[ml->size++].m = GenPromoteStraight(from, to, KNIGHT);
            ml->list[ml->size++].m = GenPromoteStraight(from, to, BISHOP);
            ml->list[ml->size++].m = GenPromoteStraight(from, to, ROOK);
            ml->list[ml->size++].m = GenPromoteStraight(from, to, QUEEN);
        }else
            ml->list[ml->size++].m = GenOneForward(from, to);
    }
    if(side == WHITE) mv_bits = (((pc_bits << 8) & ~pos->occupied & Rank3BB) << 8) & temp;
    else mv_bits = (((pc_bits >> 8) & ~pos->occupied & Rank6BB) >> 8) & temp;
    while(mv_bits){
        to = popFirstBit(&mv_bits);
        if(side == WHITE) from = to - 16;
        else from = to + 16;
        ml->list[ml->size++].m = GenTwoForward(from, to);
    }
    pc_bits = pos->knights & friends & ~pinned;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = KnightMoves[from] & temp;
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenKnightMove(from, to, getPiece(pos, to));  
        }
    }
    pc_bits = pos->bishops & friends & ~pinned;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = bishopAttacksBB(from, pos->occupied) & temp; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenBishopMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->rooks & friends & ~pinned;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = rookAttacksBB(from, pos->occupied) & temp; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenRookMove(from, to, getPiece(pos, to));
        }
    }
    pc_bits = pos->queens & friends & ~pinned;
    while(pc_bits){
        from = popFirstBit(&pc_bits);
        mv_bits = queenAttacksBB(from, pos->occupied) & temp; 
        while(mv_bits){
            to = popFirstBit(&mv_bits);
            ml->list[ml->size++].m = GenQueenMove(from, to, getPiece(pos, to));
        }
    }
    if((pos->status->epsq != -1) && (checkers & pos->pawns & pos->color[xside])){
        to = pos->status->epsq;
        mv_bits = pos->pawns & pos->color[side] & PawnCaps[to][xside] & ~pinned;
        while(mv_bits){
            from = popFirstBit(&mv_bits);
            temp = pos->occupied ^ BitMask[sqchecker] ^ BitMask[from];
            if(((bishopAttacksBB(ksq, temp) & (pos->queens|pos->bishops) 
            & pos->color[xside]) == EmptyBoardBB) &&
            ((rookAttacksBB(ksq, temp) & (pos->queens|pos->rooks) 
            & pos->color[xside]) == EmptyBoardBB))
                ml->list[ml->size++].m = GenEnPassant(from, to);
        }
    }    
}    
