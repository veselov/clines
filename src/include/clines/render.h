/*
 * file: $RCSfile$
 * author: Pawel S. Veselov
 * created: 2002/10/07
 * last modified: $Date$
 * version: $Revision$
 */

#ifndef _CLINES_RENDER_H_
#define _CLINES_RENDER_H_

// #define GETCH_DELAY 200

struct board;

void render(struct board *);
void rinit(struct board *, int);
void rfini();
void rborder(struct board *);
void render1(struct board *, int, int);
void rscore(struct board *);
void rgo(struct board *);

#ifdef HAVE_GPM
extern Gpm_Event * latest_gpm_event;
#endif

extern int r_can_render;
extern int r_has_color;

#endif
