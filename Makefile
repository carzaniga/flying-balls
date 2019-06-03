GTK_PACKAGES=gdk-pixbuf-2.0 gtk+-2.0
GTK_CFLAGS=$(shell pkg-config --cflags $(GTK_PACKAGES))
GTK_LIBS=$(shell pkg-config --libs $(GTK_PACKAGES))

# CFLAGS=-Wall -g -O2 -std=c11 -pthread $(GTK_CFLAGS)
CFLAGS=-Wall -g -O2 $(GTK_CFLAGS)

# LIBS=$(GTK_LIBS) -lm
LIBS=$(GTK_LIBS)

PROGS=balls

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
