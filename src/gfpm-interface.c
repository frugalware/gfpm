/***************************************************************************
 *  gfpm-interface.c
 *  Author(s): 		Priyank Gosalia <priyankmg@gmail.com>
 *  Old Authors(s):	Christian Hamar <krics@linuxforum.hu>
 *			Miklos Nemeth <desco@frugalware.org>
 *  Copyright 2006-2007 Frugalware Developer Team
 ****************************************************************************/

/*
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

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include "gfpm.h"
#include "gfpm-packagelist.h"
#include "gfpm-interface.h"

/* Main UI Widgets */
GtkWidget *gfpm_window;
GtkWidget *gfpm_statusbar;
GtkWidget *groups_treeview;
GtkWidget *pkgs_treeview;
GtkWidget *info_treeview;
GtkWidget *files_textview;

/* Package lists */
static GList *install_list = NULL;
static GList *remove_list = NULL;

char *repository = NULL;

extern GladeXML *xml;

/* Clear the package and info treeviews */
static void gfpm_clear_treeviews (void);

/* Callbacks */
static void cb_groups_treeview_selected (GtkTreeSelection *selection, gpointer data);
static void cb_pkgs_treeview_selected (GtkTreeSelection *selection, gpointer data);
static void cb_pkg_selection_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data);

void
gfpm_load_groups_treeview (char *repo_name)
{
	GtkTreeModel 	*model;
	GtkTreeIter 	iter;
	PM_LIST 	*i;
	char 		*tmp;
	guint		ci;

	if (gfpmdb != NULL)
	{
		alpm_db_unregister (gfpmdb);
		gfpmdb = alpm_db_register (repo_name);
	}
	else
	{
		gfpmdb = alpm_db_register (repo_name);
	}

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(groups_treeview));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	ci = gtk_statusbar_get_context_id (GTK_STATUSBAR(gfpm_statusbar), "-");
	gtk_statusbar_push (GTK_STATUSBAR(gfpm_statusbar), ci, _("Loading groups ..."));
	gtk_widget_show (gfpm_statusbar);

	/* Need this to fresh up the gui while loading pkgs */
	while (gtk_events_pending())
		gtk_main_iteration();

	for (i = alpm_db_getgrpcache(gfpmdb); i; i = alpm_list_next(i))
	{	
		asprintf (&tmp, _("Loading groups ... [%s]"), (char *)alpm_list_getdata(i));
		gtk_statusbar_push (GTK_STATUSBAR(gfpm_statusbar), ci, tmp);
		while (gtk_events_pending())
			gtk_main_iteration();
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, (char *)alpm_list_getdata(i),
				-1);
		g_free (tmp);
	}

	gtk_statusbar_push (GTK_STATUSBAR(gfpm_statusbar), ci, _("Loading groups ... DONE"));
	return;
}

