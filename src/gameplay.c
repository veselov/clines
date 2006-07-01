/*
 * file: gameplay.c
 * author: Pawel S. Veselov
 * created: 2002/10/06
 * last modified: 02/10/10
 * version: 1.7
 */

#include <clines/sysi.h>
#include <clines/board.h>
#include <clines/render.h>
#include <clines/play.h>
#include <clines/main.h>

typedef struct dir {
    int dx;
    int dy;
} dir;

typedef int (*m_check)(board *, int);
typedef int (*m_addr)(board *, int);

static int c_north(board *, int);
static int c_south(board *, int);
static int c_west(board *, int);
static int c_east(board *, int);

static int m_north(board *, int);
static int m_south(board *, int);
static int m_west(board *, int);
static int m_east(board *, int);

static m_check f_check[] = { c_north, c_south, c_west, c_east };
static m_addr f_addr[] = { m_north, m_south, m_west, m_east };

static void do_move(board *);
static int destroy(board *);
static int bclick(board *);
static int trymove(board *);

static int p_add(board *, int, int);

static void b_check(board *, dir, int);

void play(board * b) {

    b->x = 0;
    b->y = 0;
    b->jst = 0;

    while (1) {

	add(b);

	if (!b->av) {
	    break;
	}

	destroy(b);

	do {
	    do_move(b);
	} while (destroy(b));

	if (b->av < b->nev) {
	    break;
	}
    }
}

void do_move(board * b) {

    b->sel = -1;
    b->jst = 0;
    c_board = b;

    while (1) {

	int c;
	int cx = b->x;
	int cy = b->y;

	render1(b, cx, cy);

	c = getch();

	switch (c) {

	case KEY_DOWN:
	case 'j':
	    if (++b->y >= b->h) {
		// b->y = 0;
		b->y = b->h - 1;
	    }
	    break;

	case KEY_UP:
	case 'k':
	    if (--b->y < 0) {
		// b->y = b->h - 1;
		b->y = 0;
	    }
	    break;
	    
	case KEY_LEFT:
	case 'h':
	    if (--b->x < 0) {
		// b->x = b->w - 1;
		b->x = 0;
	    }
	    break;

	case KEY_RIGHT:
	case 'l':
	    if (++b->x >= b->w) {
		// b->x = 0;
		b->x = b->w - 1;
	    }
	    break;
	case KEY_MOUSE:
	    // TODO implement mouse events,
	    // need xterm to test
	    abort();
	    break;
	case ' ':
	    suspend_timer();
            // b->jst = 0; // stop "jumping"
	    if (bclick(b)) {
		// this is the only exit from function
		c_board = NULL;
		resume_timer(1);
		return;
	    }
	    resume_timer(1);
	    break;
	case 'q':
	    quit();
	case '':
	    clear();
	    rborder(b);
	    render(b);
	    break;
	}

	render1(b, cx, cy);
    }
}

#define HBIT	0x800
#define HMSK	~0x800

int destroy(board * b) {

    int i;
    int fnd = 0;
    
    int cf1 = 1;

    if (b->rec->len > 1) { // synth
	cf1 = 2;
    }
    
    for (i=0; i<b->rec->len; i++) {

	int pos = (int)b->rec->path[i];
	dir dr;

	dr.dx = 1;
	dr.dy = 0;
	b_check(b, dr, pos);

	dr.dx = 0;
	dr.dy = 1;
	b_check(b, dr, pos);

	dr.dx = 1;
	dr.dy = 1;
	b_check(b, dr, pos);

	dr.dx = -1;
	dr.dy = 1;
	b_check(b, dr, pos);

    }

    for (i=0; i<(b->w*b->h); i++) {
	if (b->board[i] & HBIT) {
	    b->board[i] = 7;
	    render1(b, i, -1);
	    fnd++;
	}
    }

    if (!fnd) { return 0; }

    refresh();
    doupdate();
    suspend_timer();
    usleep(600000);
    resume_timer();

    for (i=0; i<(b->w*b->h); i++) {

	if (b->board[i] == 7) {
	    b->board[i] = 0;
	    render1(b, i, -1);
	}
    }

    b->av += fnd;

    // calculate score

    fnd += fnd * (fnd - b->ml);
    score += fnd;

    return fnd;
    
}

