# ***************************************************************************
# *  Makefile for gfpm
# *
# *  Sat Aug 26 22:36:56 2006
# *  Copyright	2006  Frugalware Developer Team
# *  Authors		Christian Hamar (krix) & Miklos Nemeth (desco)
# ****************************************************************************
#
# *  This program is free software; you can redistribute it and/or modify
# *  it under the terms of the GNU General Public License as published by
# *  the Free Software Foundation; either version 2 of the License, or
# *  (at your option) any later version.
# *
# *  This program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with this program; if not, write to the Free Software
# *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
# 

CC = gcc
CFLAGS = $(shell pkg-config libglade-2.0 --cflags) -W -Wall -s -O2 -I./src
LDFLAGS = $(shell pkg-config libglade-2.0 --libs) -lalpm -export-dynamic

PROGRAM = gfpm

SOURCES = src/gfpm.c src/widgets.c

all:    $(PROGRAM)

$(PROGRAM):	$(SOURCES)
	$(CC) -o $(PROGRAM) $(SOURCES) $(CFLAGS) $(LDFLAGS)
	rm -f src/*.*~ *~

clean:
	rm -f src/*.o src/*.*~  *~ $(PROGRAM)
