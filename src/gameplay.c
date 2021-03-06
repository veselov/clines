/*
 * file: $RCSfile$
 * author: Pawel S. Veselov
 * created: 2002/10/06
 * last modified: $Date$
 * version: $Revision$
 */

#include "clines/sysi.h"
#include "clines/board.h"
#include "clines/render.h"
#include "clines/play.h"
#include "clines/main.h"

#ifndef KEY_MOUSE
#define KEY_MOUSE 0631
#endif


#define OP_NONE     0
#define OP_LEFT     1
#define OP_RIGHT    2
#define OP_DOWN     3
#define OP_UP       4
#define OP_MOUSE    5
#define OP_ACT      6
#define OP_QUIT     7
#define OP_CLS      8

#ifdef HAVE_MONOTONIC_RAW
uint64_t currentms() {

    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

    return ((uint64_t)tp.tv_sec) * 1000ULL +
        tp.tv_nsec / 1000000ULL;

}
#endif

typedef struct direc {
    int dx;
    int dy;
} direc;

typedef int (*m_check)(board *, int);
typedef int (*m_addr)(board *, int);

// remain outside of #ifdef, to not overcomplicate
// preprocessor conditions around mouse event recognition
int has_gpm = 0;

static int scored_moves_count = 0;

#ifdef HAVE_GPM
static int has_gpm_event = 0;
static int gpm_event_x;
static int gpm_event_y;
static int gpm_getch_bridge(void);
#endif

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

static void b_check(board *, direc, int);

void play(board * b) {

    b->x = 0;
    b->y = 0;
    b->jst = 0;

    while (1) {

	if (!add(b)) {
            break;
        }

        scored_moves_count = 0;

	destroy(b);

        if (!b->av) { break; }

	while (1) {
	    do_move(b);
            if (!destroy(b)) {
                break;
            }
            scored_moves_count++;
	}
    }

    rgo(b);

}

void do_move(board * b) {

    f_getch_t f_getch;

#ifdef HAVE_CMOUSE
    MEVENT mevt;
#endif

#ifdef HAVE_GPM
    if (has_gpm) {
        f_getch = gpm_getch_bridge;
    } else {
#endif
        f_getch = getch;
#ifdef HAVE_GPM
    }
#endif

    b->sel = -1;
    b->jst = 0;
    c_board = b;

    while (1) {

	int c;
	int cx = b->x;
	int cy = b->y;
        int op;
        
        if (!r_can_render) {
            pause();
            continue;
        }

	render1(b, cx, cy);

	if ((c = f_getch())==ERR) { continue; }

        op = OP_NONE;

        if (c != KEY_MOUSE) {
            b->con = 1;
        } else {
            op = OP_MOUSE;
        }

        if (c == KEY_DOWN || c == command_codes[CC_DOWN]) {
            op = OP_DOWN;
        } else if (c == KEY_UP || c == command_codes[CC_UP]) {
            op = OP_UP;
        } else if (c == KEY_LEFT || c == command_codes[CC_LEFT]) {
            op = OP_LEFT;
        } else if (c == KEY_RIGHT || c == command_codes[CC_RIGHT]) {
            op = OP_RIGHT;
        } else if (c == command_codes[CC_ACT] && op == OP_NONE) {
            op = OP_ACT;
        } else if (c == 'q') {
            op = OP_QUIT;
        } else if (c == '') {
            op = OP_CLS;
        }

	switch (op) {
        case OP_DOWN:
	    if (++b->y >= b->h) {
		// b->y = 0;
		b->y = b->h - 1;
	    }
	    break;

        case OP_UP:
	    if (--b->y < 0) {
		// b->y = b->h - 1;
		b->y = 0;
	    }
	    break;
	    
        case OP_LEFT:
	    if (--b->x < 0) {
		// b->x = b->w - 1;
		b->x = 0;
	    }
	    break;

        case OP_RIGHT:
	    if (++b->x >= b->w) {
		// b->x = 0;
		b->x = b->w - 1;
	    }
	    break;
#if defined(HAVE_CMOUSE) || defined(HAVE_GPM)
        case OP_MOUSE:
            {
                int has_event = 0;
                int bx=0,by=0;
#ifdef HAVE_CMOUSE
                if (!has_gpm) {
                    int gmr = getmouse(&mevt);
                    if (gmr != OK || !(mevt.bstate & BUTTON1_RELEASED)) {
                        continue;
                    }
                    bx = mevt.x;
                    by = mevt.y;
                    has_event = 1;
                }
#endif  /* HAVE_CMOUSE */

#ifdef HAVE_GPM
                if (has_gpm && has_gpm_event) {
                    bx = gpm_event_x;
                    by = gpm_event_y;
                    has_event = 1;
                    has_gpm_event = 0;
                }
#endif  /* HAVE_GPM */

                // failed to harvest any valid mouse event ?
                if (!has_event) { continue; }
                
                // did we hit on the grid ?
                if (!(bx % b->s) || !(by % b->s)) {
                    continue;
                }

                bx /= b->s;
                by /= b->s;

                // out of board ?
                if (bx >= b->w || by >= b->h) {
                    continue;
                }

                b->x = bx;
                b->y = by;

                b->con = 0;
            }
#endif  /* HAVE_CMOUSE || HAVE_GPM */
        case OP_ACT:
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
        case OP_QUIT:
	    quit();
        case OP_CLS:
	    clear();
	    rborder(b);
	    render(b);
	    break;
	}

	render1(b, cx, cy);
    }
}

#define HBIT	0x8000
#define HMSK	~0x8000

