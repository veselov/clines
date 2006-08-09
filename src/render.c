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

int r_can_render = 1;
int r_has_color;
static int mini = 0;
static void render1_mini(board *, int, int);
static void render1_full(board *, int, int);
static char get_char_code(int);
static char get_jumping_code(char);

#ifdef HAVE_GPM
static Gpm_Event _latest_gpm_event;
Gpm_Event * latest_gpm_event = &_latest_gpm_event;
#endif

void render(board * b) {

    int i;

    rscore();

    for (i=0; i<(b->h*b->w); i++) {
	render1(b, i, -1);
    }
}

void render1(board * b, int x, int y) {
    if (mini) {
        render1_mini(b, x, y);
    } else {
        render1_full(b, x, y);
    }

    color_set(7, NULL);
    move(0,0);

}

void render1_mini(board * b, int x, int y) {

    int lin;
    int cpos = b->y * b->w + b->x;
    int box_len = b->s - 1;
    char c;
    int no_draw = -1;
    int i;
    
    if (y < 0) {
	lin = x;
	x = lin % b->w;
	y = lin / b->w;
    } else {
	lin = y * b->w + x;
    }

    if (b->board[lin]) {
        // TODO : remap the color
        if (r_has_color) {
            color_set(b->board[lin], NULL);
            c = 'O';
        } else {
            c = get_char_code(b->board[lin]);
        }
    } else {
        if (r_has_color) {
            color_set(7, NULL);
        }
        c = ' ';
    }

    if (cpos == lin && b->con) {
        c = 'I';
    }

    if (lin == b->sel) {    // the position is selected
        color_set(b->board[lin], NULL);
        if (box_len == 1) {
            if (b->jst % 8 < 4) {
                if (r_has_color) {
                    c = tolower(c);
                } else {
                    c = get_jumping_code(c);
                }
            }
        } else {
            int _p = b->jst % 12;
            if (_p < 3) {
                no_draw = 0;
            } else if (_p >= 6 && _p < 9) {
                no_draw = 1;
            }
        }
    }

    for (i = 0; i<box_len; i++) {
        char dc = (i==no_draw)?' ':c;
        mvhline(y*b->s+1+i, x*b->s+1, dc, box_len);
    }
}

void render1_full(board * b, int x, int y) {

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

	cc = ' ';
        if (r_has_color) {
            // TODO : here should be color remap
            color_set(b->board[lin], NULL);
            c = 'O';
        } else {
            c = get_char_code(b->board[lin]);
        }
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
	    dlt = abs(len/2 - (i+shift));
	}

	mvhline(st_y+i, st_x+dlt, c, len-dlt*2);
	if ((cpos == lin && b->con)&&i&&(i<(len-1))) {
	    mvhline(st_y+i, st_x+1+dlt, cc, len-2-dlt*2);
	}
    }

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

void rinit(board * b, int first_time) {

    int x,y;
    int bad = 0;
#ifdef HAVE_CMOUSE
    mmask_t nis;
#endif

#ifdef HAVE_GPM
    Gpm_Connect gpm;
    int gpm_rc;
#endif

    r_can_render = 1;

    if (!first_time) {
        suspend_timer();
        endwin();
        doupdate();
    }

    initscr();
    erase();

    if (first_time) {
        r_has_color = has_colors();
    }

    noecho();
    keypad(stdscr, TRUE);

    getmaxyx(stdscr, y, x);

    // fprintf(stderr, "screen size : %d:%d\n", x, y);
    
    move(0,0);

    if (x < b->w*2+1) {
        if (first_time) {
            fatal("%d chars wide terminal required, now it's %d\n",
                    b->w*2+1, x);
        }
        printw("%d chars wide terminal required, now it's %d\n",
                b->w*2+1, x);
        bad++;
    }

    if (y < b->h*2+1) {
        if (first_time) {
            fatal("%d chars tall terminal required, now it's %d\n",
                    b->h*2+1, y);
        }
        printw("%d chars tall terminal required, now it's %d\n",
                b->h*2+1, y);
        bad++;
    }

    if (first_time && !r_has_color) {
        printw("Your terminal doesn't support colors !\n"
                "Try setting your terminal to xterm-color, cxterm, etc.\n"
                "Press ENTER to continue, Ctrl-C to quit...");
        getch();
        erase();
    }

    if (bad) {
        printw("Please resize the window again");
        doupdate();
        refresh();
        r_can_render = 0;
        return;
    }

    if (r_has_color) {
        start_color();
    }

#ifdef HAVE_CMOUSE
    mousemask(BUTTON1_RELEASED, &nis);
    // timeout(GETCH_DELAY);
#endif

#ifdef HAVE_GPM
    if (first_time) {
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
    }
#endif

    x = x/b->w;
    y = y/b->h;

    b->s = mmin(x, y);

    if (b->s<=3) {
        mini = 1;
    } else {
        mini = 0;
    }

    if (r_has_color) {
        for (x=0; x<=b->mc; x++) {
            init_pair(x, x, 0);
        }

        init_pair(7, 7, 0);
    }

    if (!first_time) {

        rborder(b);
        render(b);
        doupdate();
        refresh();

        resume_timer();
    }
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

void rscore() {

    if (!score) {
        return;
    }

    if (mini) {
        move(0, 1);
        printw("[ s: %d ]", score);
    } else {
        move(0, 8);
        printw("=[ score : %d ]=", score);
    }
}

char get_jumping_code(char val) {
    switch (val) {
        case 'O':
            return 'o';
        case 'V':
            return 'v';
        case 'A':
            return 'a';
        case '.':
            return '\'';
        case 'Z':
            return 'z';
    }

    fatal("oops! get_jumping_code()val=%d, report to developer\n", (int)val);
    return 0; // make gcc happy
}

char get_char_code(int val) {

    switch (val) {
        case 1:
            return 'O';
        case 2:
            return 'V';
        case 3:
            return 'A';
        case 4:
            return '.';
        case 5:
            return 'Z';
        case 7:
            return '*';
    }

    fatal("oops! get_char_code()val=%d, report to developer\n", val);
    return 0; // make gcc happy
}