void
gfpm_load_pkgs_treeview (char *group_name)
{
	GtkTreeIter 		iter;
	GdkPixbuf 		*icon;
	GtkTreeModel 		*model;
	PM_LIST 		*pkgnames, *i;
	PM_GRP 			*grp;
	PM_PKG 			*pkg, *check_pkg;
	PM_DB 			*check_db;
	gint 			r;
	gboolean		check;
	gchar			*version;

	grp = alpm_db_readgrp (gfpmdb, group_name);

	if (!strcmp(repository, "local"))
	{	
		r = 1; /* in 'local' repo */
	}
	else
	{	
		r = 0; /* in 'remote' repo */
		check_db = alpm_db_register ("local");
	}

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(pkgs_treeview));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	pkgnames = alpm_grp_getinfo (grp, PM_GRP_PKGNAMES);

	for (i = pkgnames; i; i = alpm_list_next(i))
	{
		pkg = alpm_db_readpkg (gfpmdb, alpm_list_getdata(i));
		if (r == 0)
		{
			check_pkg = alpm_db_readpkg (check_db, alpm_list_getdata(i));
			if (alpm_pkg_getinfo(check_pkg, PM_PKG_VERSION) == NULL)
			{	
				icon = gtk_widget_render_icon (pkgs_treeview, GTK_STOCK_NO, GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
				version = NULL;
				check = FALSE;
			}
			else
			{	
				icon = gtk_widget_render_icon (pkgs_treeview, GTK_STOCK_YES, GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
				version = (char *)alpm_pkg_getinfo (check_pkg, PM_PKG_VERSION);
				check = TRUE;
				
			}
		} 
		else
		{
			icon = gtk_widget_render_icon (pkgs_treeview, GTK_STOCK_YES, GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
		}

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, check,
				1, icon,
				2, (char *)alpm_list_getdata (i),
				3, check ? (char*)alpm_pkg_getinfo (check_pkg, PM_PKG_VERSION) : NULL,
				4, (char *)alpm_pkg_getinfo (pkg, PM_PKG_VERSION),
				5, (char *)alpm_pkg_getinfo (pkg, PM_PKG_DESC),
				-1);
	}
	
	if (r == 0)
	{
		alpm_db_unregister (check_db);
	}
	
	return;
}

void
gfpm_load_info_treeview (char *pkg_name, gboolean installed)
{
	GtkTreeModel 	*model;
	GtkTreeIter 	iter;
	PM_DB		*local_db;
	PM_LIST 	*i, *y;
	PM_PKG 		*pkg, *local_pkg;
	GString 	*foo;
	char 		*tmp;
	int 		r;
	float		size;

	if (!strcmp(repository, "local"))
		r = 1; /* in 'local' repo */
	else
		r = 0; /* in 'remote' repo */

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(info_treeview));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	if ((pkg = alpm_db_readpkg (gfpmdb, pkg_name))==NULL)
		return;

	if (installed == TRUE)
	{
		local_db = alpm_db_register ("local");
		local_pkg = alpm_db_readpkg (local_db, pkg_name);
	}

	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, _("Name:"),
			1, (char *)alpm_pkg_getinfo (pkg, PM_PKG_NAME),
			-1);

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			0, _("Version:"),
			1, (char *)alpm_pkg_getinfo (pkg, PM_PKG_VERSION),
			-1);

	if (installed == TRUE)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, _("Packager:"),
				1, (char *)alpm_pkg_getinfo (local_pkg, PM_PKG_PACKAGER),
				-1);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, _("URL:"),
				1, (char *)alpm_pkg_getinfo(local_pkg, PM_PKG_URL),
				-1);
	}

	if (r != 1)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		size = (float)((long)alpm_pkg_getinfo(pkg, PM_PKG_SIZE)/1024)/1024;
		asprintf (&tmp, "%0.2f MB", size);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, _("Size (Compressed):"),
				1, (char *)tmp,
				-1);
		g_free (tmp);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		size = (float)((long)alpm_pkg_getinfo(pkg, PM_PKG_USIZE)/1024)/1024;
		asprintf (&tmp, "%0.2f MB", size);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, _("Size (Uncompressed):"),
				1, (char *)tmp,
				-1);
		g_free (tmp);

		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, "SHA1SUM:",
				1, (char *)alpm_pkg_getinfo (pkg, PM_PKG_SHA1SUM),
				-1);
	}
	/* Get depends */
	y = alpm_pkg_getinfo (pkg, PM_PKG_DEPENDS);
	foo = g_string_new ("");
	for (i = y; i; i = alpm_list_next(i))
	{
		foo = g_string_append (foo, (char *)alpm_list_getdata(i));
		foo = g_string_append (foo, " ");
	}
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, _("Depends:"),
			1, (char *)foo->str,
			-1);

	/* Get provides */
	y = alpm_pkg_getinfo (pkg, PM_PKG_PROVIDES);
	foo = g_string_new ("");
	for (i = y; i; i = alpm_list_next(i))
	{
		foo = g_string_append (foo, (char *)alpm_list_getdata(i));
		foo = g_string_append (foo, " ");
	}
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, _("Provides:"),
			1, (char *)foo->str,
			-1);

	y = alpm_pkg_getinfo (pkg, PM_PKG_CONFLICTS);
	foo = g_string_new ("");
	for (i = y; i; i = alpm_list_next(i))
	{
		foo = g_string_append (foo, (char *)alpm_list_getdata(i));
		foo = g_string_append (foo, " ");
	}
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
			0, _("Conflicts:"),
			1, (char *)foo->str,
			-1);

	if (r != 0)
	{
		y = alpm_pkg_getinfo (pkg, PM_PKG_REQUIREDBY);
		foo = g_string_new ("");
		for (i = y; i; i = alpm_list_next(i))
		{
			foo = g_string_append (foo, (char *)alpm_list_getdata(i));
			foo = g_string_append (foo, " ");
		}
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
				0, _("Required by:"),
				1, (char *)foo->str,
				-1);
	}

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
			0, _("Description:"),
			1, (char *)alpm_pkg_getinfo (pkg, PM_PKG_DESC),
			-1);

	if (installed == TRUE)
		alpm_db_unregister (local_db);

	return;
}

void
gfpm_load_files_textview (char *pkg_name, gboolean installed)
{
	GtkTextBuffer 	*buffer;
	GtkTextIter 	iter;
	PM_DB		*local_db;
	PM_LIST 	*i;
	PM_PKG 		*pkg;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(files_textview));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
	
	if (installed == TRUE)
	{
		local_db = alpm_db_register ("local");
		pkg = alpm_db_readpkg (local_db, pkg_name);
		gtk_text_buffer_insert (buffer, &iter, "Files in package:\n", -1);		
		for (i = alpm_pkg_getinfo(pkg, PM_PKG_FILES); i; i = alpm_list_next(i))
		{
			gtk_text_buffer_insert (buffer, &iter, "   /", -1);
			gtk_text_buffer_insert (buffer, &iter, (char *)alpm_list_getdata(i), -1);
			gtk_text_buffer_insert (buffer, &iter, "\n", -1);
		}
		alpm_db_unregister (local_db);
	}
	else
		gtk_text_buffer_insert (buffer, &iter, "Package is not installed.\n", -1);

	gtk_text_view_set_buffer (GTK_TEXT_VIEW(files_textview), buffer);

	return;
}

