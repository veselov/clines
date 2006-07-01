/*
 * file: main.h
 * author: Pawel S. Veselov
 * created: 2002/10/07
 * last modified: 02/10/10
 * version: 1.4
 */

#ifndef _CLINES_MAIN_H_
#define _CLINES_MAIN_H_

struct board;

#define	snew(x)	    (x*)malloc(sizeof(x))

void fatal(char * fmt, ...);
void quit(void);

void suspend_timer();
void resume_timer();

#define	mmin(a,b)   ((a)>(b)?(b):(a))

extern int score;

#endif