void b_check(board * b, dir dr, int pos) {

    int len = 1;
    int x = pos % b->w;
    int y = pos / b->w;
    int vx = x;
    int vy = y;
    int i;

    int col = b->board[pos]&HMSK;

    for (i=0; i<2; i++) {

	while (1) {

	    vx += dr.dx;
	    vy += dr.dy;

	    if ((vx < 0)||(vy < 0)) { break; }

	    if ((vx == b->w)||(vy == b->h)) { break; }

	    if ((b->board[vy*b->w+vx]&HMSK) == col) {
		len++;
		continue;
	    }
	    break;
	}

	vx = x;
	vy = y;
	dr.dx = -dr.dx;
	dr.dy = -dr.dy;
    }

    if (len < b->ml) {
	return;
    }

    b->board[pos] |= HBIT;

    for (i=0; i<2; i++) {

	while (1) {

	    int nos;
	    vx += dr.dx;
	    vy += dr.dy;

	    nos = vy*b->w+vx;

	    if ((vx < 0)||(vy < 0)) { break; }

	    if ((vx == b->w)||(vy == b->h)) { break; }

	    if (!((b->board[nos]&HMSK) == col)) {
		break;
	    }

	    b->board[nos] |= HBIT;
	}

	vx = x;
	vy = y;
	dr.dx = -dr.dx;
	dr.dy = -dr.dy;
    }

}


/*
 * returns 1 if chip was moved
 */
int bclick(board * b) {

    int where = b->y * b->w + b->x;
    int olds = b->sel;

    if (b->board[where]) {  // we clicked on an occupied cell
	if (b->sel == where) { // the selected chip is clicked on again
	    b->sel = -1;    // reset selection
	} else {
	    b->sel = where; // set(change) selection
	}
        if (olds >= 0) {
            render1(b, olds, -1);
        }
	return 0;
    }

    if (b->sel<0) { // no selection ?
	return 0;
    }

    if (trymove(b)) {
	return 1;
    }

    /*
    b->sel = -1;
    render1(b, olds, -1);
    */
    return 0;
    
}

int trymove(board * b) {

    int target = b->y * b->w + b->x;
    int i;
    b->set->len = 0;
    b->path->len = 0;

    if (p_add(b, b->sel, target)) {
	// yes we can !
	
	int old = b->sel;
	int col = b->board[old];
	b->sel = -1;
	
	// b->board[target] = b->board[b->sel];

	for (i=b->path->len-1; i>=0; i--) {
	    b->board[old] = 0;
	    render1(b, old, -1);
	    old = b->path->path[i];
	    b->board[old] = col;
	    render1(b, old, -1);
	    refresh();
	    doupdate();
	    usleep(50000);
	}

	b->board[old] = 0;
	b->board[target] = col;
	
	render1(b, old, -1);
	render1(b, target, -1);
	b->rec->len = 1;
	b->rec->path[0] = (unsigned char)target;
	return 1;
    }
    beep();
    return 0;
}

int c_west(board * b, int n) {
    return n % b->w;
}

int c_east(board * b, int n) {
    return (n+1) % b->w;
}

int c_north(board * b, int n) {
    return n >= b->w;
}

int c_south(board * b, int n) {
    return (n/b->w)<(b->h-1);
}

int m_west(board * b, int n) {
    return n-1;
}

int m_east(board * b, int n) {
    return n+1;
}

int m_north(board * b, int n) {
    return n-b->w;
}

int m_south(board * b, int n) {
    return n+b->w;
}

#define GO_NORTH    0
#define	GO_SOUTH    1
#define	GO_WEST	    2
#define	GO_EAST	    3

int p_add(board * b, int no, int tgt) {

    int ngb[4];
    int nptr = 0;
    int i;

    // default : N S W E
    unsigned char rule[4];
    int rn = 0;

    int aux = no % b->w;
    int bux = tgt % b->w;

    // select priority in direction

    if (aux > bux) { // need west
	rule[rn++] = GO_WEST;
    } else if (aux < bux) {
	rule[rn++] = GO_EAST;
    }

    if (no > tgt) { // past, go north
	rule[rn++] = GO_NORTH;
    } else if (no < tgt) {
	rule[rn++] = GO_SOUTH;
    }

    for (i=0; i<4; i++) {

	if (memchr(rule, (unsigned char)i, rn)) {
	    continue;
	}

	rule[rn++] = (unsigned char)i;

    }

    // two bits set

    for (i=0; i<4; i++) {
	if (f_check[rule[i]](b, no)) {
	    int pt = f_addr[(int)rule[i]](b, no);
	    if (!b->board[pt]) {
		ngb[nptr++] = pt;
	    }
	}
    }

    for (i=0; i<nptr; i++) {
	// was it recorded ?
	if (memchr(b->set->path, ngb[i], b->set->len)) {
	    ngb[i] = -1;
	    continue;
	}

	if (ngb[i] == tgt) {
	    return 1;
	}

	b->set->path[b->set->len++] = ngb[i];
    }

    for (i=0; i<nptr; i++) {
	// was it recorded ?
	
	if (ngb[i] >= 0) {
	    if (p_add(b, ngb[i], tgt)) {
		b->path->path[b->path->len++] = ngb[i];
		return 1;
	    }
	}
    }
    return 0;
}

board * c_board = NULL;
