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

#define	snew(x)	    (x*)malloc(sizeof(x))

void fatal(char * fmt, ...);
void quit(void);

void suspend_timer();
void resume_timer();

#define	mmin(a,b)   ((a)>(b)?(b):(a))

extern int score;
extern int hi_score;
extern int my_hi_score;
extern char * hi_score_who;
extern int allow_hi_score;

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
// #define DEFAULT_BW_FONT     "*OVA.Zova'z"
#define DEFAULT_BW_FONT     "*OoVvAa.'Zz"

extern int * chips_colors;

#ifdef HAVE_GPM
extern int has_gpm;
#endif

typedef int (*f_getch_t)(void);

#endif

