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

static int myrand(int);

void reset(board * b) {
    b->av = b->w * b->h;
    memset(b->board, 0, sizeof(int)*b->av);
    b->sel = -1;
    b->con = 1;
}

int add(board * b) {

    int i;
    int nev = b->played?b->nev:b->first_nev;
    int p[nev];
    int * ptr = b->board;

    b->rec->len = 0;
    b->played = 1;

    if (b->av < nev) {
        return 0;
    }

    for (i=0; i<nev; i++) {

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
	for (j=0; j<nev; j++) {
	    if (p[j]/b->mc == i) {
		*ptr = p[j] % b->mc + 2;
		render1(b, ptr - b->board, -1);
		b->rec->path[b->rec->len++] = (int)(ptr-b->board);
	    }
	}
	ptr++;
    }

    b->av -= nev;
    return nev+1;
}

int myrand(int lim) {
    // from linux rand(3) manual page
    return (int) ((float)lim*rand()/(RAND_MAX+1.0));
}
