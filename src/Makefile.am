INCLUDES=@GFPM_CFLAGS@ \
	-DPREFIX=\"$(prefix)\" \
	-fPIC

bin_PROGRAMS=gfpm

## Gfpm
gfpm_SOURCES=				\
		gfpm-messages.c		\
		gfpm-packagelist.c 	\
		gfpm-interface.c	\
		gfpm.c


gfpm_LDADD= @GFPM_LIBS@ -lalpm

