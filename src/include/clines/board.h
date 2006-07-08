/*
 * file: $Source$
 * author: $Author$
 * created: 2002/10/06
 * last modified: $Date$
 * version: $Revision$
 */

#ifndef _CLINES_BOARD_H_
#define _CLINES_BOARD_H_

typedef struct pset {
    int	    len;
    unsigned char * path;
} pset;

typedef struct board {
    int	w;  // gee, board's width
    int	h;  // and height
    int	mc; // max colors
    int	ml; // max length
    int	* board;    // state matrix
    int sel;	// selected (linear address)
    int av;	// available cells
    int nev;	// how much chips appear
		// each turn
    // rendering

    int s;
    int x;
    int y;

    int jst;    // regulate "jumping"

    int con;    // cursor is visible?

    pset * set;
    pset * rec;     // recent
    pset * path;    // path to move the ball.
    
} board;

void add(board *);

/*
 * before calling reset, set w and h
 */
void reset(board *);

#endif
