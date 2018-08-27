/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

/* the recursive perft routine, it execute make_move on all moves*/
void perft(position_t *pos, u32 maxply, u64 nodesx[]){
    int move, incheck; 
    u64 pinned;
    movelist_t movelist;
    if(pos->ply+1 > maxply) return;
    if(pos->status->incheck) genEvasions(pos, &movelist);
    else genMoves(pos, &movelist);
    pinned = pinnedPieces(pos, pos->side);
    for(movelist.pos = 0; movelist.pos < movelist.size; movelist.pos++){ 
        move = movelist.list[movelist.pos].m;
        if(!moveIsLegal(pos, move, pinned)) continue; 
        makeMove(pos, move);
        perft(pos, maxply, nodesx);
        unmakeMove(pos); 
        nodesx[pos->ply+1]++;
    }   
}

/* the recursive perft divide routine, it execute make_move on all moves*/
int perftDivide(position_t *pos, u32 depth, u32 maxply){
    int move, incheck, x = 0, legal = 0; 
    u64 pinned;
    movelist_t movelist;
    if(depth > maxply) return 0;
    if(pos->status->incheck) genEvasions(pos, &movelist);
    else genMoves(pos, &movelist);
    pinned = pinnedPieces(pos, pos->side);
    for(movelist.pos = 0; movelist.pos < movelist.size; movelist.pos++){
        move = movelist.list[movelist.pos].m;
        if(!moveIsLegal(pos, move, pinned)) continue; 
        makeMove(pos, move);
        x += perftDivide(pos, depth + 1, maxply);
        unmakeMove(pos); 
        legal++;
    }   
    return legal + x;
}

/* this is the perft controller */
void runPerft(int max_depth){
    position_t pos;
    movelist_t ml;
    int depth, i, x;
    u64 nodes, time_start, nodesx[MAXPLY], duration;
    Print(1, "See the logfile.txt on the same directory to view\n");
    Print(1, "the detailed output after running.\n");
    for(x = 0; x < 6; x++){
    	setPosition(&pos, FenString[x]);        
        Print(3, "FEN %d = %s\n", x+1, FenString[x]);
    	displayBoard(&pos, 3);
    	genMoves(&pos, &ml);
    	for(depth = 1; depth <= max_depth; depth++) {     
            Print(3, "perft %d ", depth);          
            memset(nodesx, 0, sizeof(nodesx));
            time_start = getTime();
            perft(&pos, depth, nodesx);
            duration = getTime() - time_start;
            nodes = 0;
            if(duration == 0) duration = 1;
            for(i = 1; i <= depth; i++) nodes += nodesx[i];
            Print(3, "%llu\t\t", nodesx[depth]);
            Print(3, "[%llu ms - ", duration);
            Print(3, "%llu KNPS]\n", nodes/duration); 
    	}
    	Print(3, "\nDONE DOING PERFT ON FEN %d\n", x+1);
    	Print(3, "\n\n\n");
    }      
}

/* this is the perft divide controller */
/* the recursive perft divide routine, it execute make_move on all moves*/
void runPerftDivide(position_t *pos, u32 maxply){
    int move, incheck, x = 0, legal = 0, y = 0; 
    u64 pinned;
    movelist_t movelist;
    Print(1, "See the logfile.txt on the same directory to view\n");
    Print(1, "the detailed output after running.\n");
    displayBoard(pos, 3);
    genMoves(pos, &movelist);
    pinned = pinnedPieces(pos, pos->side);
    for(movelist.pos = 0; movelist.pos < movelist.size; movelist.pos++){
        move = movelist.list[movelist.pos].m;
        if(!moveIsLegal(pos, move, pinned)) continue;  
        makeMove(pos, move);
        y += x = perftDivide(pos, 2, maxply);
        unmakeMove(pos); 
        legal++;
        Print(3, "%d: %s %d\n", legal, move2Str(move), x);
    }   
    Print(3, "Perft divide %d = %d\n", maxply, y);
}

