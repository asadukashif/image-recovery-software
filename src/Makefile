CC=gcc
CFLAGS=-lm -O3

EXE_EXT=.exe
EXES=../dist/recover$(EXE_EXT)

OBJS=objs/recover.o objs/utils.o objs/getopt.o

all: $(EXES)

$(EXES): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

objs/%.o: %.c
	$(CC) -o $@ $< -c $(CFLAGS)

objs/getopt.o: ./getopt/getopt.c
	$(CC) -o $@ $^ $(CFLAGS) -c


test$(EXE_EXT): test.c objs/utils.o
	$(CC) -o $@ $^ $(CFLAGS)


clean:
	rm -rf obj objs/* $(EXES) ../dist/recover.exe