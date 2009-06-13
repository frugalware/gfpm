INCLUDES = $(THUNAR_PLUGIN_GFPM_CFLAGS) 

extensionsdir = $(THUNAR_EXTENSION_DIR)
extensions_LTLIBRARIES = libthunar-gfpm.la
libthunar_gfpm_la_SOURCES =		\
	thunar-gfpm.c			\
	thunar-gfpm.h			\
	thunar-gfpm-module.c

libthunar_gfpm_la_LDFLAGS = -module -avoid-version -no-undefined 

libthunar_gfpm_la_LIBADD  = $(THUNAR_PLUGIN_GFPM_LIBS) -lpacman

