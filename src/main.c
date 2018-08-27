/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void quit(void){
    fclose(logfile);
	fclose(errfile);
	fclose(dumpfile);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    char command[4096];
    char line[4096];
    position_t pos;
    setbuf(stdout, NULL);
    setbuf(stdin, NULL);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    Print(3, "Twisted Logic alpha by Edsel Apostol\n");
    Print(3, "Under development, use UCI commands\n");
    Print(3, "Type testloop for debugging/testing\n\n");
    logfile = fopen("logfile.txt", "a+");
    errfile = fopen("errfile.txt", "a+");
    dumpfile = fopen("dumpfile.txt", "a+");
    initArr();
    while(TRUE){
        if(fgets(command, 4096, stdin) == NULL)
            strcpy(command, "quit\n");
        Print(2, "%s\n", command);
        if(strncasecmp(command, "ucinewgame", 10) == 0){ /* trans_clear(); */ }
        else if(strncasecmp(command, "uci", 3) == 0) uciStart(&pos);
        else if(strncasecmp(command, "debug", 5) == 0){ /* dummy */}
        else if(strncasecmp(command, "isready", 7) == 0) Print(3, "readyok\n");
        else if(strncasecmp(command, "position", 8) == 0)
            uciSetPosition(&pos, command + 9);
        else if(strncasecmp(command, "go", 2) == 0) uciGo(&pos, command + 3);
        else if(strncasecmp(command, "testloop", 8) == 0) nonUCI();
        else if(strncasecmp(command, "quit", 4) == 0) break;
    }
    quit();
}
