CC = cc
CFLAGS = -Wall -O2
LDFLAGS =

all: sysneat
.c.o:
	$(CC) -c $(CFLAGS) $<
sysneat: sysneat.o
	$(CC) -o $@ sysneat.o $(LDFLAGS)
clean:
	rm -f *.o sysneat
