/*
 * file: $RCSfile$
 * author: Pawel S. Veselov
 * created: 2002/10/06
 * last modified: $Date$
 * version: $Revision$
 */

#ifndef _CLINES_BOARD_H_
#define _CLINES_BOARD_H_

typedef struct pset {
    int	len;
    int * path;
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
    int first_nev;  // how many chips to start with
    int played;     // TRUE if at chips were casted
    // rendering

    int s;
    int x;
    int y;

    int jst;    // regulate "jumping"

    int con;    // cursor is visible?

    pset * set;
    pset * rec;     // recent. This is used to check which chips need to
                    // be tracked to check for completed lines. So either
                    // this would be the location of the moved chip, or
                    // all the locations of chips placed by the computer.
                    // path[] conatins linear addresses then

    pset * path;    // path to move the ball.
    
} board;

int add(board *);

/*
 * before calling reset, set w and h
 */
void reset(board *);

#endif
