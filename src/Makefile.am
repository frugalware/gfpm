INCLUDES=@GFPM_CFLAGS@ \
	-DPREFIX=\"$(prefix)\" \
	-fPIC

bin_PROGRAMS=gfpm

## Gfpm
gfpm_SOURCES=	wejpconfig.c 		\
		gfpm-util.c		\
		gfpm-config.c		\
		gfpm-db.c		\
		gfpm-icmonitor.c	\
		gfpm-packagelist.c	\
		gfpm-progress.c		\
		gfpm-interface.c	\
		gfpm-messages.c		\
		gfpm-optimizedb.c 	\
		gfpm-quickpane.c 	\
		gfpm-about.c 		\
		gfpm-repomanager.c 	\
		gfpm-logviewer.c 	\
		gfpm-prefs.c 		\
		gfpm.c

gfpm_LDADD= @GFPM_LIBS@ $(INOTIFY_LIBS) -lpacman

