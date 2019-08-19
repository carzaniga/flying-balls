GTK_PACKAGES=gdk-pixbuf-2.0 gtk+-2.0
GTK_CFLAGS=$(shell pkg-config --cflags $(GTK_PACKAGES))
GTK_LIBS=$(shell pkg-config --libs $(GTK_PACKAGES))

# PROFILING_CFLAGS=-pg
CFLAGS=-Wall -g -O2 $(PROFILING_CFLAGS) $(GTK_CFLAGS)

LIBS=$(GTK_LIBS)

PROGS=balls

.PHONY: default
default: all

.PHONY: run
run: balls
	./balls

.PHONY: all
all: $(PROGS)

balls: balls.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(LIBS) -o $@

.PHONY: clean
clean:
	rm -f *.o $(PROGS)
