/*
 * file: board.h
 * author: Pawel S. Veselov
 * created: 2002/10/06
 * last modified: 02/10/09
 * version: 1.5
 */

#ifndef _CLINES_BOARD_H_
#define _CLINES_BOARD_H_

typedef struct pset {
    int	    len;
    unsigned char * path;
} pset;

typedef struct board {
    int	w;
    int	h;
    int	mc; // max colors
    int	ml; // max length
    int	* board;
    int sel;	// selected (linear address)
    int av;	// available cells
    int nev;	// how much chips appear
		// each turn
    // rendering

    int s;
    int x;
    int y;

    int jst;    // regulate "jumping"

    pset * set;
    pset * rec;	// recent
    pset * path;    // path to move the ball.
    
} board;

void add(board *);

/*
 * before calling reset, set w and h
 */
void reset(board *);

#endif
