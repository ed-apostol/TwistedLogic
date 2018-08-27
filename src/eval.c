/**************************************************/
/*  Name: Twisted Logic Chess Engine              */
/*  Copyright: 2007                               */
/*  Author: Edsel Apostol                         */
/*  Contact: ed_apostol@yahoo.com                 */
/*  Description: A chess playing program.         */
/**************************************************/

int eval(const position_t *pos){
    int open_score[2], end_score[2];
    int open = 0, end = 0, score = 0;
    int phase;

    open_score[0] = open_score[1] = end_score[0] = end_score[1] = 0;

    open += pos->status->open[pos->side] - pos->status->open[pos->side^1];
    end += pos->status->end[pos->side] - pos->status->end[pos->side^1];
    open += open_score[pos->side] - open_score[pos->side^1];
    end += end_score[pos->side] - end_score[pos->side^1];
    phase = pos->status->phase;

    score = ((open * phase) + (end * (24 - phase))) / 24;
    return score;
}
