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
static char get_jumping_code(int);
static void bw_font_checks_out(board *);

#ifdef HAVE_GPM
static Gpm_Event _latest_gpm_event;
Gpm_Event * latest_gpm_event = &_latest_gpm_event;
static TERMINAL * pre_gpm_term;
#endif

void render(board * b) {

    int i;

    rscore(b);

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

    color_set(1, NULL);
    move(0,0);

}

void rgo(board * b) {

    // to localize
    char * str = "Game over, press a key...";
    int len = strlen(str);

    WINDOW * win = newwin(3, len + 2, 1, 1);
    wborder(win, '|', '|', '-', '-', '+', '+', '+', '+');
    wmove(win, 1, 1);
    wprintw(win, "%s", str);

    doupdate();
    refresh();
    wrefresh(win);

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
            c = color_font[CF_CHIP];
        } else {
            c = get_char_code(b->board[lin]);
        }
    } else {
        if (r_has_color) {
            color_set(1, NULL);
        }
        c = ' ';
    }

    if (cpos == lin && b->con) {
        c = color_font[CF_CURSOR];
    }

    if (lin == b->sel) {    // the position is selected
        color_set(b->board[lin], NULL);
        if (box_len == 1) {
            if (b->jst % 8 < 4) {
                if (r_has_color) {
                    c = color_font[CF_CHIP_JUMP];
                } else {
                    c = get_jumping_code(b->board[lin]);
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
            c = color_font[CF_CHIP];
        } else {
            c = get_char_code(b->board[lin]);
        }
    } else {
	color_set(1, NULL);
	c = ' ';
	cc = color_font[CF_CURSOR];
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

void rfini() {
    int nis, y;
    getmaxyx(stdscr, y, nis);
    erase();
    refresh();
    doupdate();
    move(y-1, 0);

#ifdef HAVE_GPM
    if (has_gpm) {
        Gpm_Close();
    }

    if (pre_gpm_term != cur_term) {
        del_curterm(cur_term);
        set_curterm(pre_gpm_term);
    }
#endif

#ifndef NO_SAVE_TTY
    resetty();
#endif

    endwin();
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
        switch (color_mode) {
        case CM_AUTO:
            r_has_color = has_colors();
            break;
        case CM_BW:
            r_has_color = 0;
            break;
        case CM_COLOR:
            r_has_color = 1;
            break;
        default:
            fprintf(stderr, "color_mode is %d!\n", color_mode);
            abort();
            break;
        }
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

    if (first_time && !r_has_color && color_mode == CM_AUTO) {
        printw("Your terminal doesn't support colors !\n"
                "Try setting your terminal to xterm-color, cxterm, etc.\n"
                "Press ENTER to continue, Ctrl-C to quit...");
        getch();
        erase();
    }

    if (!r_has_color && first_time) {
        bw_font_checks_out(b);
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
        pre_gpm_term = cur_term;
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

    x = (x-1)/b->w;
    y = (y-1)/b->h;

    b->s = mmin(x, y);

    if (b->s<=3) {
        mini = 1;
    } else {
        mini = 0;
    }

    if (r_has_color) {

        init_pair(1, chips_colors[0], COLOR_BLACK);
        
        for (x=0; x<b->mc; x++) {
            init_pair(x+2, chips_colors[x+1], COLOR_BLACK);
        }
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
	mvvline(0, i*b->s, '|', b->s*b->h+1);
    }

    for (i=0; i<=b->h; i++) {
	mvhline(i*b->s, 1, '-', b->s*b->w-1);
    }
}

void rscore(board *b) {

    if (score) {
        if (mini) {
            move(0, 1);
            printw("[ s: %d ]", score);
        } else {
            move(0, 8);
            printw("=[ score : %d ]=", score);
        }
    }

    if (hi_score) {

        mvhline(b->s * b->h, 1, '-', b->s*b->w-1);

        if (mini) {
            move(b->h * b->s, 1);
            printw("[ hi : %d ]", hi_score);
        } else {

            move(b->h * b->s, 8);

            if (hi_score <= my_hi_score) {
                // it's just me then
                printw("=[ hi : %d ]=", my_hi_score);
            } else {
                printw("=[ hi -- %s:%d you:%d ]=",
                        hi_score_who, hi_score, my_hi_score);
            }
        }
    }
    
}

char get_jumping_code(int val) {
    // return bw_font[(val-1) * 2 + 1];
    return bw_font[(val-2) * 2 + 2];
}

char get_char_code(int val) {
    if (val == 1) { return bw_font[0]; }
    return bw_font[(val-2)*2+1];
}

void bw_font_checks_out(board * b) {

    if (!bw_font) {
        // no user spec
        if (b->mc > 5) {
            fatal("Custom B&W font must be specified along (use -F)\n");
        } else {
            bw_font = DEFAULT_BW_FONT;
        }
        return;
    }

    if (strlen(bw_font) < (b->mc * 2) + 1) {
        fatal("Not enough characters in B&W font, need %d\n", (b->mc*2)+1);
    }
}
