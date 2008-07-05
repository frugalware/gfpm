/*
 *  gfpm-prefs.c for gfpm
 *
 *  Copyright (C) 2008 by Priyank Gosalia <priyankmg@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "gfpm-prefs.h"
#include "gfpm-messages.h"
#include "gfpm-interface.h"
#include "gfpm-util.h"
#include "gfpm-db.h"


static GtkWidget *gfpm_prefs_log_check;
static GtkWidget *gfpm_prefs_log_location;

static GtkWidget *gfpm_prefs_holdpkg_tvw;
static GtkWidget *gfpm_prefs_ignorepkg_tvw;

static GList *gfpm_prefs_holdpkg_list = NULL;
static GList *gfpm_prefs_ignorepkg_list = NULL;


static void gfpm_prefs_populate_holdpkg (void);
static void gfpm_prefs_populate_ignorepkg (void);
static void gfpm_prefs_populate_holdpkg_tvw (void);
static void gfpm_prefs_populate_ignorepkg_tvw (void);

void
gfpm_prefs_init (void)
{
	GtkListStore		*store = NULL;
	GtkCellRenderer		*renderer = NULL;
	GtkTreeViewColumn	*column = NULL;
	
	gfpm_prefs_holdpkg_tvw = gfpm_get_widget ("gfpm_prefs_holdpkg_tvw");
	gfpm_prefs_ignorepkg_tvw = gfpm_get_widget ("gfpm_prefs_ignorepkg_tvw");

	/* setup ui */
	store = gtk_list_store_new (1, G_TYPE_STRING);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Packages"),
								renderer,
								"text", 0,
								NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_prefs_holdpkg_tvw), column);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_prefs_holdpkg_tvw), GTK_TREE_MODEL(store));

	store = gtk_list_store_new (1, G_TYPE_STRING);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Packages"),
								renderer,
								"text", 0,
								NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_prefs_ignorepkg_tvw), column);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_prefs_ignorepkg_tvw), GTK_TREE_MODEL(store));
	
	gfpm_prefs_populate_holdpkg_tvw ();
	gfpm_prefs_populate_ignorepkg_tvw ();

	return;
}

static void
gfpm_prefs_populate_holdpkg (void)
{
	PM_LIST	*list = NULL;
	long	data;
	
	/* free the old list */
	if (gfpm_prefs_holdpkg_list != NULL)
		g_list_free (gfpm_prefs_holdpkg_list);
	
	/* populate the new list */
	pacman_parse_config (CFG_FILE, NULL, "");
	if (!pacman_get_option(PM_OPT_HOLDPKG, &data))
	{
		list = (PM_LIST*)data;
		while (list != NULL)
		{
			gfpm_prefs_holdpkg_list = g_list_append (gfpm_prefs_holdpkg_list,
								g_strdup(pacman_list_getdata(list)));
			list = pacman_list_next (list);
		}
	}
	
	return;
}

static void
gfpm_prefs_populate_ignorepkg (void)
{
	PM_LIST	*list = NULL;
	long	data;
	
	/* free the old list */
	if (gfpm_prefs_ignorepkg_list != NULL)
		g_list_free (gfpm_prefs_ignorepkg_list);
	
	/* populate the new list */
	pacman_parse_config (CFG_FILE, NULL, "");
	if (!pacman_get_option(PM_OPT_IGNOREPKG, &data))
	{
		list = (PM_LIST*)data;
		while (list != NULL)
		{
			gfpm_prefs_ignorepkg_list = g_list_append (gfpm_prefs_ignorepkg_list,
									g_strdup(pacman_list_getdata(list)));
			list = pacman_list_next (list);
		}
	}

	return;
}

static void
gfpm_prefs_populate_holdpkg_tvw (void)
{
	GList		*list = NULL;
	GtkListStore	*store = NULL;
	GtkTreeModel	*model = NULL;
	GtkTreeIter	iter;
	
	gfpm_prefs_populate_holdpkg ();
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_prefs_holdpkg_tvw));
	store = GTK_LIST_STORE (model);
	gtk_list_store_clear (store);
	
	list = gfpm_prefs_holdpkg_list;
	while (list != NULL)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, (char*)list->data, -1);
		list = g_list_next (list);
	}
	
	return;
}

static void
gfpm_prefs_populate_ignorepkg_tvw (void)
{
	GList		*list = NULL;
	GtkListStore	*store = NULL;
	GtkTreeModel	*model = NULL;
	GtkTreeIter	iter;
	
	gfpm_prefs_populate_ignorepkg ();
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_prefs_ignorepkg_tvw));
	store = GTK_LIST_STORE (model);
	gtk_list_store_clear (store);
	
	list = gfpm_prefs_ignorepkg_list;
	while (list != NULL)
	{
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, (char*)list->data, -1);
		list = g_list_next (list);
	}
	
	return;
}

