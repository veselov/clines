/*
 * file: $RCSfile$
 * author: Pawel S. Veselov
 * created: 2002/10/06
 * last modified: $Date$
 * version: $Revision$
 */

#include <clines/sysi.h>
#include <clines/main.h>
#include <clines/board.h>
#include <clines/render.h>
#include <clines/play.h>

static void mysig(int);
static void my_quit(void);

static struct itimerval timer = {
    { 0, 200000 },
    { 0, 200000 } 
};

static struct itimerval stoptimer = {
    { 0, 0 },
    { 0, 0 }
};

int main(int argc, char ** argv) {

    board * game;

#ifndef NO_SAVE_TTY
    savetty();
#endif

    signal(SIGQUIT, mysig);
    signal(SIGINT, mysig);
    signal(SIGALRM, mysig);
    resume_timer();

    game = snew(board);
    
    game->w = 9;
    game->h = 9;
    game->nev = 3;
    game->mc = 5;
    game->ml = 5;

    srand(time(NULL));

    game->board = (int*)malloc(game->w*game->h*sizeof(int));
    game->set = snew(pset);
    game->rec = snew(pset);
    game->path = snew(pset);
    game->rec->path = (unsigned char*)malloc(3);
    game->set->path = (unsigned char*)malloc(game->w*game->h);
    game->path->path = (unsigned char*)malloc(game->w*game->h);

    reset(game);
    rinit(game);
    rborder(game);

    score = 0;
    play(game);

    getch();

    quit();

    return 0;
}

void mysig(int s) {

    if (s == SIGALRM) {
	if (c_board) {
	    if (c_board->sel >= 0) {
		c_board->jst++;
		render1(c_board, c_board->sel, -1);
		refresh();
		doupdate();
	    }
	}
	signal(s, mysig);
	return;
    }
    
    endwin();
    clear();
    exit(0);
}

void fatal(char * fmt, ...) {

    va_list argp;

    my_quit();

    va_start(argp, fmt);
    vfprintf(stderr, fmt, argp);
    va_end(argp);

    exit(1);
}

void quit() {
    my_quit();
    fflush(stdout);
    fprintf(stdout, "Scores earned : %d\n", score);
    fflush(stdout);
    exit(0);
}

void my_quit() {

    int nis, y;
    getmaxyx(stdscr, y, nis);
    erase();
    refresh();
    doupdate();
    move(y-1, 0);
#ifndef NO_SAVE_TTY
    resetty();
#endif
    endwin();
}

void resume_timer() {
    if (!timer.it_value.tv_usec) {
	timer.it_value.tv_usec++;
    }
    setitimer(ITIMER_REAL, &timer, NULL);
}

void suspend_timer() {
    setitimer(ITIMER_REAL, &stoptimer, &timer);
}

int score;
