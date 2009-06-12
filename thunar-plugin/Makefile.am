INCLUDES = $(THUNAR_PLUGIN_GFPM_CFLAGS) 

extensiondir=$(THUNAR_EXTENSION_DIR)
extension_LTLIBRARIES=libthunar-gfpm.la
libthunar_gfpm_la_SOURCES =		\
	thunar-gfpm.c			\
	thunar-gfpm.h			\
	thunar-gfpm-module.c

libthunar_gfpm_la_LDFLAGS = -module -avoid-version -no-undefined 

libthunar_gfpm_la_LIBADD  = $(THUNAR_PLUGIN_GFPM_LIBS) -lpacman


