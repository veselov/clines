/*
 * file: $Source$
 * author: $Author$
 * created: 2002/10/07
 * last modified: $Date$
 * version: $Revision$
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

