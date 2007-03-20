ENV_CFLAGS=$CFLAGS

AC_PREREQ(2.59)
AC_INIT([Gfpm], 0.1, [priyankmg@gmail.com], gfpm)
AC_CONFIG_SRCDIR([src/gfpm.c])
AC_LANG(C)
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_SYSTEM
AC_PROG_INTLTOOL([0.21])
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE

AC_PROG_CC
AC_HEADER_STDC
AC_CHECK_HEADERS([locale.h stdlib.h string.h sys/ioctl.h unistd.h alpm.h])

if test "${GCC}" = "yes"
then
	CFLAGS="${CFLAGS} -pipe -Wall"
fi

ALL_LINGUAS=""
GETTEXT_PACKAGE=gfpm
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [needed for bindtextdomain() and textdomain()])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(LOCALEDIR)

pkg_modules="gtk+-2.0 >= 2.6.0 \
			 libglade-2.0 >= 2.6.0"
PKG_CHECK_MODULES([GFPM], [$pkg_modules])

CFLAGS="${CFLAGS}"

dnl ==========================================================================

AC_OUTPUT([
src/Makefile
po/Makefile.in
Makefile
])

echo "
Gfpm $VERSION configuration summary:

        prefix                   : ${prefix}
        source code location     : ${srcdir}
        compiler                 : ${CC}
        compiler flags           : ${CFLAGS}
"
