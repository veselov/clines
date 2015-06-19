AC_INIT(clines, 1.0.5, Pawel.Veselov@gmail.com)
AC_PREREQ(2.61)
AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(clines-config.h)

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

AC_CHECK_FUNCS([bzero],[],[ AC_MSG_FAILURE([need bzero]) ])
AC_CHECK_FUNCS([memset],[],[AC_MSG_FAILURE([need memset])])
AC_CHECK_FUNCS([strchr],[],[AC_MSG_FAILURE([need strchr])])
AC_CHECK_FUNCS([strdup],[],[AC_MSG_FAILURE([need strdup])])
AC_CHECK_FUNCS([strtol],[],[AC_MSG_FAILURE([need strtol])])
# AC_FUNC_MALLOC([],[AC_MSG_FAILURE([need malloc])])
# AC_FUNC_REALLOC([],[AC_MSG_FAILURE([need realloc])])
AC_TYPE_SIZE_T([],[AC_MSG_FAILURE([need size_t])])
AC_TYPE_UID_T([],[AC_MSG_FAILURE([need uid_t])])

AC_CHECK_HEADERS(stdio.h)
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(strings.h)
AC_CHECK_HEADERS(sys/stat.h)
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(sys/types.h)
AC_CHECK_HEADERS(netinet/in.h)
AC_CHECK_HEADERS(dirent.h)
AC_CHECK_HEADERS(term.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(ctype.h)
AC_CHECK_HEADERS(fcntl.h)
AC_CHECK_HEADERS(pwd.h)
AC_HEADER_TIME
AC_CHECK_HEADERS(stdarg.h)
AC_CHECK_HEADERS(gpm.h)
AC_SEARCH_LIBS([clock_gettime],[rt posix4])
AC_CHECK_FUNCS([clock_gettime])


AC_MSG_CHECKING([Checking for CLOCK_MONOTONIC_RAW])
AC_TRY_COMPILE([
                #include <time.h>
                ], [
                        struct timespec tp;
                        uint64_t want64;
                        clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

                    ],[AC_MSG_RESULT([yes]) AC_DEFINE(HAVE_MONOTONIC_RAW,[],[Have raw monotonic clock])],[AC_MSG_RESULT([no])]);

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

ncurses_direct=0

if test $fixed_curses = 1; then
    hcurses=$ucurses
    AC_CHECK_HEADER($curs_hdr, [], [
        AC_MSG_ERROR(Specified curses headers for "$ucurses" not found)
    ])
else
    AC_CHECK_HEADER(ncursesw/ncurses.h, [hcurses=ncursesw], [
        AC_CHECK_HEADER(ncurses/ncurses.h, [hcurses=ncurses], [
            AC_CHECK_HEADER(ncurses.h, [hcurses=ncurses;ncurses_direct=1], [
                AC_CHECK_HEADER(curses.h, [hcurses=curses], [
                    AC_MSG_ERROR(Curses headers not found)
                ])
            ])
        ])
    ])
fi

AC_CHECK_LIB(gpm, Gpm_Open, [], [])

if test $ac_cv_lib_gpm_Gpm_Open = yes; then
    AC_DEFINE(HAVE_GPM, [], [Have gpm support])
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

AC_MSG_CHECKING([if we are using standard curses on Solaris])
if test `uname` = SunOS -a $lcurses = curses; then
    AC_MSG_RESULT([yes, trying xpg4])
    LDFLAGS="-L/usr/xpg4/lib -L/usr/xpg4/lib/sparcv9 -R/usr/xpg4/lib/sparcv9 -R/usr/xpg4/lib $LDFLAGS"
    CFLAGS="-I/usr/xpg4/include $CFLAGS"
    AC_DEFINE(NO_SAVE_TTY, [], [If set, do not use savetty/resetty])
else
    AC_MSG_RESULT([no])
fi

AC_MSG_CHECKING([for OpenBSD])
if test `uname` = OpenBSD; then
    AC_MSG_RESULT([yes, do not use savetty])
    AC_DEFINE(NO_SAVE_TTY, [], [If set, do not use savetty/resetty])
else
    AC_MSG_RESULT([no])
fi

AC_CHECK_LIB($lcurses, color_set, [], [
    AC_MSG_ERROR([
Your curses library is so puny it does not support colors.
Please switch to a better curses library. If you are compiling
on Solaris, one in /usr/xpg4 may be a lot better, but I could not find it
    ])
])

AC_ARG_ENABLE([debug], AS_HELP_STRING([--enable-debug], [Compile with debug (disabled by default)]), [
    CFLAGS="$CFLAGS -g -O0"
    debug_on=yes
], [debug_on=no])

AC_ARG_ENABLE([efence], AS_HELP_STRING([--enable-efence], [Compile with libefence (disabled by default)]), [
    LDFLAGS="$LDFLAGS -lefence"
    efence_on=yes
], [efence_on=no])

AC_ARG_ENABLE([hiscores], AS_HELP_STRING([--enable-hiscores],
        [Enable hi scores (enabled by default)]),
    [], [enable_hiscores=yes])

AC_MSG_CHECKING([if debugging should be included])
AC_MSG_RESULT([$debug_on])

if test "efence_on" = "yes"; then
    AC_CHECK_LIB([efence], [malloc], [], [
        AC_MSG_FAILURE([efence library not found])
    ])
fi

case $hcurses in
ncursesw)
    AC_DEFINE(CURSES_NCURSESW, [], [Wide char support ncurses library])
    ;;
ncurses)
    AC_DEFINE(CURSES_NCURSES, [], [ncurses library])
    if test $ncurses_direct = 1; then
        AC_DEFINE(NCURSES_DIRECT, [], [ncurses.h is not in ncurses])
    fi
    ;;
curses)
    AC_DEFINE(CURSES_CURSES, [], [curses library])
    ;;
*)
    AC_MSG_FAILURE([Internal error : incorrect curses headers var : "$hcurses"])
    ;;
esac

if test "x$enable_hiscores" = "xyes"; then
    AC_DEFINE(ALLOW_HI_SCORES, [], [whether hi scores are supported])
fi


CFLAGS="$CFLAGS -DCLINES_DATA_DIR=\\\"\$(pkgdatadir)\\\""

AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)

AC_CONFIG_FILES([Makefile version])

AC_OUTPUT
