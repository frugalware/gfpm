ENV_CFLAGS=$CFLAGS

AC_PREREQ(2.59)
AC_INIT([Gfpm], 1.0.0, [priyankmg@gmail.com], gfpm)
AC_DEFINE_UNQUOTED([GFPM_RELEASE_NAME], ["Triton"], [Release codenames])
AC_CONFIG_SRCDIR([src/gfpm.c])
AC_LANG(C)
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_SYSTEM
AC_PROG_INTLTOOL([0.21])
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE

AC_PROG_CC(GCC)
AC_HEADER_STDC
AC_CHECK_HEADERS([locale.h stdlib.h string.h sys/ioctl.h unistd.h alpm.h])

if test "${GCC}" = "yes" ; then
    CFLAGS="${ENV_CFLAGS} -pipe"
fi

AC_ARG_ENABLE(werror,
    AC_HELP_STRING([  --enable-werror], [Enable -Werror usage]), [werror="yes"])
AC_ARG_ENABLE(debug,
    AC_HELP_STRING([  --enable-debug], [Enable debugging support]), [debug="yes"])

dnl Check for Werror request
AC_MSG_CHECKING(for werror usage)
if test "x$werror" = "xyes" ; then
    CFLAGS="${CFLAGS} -Werror"
    werror=yes
    AC_MSG_RESULT(yes)
else
    werror=no
    AC_MSG_RESULT(no)
fi

dnl Check for debug enabled or not
AC_MSG_CHECKING(for debug request)
if test "x$debug" = "xyes" ; then
    AC_DEFINE(GFPM_DEBUG, 1, [Debug support: enabled])
    CFLAGS="${CFLAGS} -ggdb3 -Wall -DGFPM_DEBUG"
    debug=yes
    AC_MSG_RESULT(yes)
else
    AC_DEFINE(GFPM_DEBUG, 0, [Debug support: disabled])
    CFLAGS="${CFLAGS} -Wall"
    debug=no
    AC_MSG_RESULT(no)
fi

AC_SUBST([GFPM_DEBUG])
AC_SUBST([GFPM_RELEASE_NAME])

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
data/Makefile
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/24x24/Makefile
data/icons/32x32/Makefile
data/icons/48x48/Makefile
data/icons/128x128/Makefile
po/Makefile.in
Makefile
])

echo "
Gfpm $VERSION configuration summary:

        prefix                   : ${prefix}
        source code location     : ${srcdir}
        compiler                 : ${CC}
        compiler flags           : ${CFLAGS}
	debug support            : ${debug}
	werror usage             : ${werror}
"
