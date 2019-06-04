CC = cc
CFLAGS = -O2
LIBS = -lm

TARGETS = flat2stm stm2pgm convert genstm cropstm


flat2stm: flat2stm.o stmops.o
	$(CC) -o flat2stm flat2stm.o stmops.o $(LIBS)

stm2pgm: stm2pgm.o stmops.o
	$(CC) -o stm2pgm stm2pgm.o stmops.o $(LIBS)

genstm: genstm.o stmops.o
	$(CC) -o genstm genstm.o stmops.o $(LIBS)

cropstm: cropstm.o stmops.o
	$(CC) -o cropstm cropstm.o stmops.o $(LIBS)

convert: DEMutil/convert.c
	$(CC) -o convert DEMutil/convert.c -lm


all: $(TARGETS)

clean:
	/bin/rm -f *.o
	/bin/rm -f $(TARGETS)
