/*
 * file: $RCSfile$
 * author: Pawel S. Veselov
 * created: 2002/10/07
 * last modified: $Date$
 * version: $Revision$
 */

#include <clines/sysi.h>
#include <clines/board.h>
#include <clines/render.h>
#include <clines/main.h>
#include <clines/play.h>

#ifdef HAVE_GPM
static Gpm_Event _latest_gpm_event;
Gpm_Event * latest_gpm_event = &_latest_gpm_event;
#endif

void render(board * b) {

    int i;

    for (i=0; i<(b->h*b->w); i++) {
	render1(b, i, -1);
    }
}

void render1(board * b, int x, int y) {

    int lin;
    char c;
    
    int st_x, st_y, len;
    int i;
    int cpos = b->y * b->w + b->x;
    int cc;
    int shift = 0;

    if (y < 0) {
	lin = x;
	x = lin % b->w;
	y = lin / b->w;
    } else {
	lin = y * b->w + x;
    }

    len = b->s - 1;

    if (b->board[lin]) {
	if (lin == b->sel) {

	    switch(b->jst % 4) {
		case 1:
		    shift = 1;
		    break;
		case 3:
		    shift = -1;
		    break;
	    }
	}

	c = 'O';
	cc = ' ';
	// TODO : here should be color remap
	color_set(b->board[lin], NULL);
    } else {
	color_set(7, NULL);
	c = ' ';
	cc = 'I';
    }

    st_x = x * b->s + 1;
    st_y = y * b->s + 1;

    for (i=0; i<len; i++) {

	int dlt = 0;
	if (c != ' ') {
	    mvhline(st_y+i, st_x, ' ', len-dlt);
	}

	if (c != ' ') {
	    dlt = abs(len/2 - (i+shift));
	}
	
	mvhline(st_y+i, st_x+dlt, c, len-dlt*2);
	if ((cpos == lin && b->con)&&i&&(i<(len-1))) {
	    mvhline(st_y+i, st_x+1+dlt, cc, len-2-dlt*2);
	}
    }

    color_set(7, NULL);
    move(0,0);

#ifdef HAVE_GPM
    // must test for !b->con, that means that the text
    // cursor has been disabled, and mouse cursor should be
    // displayed, also since text cursor is disabled, at least
    // one mouse event has had happened !
    if (has_gpm && !b->con) {
        GPM_DRAWPOINTER(latest_gpm_event);
    }
#endif

}

void rinit(board * b) {

    int x,y;
#ifdef HAVE_CMOUSE
    mmask_t nis;
#endif

#ifdef HAVE_GPM
    Gpm_Connect gpm;
    int gpm_rc;
#endif

    initscr();
    start_color();
    noecho();
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, y, x);

    // fprintf(stderr, "screen size : %d:%d\n", x, y);

    if (x < b->w*4+1) {
	fatal("%d char wide terminal required\n", b->w*4+1);
    }

    if (y < b->h*4+1) {
	fatal("%d char tall terminal required\n", b->h*4+1);
    }

#ifdef HAVE_CMOUSE
    mousemask(BUTTON1_RELEASED, &nis);
    // timeout(GETCH_DELAY);
#endif

#ifdef HAVE_GPM
    gpm.minMod = gpm.maxMod = 0;
    gpm.eventMask = GPM_UP|GPM_SINGLE|GPM_MOVE;
    // gpm.defaultMask = GPM_MOVE|GPM_HARD;
    gpm.defaultMask = 0;
    has_gpm = (gpm_rc = Gpm_Open(&gpm, 0)) != -1;
    if (gpm_rc == -2) { // xterm ?? NOOO!
        Gpm_Close();
        has_gpm = 0;
    }
    if (has_gpm) {
        gpm_handler = my_gpm_handler;
        timeout(100);   // 10Hz selection
    }
#endif

    x = x/b->w;
    y = y/b->h;

    b->s = mmin(x, y);
    /*
     * should I do that ?
     * that would mean all squares need to be of odd number of chars
    if (b->s % 2) {
        b->s--;
    }
    */

    for (x=0; x<=b->mc; x++) {
	init_pair(x, x, 0);
    }

    init_pair(7, 7, 0);
}

void rborder(board *b) {

    int i;

    for (i=0; i<=b->w; i++) {
	mvvline(0, i*b->s, '|', b->s*b->h);
    }

    for (i=0; i<=b->h; i++) {
	mvhline(i*b->s, 1, '-', b->s*b->w-1);
    }
}

