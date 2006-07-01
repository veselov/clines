# 
# file: GNUmakefile
# author: Pawel S. Veselov
# created: 2002/10/06
# last modified: 02/10/08
# version: 1.3
# 

SRC = src

OBJS =  board.o \
	main.o \
	render.o \
	gameplay.o

DEFINES =
CURSES = -lcurses

# DM is defined to flag that turns on 'dependency' mode
# for the compiler. Default value is -M, but SUNWspro needs -xM
# DM=-xM1

ifeq ($(strip $(DM)),)
    DM=-M
endif

all:	clines

.depend:
	$(CC) $(DEFINES) $(DM) -I$(SRC)/include $(SRC)/*.c > .depend

-include .depend

vpath %.c   $(SRC)

clines:	$(OBJS)
	$(LD) -o $@ $^ $(CURSES)

.c.o:
	$(CC) $(DEFINES) -g -I$(SRC)/include -c $<

clean:
	rm -f *.o core*

distclean:  clean
	rm -f clines .depend

