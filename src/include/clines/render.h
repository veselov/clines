/*
 * file: render.h
 * author: Pawel S. Veselov
 * created: 2002/10/07
 * last modified: 02/10/07
 * version: 1.2
 */

#ifndef _CLINES_RENDER_H_
#define _CLINES_RENDER_H_

struct board;

void render(struct board *);
void rinit(struct board *);
void rborder(struct board *);
void render1(struct board *, int, int);

#endif