/* this are the non-uci commands */
void nonUCI(void){
    position_t pos;
    movelist_t ml;
    char command[256];
    char temp[256];
    int move, i, incheck;
    Print(3, "This is the main loop of the non-UCI commands\n");
    Print(3, "Used for testing and debugging purposes\n");
    Print(3, "Type help for commands\n");
    setPosition(&pos, FenString[0]);
    while(TRUE){
        Print(1, "Logic >>");
       	if(!fgets(command, 256, stdin)) break;
        if(command[0]=='\n') continue;
        sscanf(command, "%s", temp);
        if(!strcmp(temp, "new")){
            setPosition(&pos, FenString[0]);
        }else if(!strcmp(temp, "undo")){
            if(pos.status > &pos.undos[0]) unmakeMove(&pos);
        }else if(!strcmp(temp, "moves")){
            genMoves(&pos, &ml);
            Print(3, "pseudo-legal moves = %d:", ml.size);
            for(ml.pos = 0; ml.pos < ml.size; ml.pos++){
                if(!(ml.pos%12)) Print(3, "\n");
                Print(3, "%s ", move2Str(ml.list[ml.pos].m));
            }    
            Print(3, "\n\n");
        }else if(!strcmp(temp, "qmoves")){
            genQmoves(&pos, &ml);
            Print(3, "quiescent moves = %d:", ml.size);
            for(ml.pos = 0; ml.pos < ml.size; ml.pos++){
                if(!(ml.pos%12)) Print(3, "\n");
                Print(3, "%s ", move2Str(ml.list[ml.pos].m));
            }    
            Print(3, "\n\n");
        }else if(!strcmp(temp, "evasions")){
            genEvasions(&pos, &ml);
            Print(3, "evasion moves = %d:", ml.size);
            for(ml.pos = 0; ml.pos < ml.size; ml.pos++){
                if(!(ml.pos%12)) Print(3, "\n");
                Print(3, "%s ", move2Str(ml.list[ml.pos].m));
            }    
            Print(3, "\n\n");
        }else if(!strcmp(temp, "d")){
            displayBoard(&pos, 3);
        }else if(!strcmp(temp, "perft")){
            sscanf(command, "perft %d", &move);
            runPerft(move);
        }else if(!strcmp(temp, "divide")){
            sscanf(command, "divide %d", &move);
            runPerftDivide(&pos, move);
        }else if(!strcmp(temp, "search")){
            int rootPV[MAXPLY];
            sscanf(command, "search %d", &i);
            getBestMove(&pos, 0, 0, 0, 0, 0, 0, 0, 0, i, 0, 0, NULL, rootPV);
        }else if(!strcmp(temp, "quit")){
            break;
        }else if(!strcmp(temp, "help")){
            Print(3, "\nhelp - displays this texts\n");
            Print(3, "new - initialize to the starting position\n");
            Print(3, "d - displays the board\n");
            Print(3, "moves - displays all the pseudo-legal moves\n");
            Print(3, "qmoves - displays all the quiescent moves\n");
            Print(3, "evasions - displays all the evasion moves\n");
            Print(3, "search X - search the position for X plies\n");
            Print(3, "undo - undo the last move done\n");
            Print(3, "perft X - do a perft test from a set of test positions for depth X\n");
            Print(3, "divide X - displays perft results for every move for depth X\n");
            Print(3, "quit - quits the program\n");
            Print(3, "this are just the commands as of now\n");
            Print(3, "press any key to continue...\n");
            getch();
        }else{    
            if(pos.status->incheck) genEvasions(&pos, &ml);
            else genMoves(&pos, &ml);
            move = parseMove(&ml, command);
            if(move){
                if(!moveIsLegal(&pos, move, pinnedPieces(&pos, pos.side))) 
                    Print(3, "Illegal move: %s\n", command);    
                else makeMove(&pos, move); 
            }else Print(3, "Unknown command: %s\n", command);    
        }    
    }    
    Print(3, "Test loop is quitting, back to uci main loop\n\n");
}
