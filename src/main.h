/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

void quit(void) {
	fclose(logfile);
	fclose(errfile);
	fclose(dumpfile);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
	char command[4096];
	char line[4096];
	position_t pos;
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stdin, NULL, _IONBF, 0);
	Print(3, "Twisted Logic alpha by Edsel Apostol\n");
	Print(3, "Under development, use UCI commands\n");
	Print(3, "Type testloop for debugging/testing\n\n");
	fopen_s(&logfile, "logfile.txt", "a+");
	fopen_s(&errfile, "errfile.txt", "a+");
	fopen_s(&dumpfile, "dumpfile.txt", "a+");
	initArr();
	while (TRUE) {
		if (fgets(command, 4096, stdin) == NULL)
			strcpy_s(command, 5, "quit\n");
		Print(2, "%s\n", command);
		if (_strnicmp(command, "ucinewgame", 10) == 0) { /* trans_clear(); */ }
		else if (_strnicmp(command, "uci", 3) == 0) uciStart(&pos);
		else if (_strnicmp(command, "debug", 5) == 0) { /* dummy */ }
		else if (_strnicmp(command, "isready", 7) == 0) Print(3, "readyok\n");
		else if (_strnicmp(command, "position", 8) == 0)
			uciSetPosition(&pos, command + 9);
		else if (_strnicmp(command, "go", 2) == 0) uciGo(&pos, command + 3);
		else if (_strnicmp(command, "testloop", 8) == 0) nonUCI();
		else if (_strnicmp(command, "quit", 4) == 0) break;
	}
	quit();
}