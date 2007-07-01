INCLUDES=@GFPM_CFLAGS@ \
	-DPREFIX=\"$(prefix)\" \
	-fPIC

bin_PROGRAMS=gfpm

## Gfpm
gfpm_SOURCES=				\
			gfpm-db.c		\
			gfpm-packagelist.c		\
			gfpm-interface.c \
			gfpm-messages.c \
			gfpm-about.c \
			gfpm.c


gfpm_LDADD= @GFPM_LIBS@ -lpacman

