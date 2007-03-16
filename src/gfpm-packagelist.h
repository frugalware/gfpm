#ifndef __GFPM_PACKAGELIST_H__
#define __GFPM_PACKAGELIST_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <gtk/gtk.h>

/* Insert a new item into the 'install' list */
GList * gfpm_install_package_list_insert (GList *, gchar *);

/* Insert a new item into the 'remove' list */
GList * gfpm_remove_package_list_insert (GList *, gchar *);

/* Searches for an item in the list */
gboolean gfpm_package_list_find (GList *, gchar *);

/* Insert a new item into the 'remove' list */
GList * gfpm_remove_package_list_remove (GList *, gchar *);

#endif
