
AUTOMAKE_OPTIONS = 1.7.8
EXTRA_DIST = version src/include

bin_PROGRAMS = clines
clines_SOURCES = src/board.c src/gameplay.c src/main.c src/render.c
clines_LDADD = $(LIBOBJS)
clines_CFLAGS = -I. -Isrc/include -Wall

dist-hook:
	rm -rf `find $(distdir) -name CVS`

