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
	INSTALL_LIST = 1,
	REMOVE_LIST
} GfpmListType;

/* Print contents of a GfpmList */
void printlist (GfpmListType);

/* Inserts a new item into a GfpmList */
void gfpm_package_list_add (GfpmListType, const gchar *);

/* Remove an item from a GfpmList */
void gfpm_package_list_del (GfpmListType, const gchar *);

#endif
