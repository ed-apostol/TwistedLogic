/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef _WIN32
    #include <sys/time.h>
#else
    #include <windows.h>
    #include <sys/timeb.h>
#endif
#include "twisted.h"
#include "init.c"
#include "utils.c"
#include "bitutils.c"
#include "attacks.c"
#include "movegen.c"
#include "position.c"
#include "eval.c"
#include "search.c"
#include "tests.c"
#include "uci.c"
#include "main.c"

