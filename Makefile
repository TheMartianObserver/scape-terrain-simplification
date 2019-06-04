CC = CC
cc = cc
#
# Notes on compiler flags:
#
#     -Olimit 1400 is necessary to allow optimization in the
#                  check_swap routine
#
CFLAGS = -O2 -Olimit 1400 -I.
LFLAGS =
LM = -lmalloc -lfastm -lm
LIBS = -lgl -lX11 $(LM)

CORE = quadedge.o hfield.o stuff.o Basic.o stmops.o
SIMPL = $(CORE) simplfield.o heap.o scan.o cmdline.o

SCAPE = $(SIMPL) scape.o nogl.o
GLSCAPE = $(SIMPL) glscape.o views.o circle.o glcode.o
DRAW  = $(SIMPL) drawscape.o views.o circle.o glcode.o

.C.o: scape.H
	$(CC) $(CFLAGS) -c $*.C

.c.o:
	$(cc) $(CFLAGS) -c $*.c

glscape : $(GLSCAPE)
	rm -f glscape
	$(CC) $(CFLAGS) -o glscape $(GLSCAPE) $(LIBS)

scape : $(SCAPE)
	rm -f scape
	$(CC) $(CFLAGS) -o scape $(SCAPE) $(LM)


drawscape : $(DRAW)
	rm -f drawscape
	$(CC) $(LFLAGS) -o drawscape $(DRAW) $(LIBS)

quadedge.o heap.o hfield.o scan.o scape.o simplfield.o stuff.o views.o: \
	geom2d.H quadedge.H scape.H simplfield.H

stmops.o: STM-tools/stmops.c
	$(cc) $(CFLAGS) -c STM-tools/stmops.c

clean:
	/bin/rm -f glscape scape drawscape *.o core
	cd STM-tools ; $(MAKE) clean
