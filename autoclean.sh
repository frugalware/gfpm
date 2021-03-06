#!/bin/sh -xu

[ -f Makefile ] && make distclean
rm -rf autom4te.cache
rm -rf Makefile
rm -rf Makefile.in
rm -rf configure
rm -rf config.*
rm -rf stamp*
rm -rf depcomp
rm -rf install-sh
rm -rf missing
rm -rf src/Makefile
rm -rf src/Makefile.in
rm -rf aclocal.m4
rm -rf ltmain.sh
rm -rf compile
rm -rf libtool
rm -rf mkinstalldirs
rm -rf config.rpath
rm -rf data/Makefile.in
rm -rf data/Makefile
rm -rf data/icons/Makefile.in
rm -rf data/icons/Makefile
rm -rf data/icons/{16x16,24x24,32x32,128x128,48x48}/Makefile.in
rm -rf data/icons/{16x16,24x24,32x32,128x128,48x48}/Makefile
rm -rf po/stamp-it
rm -rf intltool-{extract,merge,update}
rm -rf intltool-{extract,merge,update}.in
rm -rf po/POTFILES
rm -rf po/Makefile{.in,.in.in}
rm -rf po/Makefile
