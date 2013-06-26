#ifndef __GFPM_UTIL_H__
#define __GFPM_UTIL_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <pacman.h>
#include "gfpm.h"

#define gfpm_min(p, q)  ((p) < (q) ? (p) : (q))

char * gfpm_trim (char *);

char * gfpm_bold (const char *);

GList * gfpm_pmlist_to_glist (PM_LIST *);

GdkPixbuf *gfpm_get_icon (const char *, int);

gint gfpm_check_if_package_updatable (const gchar *);

void gfpm_update_iconcache (void);

/* Converts a string to UTF-8. Returned string must be freed */
gchar * gfpm_convert_to_utf8 (const char *str);

#endif
