
AUTOMAKE_OPTIONS = 1.9
EXTRA_DIST = version src/include clines-man.xml

bin_PROGRAMS = clines
clines_SOURCES = src/board.c src/gameplay.c src/main.c src/render.c
clines_LDADD = $(LIBOBJS)
clines_CFLAGS = -I. -Isrc/include -Wall
dist_pkgdata_DATA = data/version1
man_MANS = clines.6

dist-hook:
	rm -rf `find $(distdir) -name CVS`
	cd $(distdir) && xsltproc /usr/local/share/xsl/docbook/manpages/docbook.xsl clines-man.xml

install-data-local:
	-chown 0 $(pkgdatadir)
	-chmod 1777 $(pkgdatadir)
