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
#include <glade/glade.h>

#ifdef _
#undef _
#endif
#define _(string) gettext (string)

#endif
