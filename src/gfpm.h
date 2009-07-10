#ifndef __GFPM_H__
#define __GFPM_H__

#include <pacman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>

#ifdef _
#undef _
#endif
#define _(string) gettext (string)

#define gfpm_get_widget(x) GTK_WIDGET(gtk_builder_get_object(gb,x))

typedef enum {
	ARG_ADD = 0x01
} ARGS;

#endif
