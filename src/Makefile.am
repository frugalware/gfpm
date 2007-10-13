INCLUDES=@GFPM_CFLAGS@ \
	-DPREFIX=\"$(prefix)\" \
	-fPIC

bin_PROGRAMS=gfpm

## Gfpm
gfpm_SOURCES=				\
			gfpm-util.c		\
			gfpm-db.c		\
			gfpm-packagelist.c		\
			gfpm-progress.c		\
			gfpm-interface.c \
			gfpm-messages.c \
			gfpm-optimizedb.c \
			gfpm-quickpane.c \
			gfpm-about.c \
			gfpm-icmonitor.c \
			gfpm.c

gfpm_LDADD= @GFPM_LIBS@ -lpacman -linotifytools

