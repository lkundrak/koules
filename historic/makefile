
CC	 = gcc
OPTIMIZE = -fomit-frame-pointer -O2 -s
#OPTIMIZE = -Wall
CFLAGS	 = $(DEFINES) $(OPTIMIZE)
LFLAGS   = -N

PROGS    = koule
PROGS_O  = koule.o soundIt.o
LIBS     = -lvgagl -lvga

all:	progs

progs : $(PROGS)

objs:	$(PROGS_O)

.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

.c.s:
	$(CC) $(CFLAGS) -S -o $*.s $<

.o:
	$(CC) $(CFLAGS) $(LFLAGS) -o $* $*.o $(LIBS)
	chmod a+rs,go-w $*

koule: koule.o soundIt.o
	$(CC) $(CFLAGS) $(LFLAGS) -o koule koule.o soundIt.o $(LIBS) -lm

