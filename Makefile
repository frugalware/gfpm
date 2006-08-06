# Purpose: makefile for calendar example (Unix)
# Created: 2000-01-03

CXX = $(shell wx-config --cxx)
CXXFLAGS = $(shell wx-config --cxxflags) -s -O2 -march=i686
LDFLAGS = $(shell wx-config --libs) -lalpm

PROGRAM = wxfpm

OBJECTS = src/app.cpp src/frame.cpp

all:    $(PROGRAM)

$(PROGRAM):	$(OBJECTS)
	$(CXX) -o $(PROGRAM) $(OBJECTS) $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -f src/*.o $(PROGRAM)
