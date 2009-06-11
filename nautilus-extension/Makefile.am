INCLUDES = $(NAUTILUS_EXT_GFPM_CFLAGS) 

nautilus_extensiondir=$(NAUTILUS_EXTENSION_DIR)
nautilus_extension_LTLIBRARIES=libnautilus-gfpm.la
libnautilus_gfpm_la_SOURCES =		\
	nautilus-gfpm.c			\
	nautilus-gfpm.h	\
	nautilus-gfpm-module.c

libnautilus_gfpm_la_LDFLAGS = -module -avoid-version -no-undefined
libnautilus_gfpm_la_LIBADD  = $(NAUTILUS_EXT_GFPM_LIBS) -lpacman


