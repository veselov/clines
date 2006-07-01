/*
 * file: board.c
 * author: Pawel S. Veselov
 * created: 2002/10/06
 * last modified: 02/10/08
 * version: 1.3
 */

#include <clines/sysi.h>
#include <clines/board.h>
#include <clines/render.h>

static int myrand(int);

void reset(board * b) {
    b->av = b->w * b->h;
    memset(b->board, 0, sizeof(int)*b->av);
    b->sel = -1;
}

void add(board * b) {

    int i;
    int p[b->nev];
    int * ptr = b->board;
    // int sz = b->w * b->h * sizeof(int);

    b->rec->len = 0;

    for (i=0; i<b->nev; i++) {

	int j;

	while (1) {

	    int ok = 1;

	    p[i] = myrand(b->mc * b->av);
	    for (j=0; j<i; j++) {
		if (p[i]/b->mc == p[j]/b->mc) {
		    ok--;
		    break;
		}
	    }

	    if (ok) { break; }
	}
    }

    for (i=0; i<b->av; i++) {
	int j;
	while ((*(ptr++)));
	ptr--;
	for (j=0; j<b->nev; j++) {
	    if (p[j]/b->mc == i) {
		*ptr = p[j] % b->mc + 1;
		render1(b, ptr - b->board, -1);
		b->rec->path[b->rec->len++] = (unsigned char)(ptr-b->board);
	    }
	}
	ptr++;
    }

    b->av -= b->nev;
}

int myrand(int lim) {
    // from linux rand(3)
    return (int) ((float)lim*rand()/(RAND_MAX+1.0));
}
