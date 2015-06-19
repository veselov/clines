/*
 * file: $RCSfile$
 * author: Pawel S. Veselov
 * created: 2002/10/07
 * last modified: $Date$
 * version: $Revision$
 */

#ifndef _CLINES_MAIN_H_
#define _CLINES_MAIN_H_

#define CSCORE_EXT  ".cscore"

struct board;

#define	snew(x)	    (x*)fmalloc(sizeof(x))

extern void fatal(char * fmt, ...);
extern void quit(void);

extern void suspend_timer();
extern void resume_timer();

extern void * fmalloc(size_t);
extern void * frealloc(void *,size_t);

#define DBG(a,b...) do { if (is_debug) { fprintf(debug_file, a "\n" , ## b); } } while(0)

#define	mmin(a,b)   ((a)>(b)?(b):(a))

extern int score;
extern int hi_score;
extern int my_hi_score;
extern char * hi_score_who;
extern int allow_hi_score;
extern int is_debug;
extern FILE * debug_file;

extern char * command_codes;
#define CC_LEFT     0
#define CC_DOWN     1
#define CC_UP       2
#define CC_RIGHT    3
#define CC_ACT      4

extern char * color_font;
#define CF_CURSOR       0
#define CF_CHIP         1
#define CF_CHIP_JUMP    2

extern int color_mode;
#define CM_AUTO     0
#define CM_BW       1
#define CM_COLOR    2

extern char * bw_font;
#define DEFAULT_BW_FONT     "*OoVvAa.'Zz"

extern int * chips_colors;

#ifdef HAVE_GPM
extern int has_gpm;
#endif

typedef int (*f_getch_t)(void);

#endif

