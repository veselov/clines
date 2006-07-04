AC_INIT(clines, 1.0.1-alpha, vps@manticore.2y.net)
AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([clines-config.h])

AC_ARG_WITH(curses, [AS_HELP_STRING(--with-curses=auto|curses|ncurses|ncursesw,
    Select curses variant to compile with)],
    [ucurses="$withval"], [ucurses="auto"])

fixed_curses=1

AC_MSG_CHECKING(Which curses library to use)

case "$ucurses" in
"no"|"none")
    AC_MSG_RESULT(none)
    AC_MSG_ERROR(Cannot compile without curses)
    ;;
""|"auto")
    fixed_curses=0
    ;;
ncursesw)
    curs_hdr="ncursesw/ncurses.h"
    ;;
ncurses)
    curs_hdr="ncurses/ncurses.h"
    ;;
curses)
    curs_hdr="curses.h"
    ;;
*)
    AC_MSG_RESULT(unknown)
    AC_MSG_ERROR(Unknown curses library specified : "$ucurses")
    ;;
esac

AC_MSG_RESULT($ucurses)
AC_CHECK_HEADERS(stdio.h)
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(strings.h)
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(unistd.h)
AC_HEADER_TIME
AC_CHECK_HEADERS(stdarg.h)

if test $fixed_curses = 1; then
    lcurses=$ucurses
    AC_CHECK_LIB($ucurses, initscr, [], [
        AC_MSG_ERROR(Specified curses library "$ucurses" not found)
    ])
else
    AC_CHECK_LIB(ncursesw, initscr, [lcurses="ncursesw"], [
        AC_CHECK_LIB(ncurses, initscr, [lcurses="ncurses"], [
            AC_CHECK_LIB(curses, initscr, [lcurses="curses"], [
                AC_MSG_ERROR(Curses library is required)
            ])
        ])
    ])
fi

if test $fixed_curses = 1; then
    hcurses=$ucurses
    AC_CHECK_HEADER($curs_hdr, [], [
        AC_MSG_ERROR(Specified curses headers for "$ucurses" not found)
    ])
else
    AC_CHECK_HEADER(ncursesw/ncurses.h, [hcurses=ncursesw], [
        AC_CHECK_HEADER(ncurses/ncurses.h, [hcurses=ncurses], [
            AC_CHECK_HEADER(curses.h, [hcurses=curses], [
                AC_MSG_ERROR(Curses headers not found)
            ])
        ])
    ])
fi

if test $lcurses != $hcurses; then
    AC_MSG_WARN(
You are using "$lcurses" library and "$hcurses" headers
This not may be at all compatible. Check your \$CFLAGS and ld configuration.
Or, alternatively, use "--with-curses=$hcurses" switch to configure.
The compilation or linkage will most likely fail otherwise.
)
fi

AC_CHECK_LIB($lcurses, mousemask, [AC_DEFINE(HAVE_CMOUSE, [], [Curses mouse support])], [])

AC_CHECK_LIB($lcurses, color_set, [], [
    AC_MSG_ERROR([
Your curses library is so puny it doesn't support colors.
Please switch to a better curses library. If you are compiling
on Solaris, one in /usr/xpg4 may be a lot better
    ])
])

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Compile with debug (disabled by default)]), [
    CFLAGS="$CFLAGS -g"
    debug_on=yes
], [debug_on=no])

AC_MSG_CHECKING([if debug code should be buit])
AC_MSG_RESULT([$debug_on])

case $hcurses in
ncursesw)
    AC_DEFINE(CURSES_NCURSESW, [], [Wide char support ncurses library])
    ;;
ncurses)
    AC_DEFINE(CURSES_NCURSES, [], [ncurses library])
    ;;
curses)
    AC_DEFINE(CURSES_CURSES, [], [curses library])
    ;;
*)
    AC_MSG_FAILURE([Internal error : incorrect curses headers var : "$hcurses"])
    ;;
esac

AC_SUBST(CFLAGS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT