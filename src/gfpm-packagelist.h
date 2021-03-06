#ifndef __GFPM_PACKAGELIST_H__
#define __GFPM_PACKAGELIST_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <gtk/gtk.h>

typedef struct _gfpmlist
{
	gchar *data;
	struct _gfpmlist *next;
} GfpmList;

typedef enum _gfpmlisttype
{
	GFPM_INSTALL_LIST = 1,
	GFPM_REMOVE_LIST
} GfpmListType;

/* Inserts a new item into a GfpmList */
void gfpm_package_list_add (GfpmListType, const gchar *);

/* Remove an item from a GfpmList */
void gfpm_package_list_del (GfpmListType, const gchar *);

/* Free memory used by a GfpmList */
void gfpm_package_list_free (GfpmListType);

/* Print contents of a GfpmList */
/* temporary function, to be removed after testing */
void gfpm_package_list_print (GfpmListType);

/* Check whether a list is empty */
int gfpm_package_list_is_empty (GfpmListType);

#endif
