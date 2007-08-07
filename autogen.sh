#!/bin/sh -e

gen_pot()
{
	cd po/
	mv Makevars Makevars.tmp
	cp /usr/bin/intltool-extract ./
	intltool-update --pot --gettext-package=gfpm
	rm intltool-extract
	mv Makevars.tmp Makevars
	cd - >/dev/null
}

import_pootle()
{
	po_dir=~/git/translations/po
	if [ -d $po_dir ]; then
		: > po/LINGUAS
		for i in $(/bin/ls $po_dir/gfpm)
		do
			[ -e $po_dir/gfpm/$i/gnetconfig.po ] || continue
			cp $po_dir/gfpm/$i/gnetconfig.po po/$i.po
			if msgfmt -c --statistics -o po/$i.gmo po/$i.po; then
				echo $i >> po/LINGUAS
			else
				echo "WARNING: po/$i.po will break your build!"
			fi
		done
	else
		echo "WARNING: no po files will be used"
	fi
}

cd `dirname $0`

if [ "$1" == "--pot-only" ]; then
	gen_pot
	exit 0
fi

if [ "$1" == "--dist" ]; then
	ver=`grep AC_INIT configure.ac|sed 's/.*, \([0-9\.]*\), .*/\1/'`
	git-archive --format=tar --prefix=gfpm-$ver/ HEAD | tar xf -
	git log --no-merges |git name-rev --tags --stdin > gfpm-$ver/ChangeLog
	cd gfpm-$ver
	./autogen.sh --git
	cd ..
	tar czf gfpm-$ver.tar.gz gfpm-$ver
	rm -rf gfpm-$ver
	gpg --comment "See http://ftp.frugalware.org/pub/README.GPG for info" \
		-ba -u 20F55619 gfpm-$ver.tar.gz
	#mv gfpm-$ver.tar.gz.asc ../..
	exit 0
fi

cat /usr/share/aclocal/libtool.m4 >> aclocal.m4

intltoolize -c -f --automake
import_pootle
gen_pot
libtoolize -f -c
aclocal --force
autoheader -f
autoconf -f
cp -f $(dirname $(which automake))/../share/automake/mkinstalldirs ./
cp -f $(dirname $(which automake))/../share/gettext/config.rpath ./
automake -a -c --gnu --foreign

if [ "$1" == "--git" ]; then
	rm -rf autom4te.cache
fi