int destroy(board * b) {

    int i;
    int fnd = 0;
    
    for (i=0; i<b->rec->len; i++) {

	int pos = (int)b->rec->path[i];
	direc dr;

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
	    b->board[i] = 1;
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

	if (b->board[i] == 1) {
	    b->board[i] = 0;
	    render1(b, i, -1);
	}
    }

    b->av += fnd;

    // calculate score

    fnd += fnd * (fnd - b->ml);
    score += (fnd + scored_moves_count * scored_moves_count);
    if (allow_hi_score) {
        if (score > my_hi_score) {
            my_hi_score = score;
        }
        if (score > hi_score) {
            hi_score = score;
        }
    }

    rscore(b);

    return fnd;
}

void b_check(board * b, direc dr, int pos) {

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
	b->rec->path[0] = target;
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
#define	ALL_DIRS    0xf

int p_add(board * b, int no, int tgt) {

    struct memory {
        int dirs;
        int coordinate;
    };

    // we won't have more moves than the size of the board.
    struct memory stack[b->w*b->h];
    int stack_next = 1;
    char marked[b->w*b->h];
    int cost[b->w*b->h];
    int distance;
    int tgtY = tgt / b->w;
    int aux;

    int hits = 0;
    int cuts = 0;
    int cost_cuts = 0;
    int cost_adj = 0;
    int pushes = 0;
#ifdef HAVE_MONOTONIC_RAW
    uint64_t time = currentms();
#endif

    pset route;

    route.len = 0;
    route.path = 0;

    memset(marked, 0, b->w*b->h);
    memset(cost, 0, b->w*b->h*sizeof(int));

    stack[0].dirs = 0;
    stack[0].coordinate = no;
    marked[no] = 1;

    // minimal distance between the two points.

    distance = tgt / b->w - no / b->w;
    if (distance < 0) { distance = -distance; }
    aux = tgt%b->w - no%b->w;
    if (aux < 0) { aux = -aux; }
    distance += aux;

    while (1) {

        if (stack_next <= 0) { break; }

        struct memory * cur = &stack[stack_next-1];
        // where can we go next
        int dir = cur->dirs;
        int loc = cur->coordinate;
        int locY = loc / b->w;
        if (dir == ALL_DIRS) {
            marked[loc] = 0;
            stack_next--;
            continue;
        }

        if (locY < tgtY && !(dir & (1<<GO_SOUTH))) {
            dir = GO_SOUTH;
        } else if (locY > tgtY && !(dir & (1<<GO_NORTH))) {
            dir = GO_NORTH;
        } else if (loc < tgt && !(dir & (1<<GO_EAST))) {
            dir = GO_EAST;
        } else if (loc > tgt && !(dir & (1<<GO_WEST))) {
            dir = GO_WEST;
        } else {
            int next_dir = GO_NORTH;
            while (dir & 1) {
                dir>>=1;
                next_dir++;
            }
            dir = next_dir;
        }

        cur->dirs |= (1<<dir);

        if (f_check[dir](b, loc)) {

            int loc2 = loc;

            loc = f_addr[dir](b, loc);

            // printf("%d, dir %d, going to %d\n\r", loc2, dir, loc);

            if (!b->board[loc] && !marked[loc]) {
                // we can go there!

                // but is it the target?
                if (loc == tgt) {
                    hits++;
                    // fprintf(stderr, "location hit, len:%d, best len: %d, distance:%d\n\r", stack_next, route.len, distance);
                    // we got ourselves a valid path,
                    // let's save it.
                    if (!route.len || route.len > stack_next) {
                        int i,j;
                        if (!route.len) {
                            route.path = fmalloc(stack_next * sizeof(int));
                        }
                        route.len = stack_next;
                        for (i=1,j=stack_next-1; i<stack_next; i++,j--) {
                            route.path[j] = stack[i].coordinate;
                        }
                        route.path[0] = loc;
                        // if the path that we found is minimal distance
                        // then we don't need to search further.
                        if (route.len == distance) { break; }
                    }
                } else {

                    // if we have found a solution, and we know
                    // we won't be able to improve from this point on,
                    // then let's not even try.
                    if (!route.len || stack_next < route.len) {

                        // now, let's check/establish the cost.
                        if (!cost[loc] || stack_next < cost[loc]) {
                            if (cost[loc]) {
                                cost_adj++;
                            }
                            cost[loc] = stack_next;
                            pushes++;
                            cur = &stack[stack_next++];
                            cur->dirs = 0;
                            cur->coordinate = loc;
                            marked[loc] = 1;
                        } else {
                            cost_cuts++;
                        }

                    } else {
                        cuts++;
                    }
                }
            }
        }
    }

    DBG("From %d to %d : %d hits, %d cuts, %d cost cuts, %d cust adj, %d pushes, %sfound"
#ifdef HAVE_MONOTONIC_RAW
            " took %llums"
#endif
            
            ,
            no, tgt, hits, cuts, cost_cuts, cost_adj, pushes, route.len?"":"NOT "

#ifdef HAVE_MONOTONIC_RAW
            ,currentms() - time
#endif
            
            );

    if (route.len) {
        b->path->len = route.len;
        memcpy(b->path->path, route.path, sizeof(int) * route.len);
        free(route.path);
        return 1;
    }

    return 0;

}

board * c_board = NULL;

#ifdef HAVE_GPM

int my_gpm_handler(Gpm_Event * evt, void * nothing) {
    // only interested in left mouse button release

    memcpy(latest_gpm_event, evt, sizeof(Gpm_Event));
    GPM_DRAWPOINTER(latest_gpm_event);
    
    if (!(evt->type & GPM_UP) ||
            !(evt->buttons & GPM_B_LEFT)) { return 0; }
    gpm_event_x = evt->x - 1;
    gpm_event_y = evt->y - 1;
    has_gpm_event = 1;
    return KEY_MOUSE;
}

int gpm_getch_bridge() {
    return Gpm_Getch();
}

#endif

