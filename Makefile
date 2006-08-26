
CC = gcc
CFLAGS = $(shell pkg-config libglade-2.0 --cflags) -s -O2
LDFLAGS = $(shell pkg-config libglade-2.0 --libs)

PROGRAM = gfpm

SOURCES = src/gfpm.c

all:    $(PROGRAM)

$(PROGRAM):	$(SOURCES)
	$(CXX) -o $(PROGRAM) $(SOURCES) $(CFLAGS) $(LDFLAGS)
	rm -f src/*.*~ *~

clean:
	rm -f src/*.o src/*.*~  *~ $(PROGRAM)
