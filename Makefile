GTK_PACKAGES=gdk-pixbuf-2.0 gtk+-3.0
GTK_CFLAGS=$(shell pkg-config --cflags $(GTK_PACKAGES))
GTK_LIBS=$(shell pkg-config --libs $(GTK_PACKAGES))

# PROFILING_CFLAGS=-pg
CFLAGS=-Wall -g -O2 $(PROFILING_CFLAGS) $(GTK_CFLAGS)

LIBS=$(GTK_LIBS) -lm

PROGS=balls
OBJS=balls.o c_index.o

.PHONY: default
default: all

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
