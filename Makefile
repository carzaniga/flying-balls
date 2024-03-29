.PHONY: default
default: all

GTK_PACKAGES=gdk-pixbuf-2.0 gtk+-3.0
GTK_CFLAGS=$(shell pkg-config --cflags $(GTK_PACKAGES))
GTK_LIBS=$(shell pkg-config --libs $(GTK_PACKAGES))

# PROFILING_CFLAGS=-pg
CFLAGS=-Wall -g -O2 $(PROFILING_CFLAGS) $(GTK_CFLAGS)

LIBS=$(GTK_LIBS) -lm

PROGS=balls
OBJS=balls.o c_index.o game.o gravity.o spaceship.o main.o

# dependencies (gcc -MM *.c)
balls.o: balls.c game.h balls.h gravity.h
c_index.o: c_index.c balls.h game.h c_index.h
game.o: game.c game.h
gravity.o: gravity.c gravity.h game.h
main.o: main.c game.h balls.h c_index.h gravity.h spaceship.h
spaceship.o: spaceship.c balls.h game.h
stats.o: stats.c

.PHONY: run
run: balls
	./balls

.PHONY: all
all: $(PROGS)

balls: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

.PHONY: clean
clean:
	rm -f *.o $(PROGS) $(OBJS)