/* Lookup widgets and initialize signals */
void
gfpm_interface_init (void)
{
	GtkWidget	*gfpm_splash;
	GtkListStore 	*store;
	GtkCellRenderer *renderer;
	GtkTreeSelection *selection;
	
	gfpm_window = glade_xml_get_widget (xml, "mainwindow");
	gfpm_splash = glade_xml_get_widget (xml, "splash_window");
	gfpm_statusbar = glade_xml_get_widget (xml, "statusbar");
	groups_treeview = glade_xml_get_widget (xml, "grouptreeview");
	pkgs_treeview = glade_xml_get_widget (xml, "pkgstreeview");
	info_treeview = glade_xml_get_widget (xml, "infotreeview");
	files_textview = glade_xml_get_widget (xml, "filestextview");
	
	/* Setup groups treeview */
	store = gtk_list_store_new (1, G_TYPE_STRING);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(groups_treeview), -1, "Groups", renderer, "text", 0, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(groups_treeview), GTK_TREE_MODEL(store));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(groups_treeview));
	g_signal_connect (selection, "changed", G_CALLBACK(cb_groups_treeview_selected), NULL);

	/* Setup pkgs treeview */
	store = gtk_list_store_new (6,
				G_TYPE_BOOLEAN,
				GDK_TYPE_PIXBUF,
				G_TYPE_STRING,
				G_TYPE_STRING, G_TYPE_STRING,
				G_TYPE_STRING);
	renderer = gtk_cell_renderer_toggle_new ();
	g_object_set (G_OBJECT(renderer), "activatable", TRUE, NULL);
	g_signal_connect (renderer, "toggled", G_CALLBACK(cb_pkg_selection_toggled), store);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(pkgs_treeview), -1, _("S"), renderer, "active", 0, NULL);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(pkgs_treeview), -1, _("Status"), renderer, "pixbuf", 1, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(pkgs_treeview), -1, _("Package Name"), renderer, "text", 2, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(pkgs_treeview), -1, _("Installed Version"), renderer, "text", 3, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pkgs_treeview), -1, _("Latest Version"), renderer, "text", 4, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(pkgs_treeview), -1, _("Description"), renderer, "text", 5, NULL);

	gtk_tree_view_set_model (GTK_TREE_VIEW(pkgs_treeview), GTK_TREE_MODEL(store));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(pkgs_treeview));
	g_signal_connect(selection, "changed", G_CALLBACK(cb_pkgs_treeview_selected), NULL);

	/* Setup info treeview */
	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(info_treeview), -1, "Info", renderer, "text", 0, NULL);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "wrap-width", 300, NULL);
	g_object_set (renderer, "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(info_treeview), -1, "Value", renderer, "text", 1, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(info_treeview), GTK_TREE_MODEL(store));
	g_object_set (info_treeview, "hover-selection", TRUE, NULL);

	gtk_widget_show (gfpm_splash);
	gfpm_load_groups_treeview ("frugalware-current");
	asprintf (&repository, "frugalware-current");
	gtk_widget_hide (gfpm_splash);
	gtk_widget_show (gfpm_window);
	
	return;
}

static void
gfpm_clear_treeviews (void)
{
	GtkTreeModel *model;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(info_treeview));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(pkgs_treeview));
	gtk_list_store_clear (GTK_LIST_STORE(model));

}

static void
cb_groups_treeview_selected (GtkTreeSelection *selection, gpointer data)
{
	gchar			*groupname;
	GtkTreeModel 		*model;
	GtkTreeIter		iter;

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 0, &groupname, -1);
		gfpm_load_pkgs_treeview (groupname);
		g_free (groupname);
	}
	
	return;
}

static void
cb_pkgs_treeview_selected (GtkTreeSelection *selection, gpointer data)
{
	gchar			*pkgname;
	GtkTreeModel 		*model;
	GtkTreeIter		iter;
	gboolean		installed;

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 0, &installed, 2, &pkgname, -1);
		gfpm_load_info_treeview (pkgname, installed);
		gfpm_load_files_textview (pkgname, installed);
		g_free (pkgname);
	}
	
	return;
}

static void
cb_pkg_selection_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data)
{
	GtkTreeModel 	*model;
	GtkTreeIter 	iter;
	GtkTreePath 	*path;
	gchar		*pkgname;
	gboolean 	fixed;

	model = (GtkTreeModel *)data;
	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, 0, &fixed, 2, &pkgname, -1);

	fixed ^= 1;
	gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, fixed, -1);
	
	if (TRUE != gtk_cell_renderer_toggle_get_active(toggle))
	{	
		//g_print ("Adding to install list...\n");
		if (gfpm_package_list_find (remove_list, pkgname))
			gfpm_remove_package_list_remove (remove_list, pkgname);
		gfpm_install_package_list_insert (install_list, pkgname);
	}
	else
		gfpm_remove_package_list_insert (remove_list, pkgname);
	
	g_print ("Contents of INSTALL LIST\n");
	while (install_list != NULL)
	{
		g_print ("name: %s\n",(gchar*)install_list->data);
		g_print ("===\n");
		install_list = g_list_next (install_list);
	}

	gtk_tree_path_free (path);
	
	return;
}

