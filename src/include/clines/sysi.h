/*
 * file: $Source$
 * author: $Author$
 * created: 2002/10/06
 * last modified: $Date$
 * version: $Revision$
 */

#ifndef _CLINES_SYSI_H_
#define _CLINES_SYSI_H_

/*
 * All system includes go here.
 * This file has to be included first,
 * if included at all.
 */

#ifdef HAVE_CONFIG_H

#include "clines-config.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef CURSES_CURSES
#include <curses.h>
#endif

#ifdef CURSES_NCURSES
#ifdef NCURSES_DIRECT
#include <ncurses.h>
#else
#include <ncurses/ncurses.h>
#endif
#endif

#ifdef CURSES_NCURSESW
#include <ncursesw/ncurses.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(HAVE_TIME_H) && defined(HAVE_SYS_TIME_H) && defined(TIME_WITH_SYS_TIME)
#include <time.h>
#include <sys/time.h>
#else
#ifdef HAVE_TIME_H
#include <time.h>
#else
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#endif
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#else
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#endif

#endif
