/*
 *  gfpm-interface.c for gfpm
 *
 *  Copyright (C) 2006-2007 by Priyank Gosalia <priyankmg@gmail.com>
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

#define _GNU_SOURCE
#include <locale.h>
#include <gtk/gtk.h>
#include <glade/glade.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "gfpm.h"
#include "gfpm-interface.h"
#include "gfpm-messages.h"
#include "gfpm-packagelist.h"
#include "gfpm-about.h"
#include "gfpm-db.h"

extern GladeXML *xml;
extern PM_DB	*sync_db;
extern PM_DB	*local_db;
extern char	*repo;

/* The GFPM main window */
GtkWidget *gfpm_mw;

static GtkWidget *gfpm_statusbar;
static GtkWidget *gfpm_groups_tvw;
static GtkWidget *gfpm_pkgs_tvw;
static GtkWidget *gfpm_info_tvw;
static GtkWidget *gfpm_files_txtvw;

static void cb_gfpm_repos_combo_changed (GtkComboBox *combo, gpointer data);
static void cb_gfpm_groups_tvw_selected (GtkTreeSelection *selection, gpointer data);
static void cb_gfpm_pkgs_tvw_selected (GtkTreeSelection *selection, gpointer data);
static void cb_gfpm_search_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data);
static void cb_gfpm_pkg_selection_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data);

void
gfpm_interface_init (void)
{
	GtkWidget		*gfpm_splash;
	GtkWidget		*widget;
	GtkListStore		*store;
	GtkCellRenderer		*renderer;
	GtkTreeSelection	*selection;

	gfpm_mw		= glade_xml_get_widget (xml, "mainwindow");
	gfpm_splash	= glade_xml_get_widget (xml, "splash_window");
	gfpm_statusbar	= glade_xml_get_widget (xml, "statusbar");
	gfpm_groups_tvw = glade_xml_get_widget (xml, "grouptreeview");
	gfpm_pkgs_tvw	= glade_xml_get_widget (xml, "pkgstreeview");
	gfpm_info_tvw	= glade_xml_get_widget (xml, "infotreeview");
	gfpm_files_txtvw= glade_xml_get_widget (xml, "filestextview");

	/* Setup repository combobox */
	widget = glade_xml_get_widget (xml, "combobox_repos");
	store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(widget)));
	gtk_combo_box_set_active (GTK_COMBO_BOX(widget), 0);
	g_signal_connect (G_OBJECT(widget), "changed", G_CALLBACK(cb_gfpm_repos_combo_changed), NULL);

	/* Refresh db button */
	//g_signal_connect (G_OBJECT(glade_xml_get_widget(xml, "button_refresh1")), "clicked", G_CALLBACK(cb_refresh_button_clicked), NULL);

	/* Setup groups treeview */
	store = gtk_list_store_new (1, G_TYPE_STRING);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_groups_tvw), -1, "Groups", renderer, "text", 0, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_groups_tvw), GTK_TREE_MODEL(store));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_groups_tvw));
	g_signal_connect (selection, "changed", G_CALLBACK(cb_gfpm_groups_tvw_selected), NULL);

	/* Setup pkgs treeview */
	store = gtk_list_store_new (6,
				G_TYPE_BOOLEAN,  /* Install status */
				GDK_TYPE_PIXBUF, /* Status icon */
				G_TYPE_STRING,   /* Package name */
				G_TYPE_STRING,   /* Installed version */
				G_TYPE_STRING,   /* Latest version */
				G_TYPE_STRING);  /* Package Description */

	renderer = gtk_cell_renderer_toggle_new ();
	g_object_set (G_OBJECT(renderer), "activatable", TRUE, NULL);
	g_signal_connect (renderer, "toggled", G_CALLBACK(cb_gfpm_pkg_selection_toggled), store);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_pkgs_tvw), -1, _("S"), renderer, "active", 0, NULL);

	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_pkgs_tvw), -1, _("Status"), renderer, "pixbuf", 1, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_pkgs_tvw), -1, _("Package Name"), renderer, "text", 2, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_pkgs_tvw), -1, _("Installed Version"), renderer, "text", 3, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(gfpm_pkgs_tvw), -1, _("Latest Version"), renderer, "text", 4, NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_pkgs_tvw), -1, _("Description"), renderer, "text", 5, NULL);

	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_pkgs_tvw), GTK_TREE_MODEL(store));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	g_signal_connect(selection, "changed", G_CALLBACK(cb_gfpm_pkgs_tvw_selected), NULL);

	/* Setup info treeview */
	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_info_tvw), -1, "Info", renderer, "text", 0, NULL);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "wrap-width", 300, NULL);
	g_object_set (renderer, "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_info_tvw), -1, "Value", renderer, "text", 1, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_info_tvw), GTK_TREE_MODEL(store));
	g_object_set (gfpm_info_tvw, "hover-selection", TRUE, NULL);

	/* search */
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml, "search_entry1")), "key-release-event", G_CALLBACK(cb_gfpm_search_keypress), NULL);
	
	/* about */
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml, "about_gfpm1")), "activate", G_CALLBACK(gfpm_about), NULL);

	/* refresh db */
	//g_signal_connect (G_OBJECT(glade_xml_get_widget(xml, "button_refresh1")), "clicked", G_CALLBACK(cb_refresh_button_clicked), NULL);

	/* initialize progressbar */
	//gfpm_progress_init ();

	/* Disable Apply, Refresh and File buttons if user is not root */
	if ( geteuid() != 0 )
	{
		gtk_widget_set_sensitive (GTK_WIDGET(glade_xml_get_widget(xml, "button_execute1")), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(glade_xml_get_widget(xml, "button_refresh1")), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(glade_xml_get_widget(xml, "button_file1")), FALSE);
	}

	gtk_widget_show (gfpm_splash);

	/* initialize dbs */
	gfpm_db_init ();

	/* load default repo  */
	gfpm_load_groups_tvw ("frugalware-current");
	gtk_widget_hide (gfpm_splash);
	gtk_widget_show_all (gfpm_mw);

	/* unref the glade xml object */
	g_object_unref (xml);

	return;
}

void
gfpm_update_status (const gchar *message)
{
	guint ci;

	if (!message)
		return;
	ci = gtk_statusbar_get_context_id (GTK_STATUSBAR(gfpm_statusbar), "-");
	gtk_statusbar_push (GTK_STATUSBAR(gfpm_statusbar), ci, message);

	return;
}

void
gfpm_load_groups_tvw (const char *repo_name)
{
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	PM_LIST		*l;
	PM_DB		*db;
	char		*temp;

	if (!strcmp(repo_name,"frugalware-current"))
		db = sync_db;
	else
		db = local_db;

	//display status here
	// clear tvws
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_groups_tvw));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	while (gtk_events_pending())
		gtk_main_iteration ();

	for (l=pacman_db_getgrpcache(db); l; l=pacman_list_next(l))
	{
		asprintf (&temp, _("Loading groups ... [%s]"), (char*)pacman_list_getdata(l));
		gfpm_update_status (temp);
		while (gtk_events_pending())
			gtk_main_iteration ();
		//display temp status
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, (char*)pacman_list_getdata(l), -1);
		g_free (temp);
	}

	gfpm_update_status (_("Loading groups ... DONE"));
	return;
}

void
gfpm_load_pkgs_tvw (const char *group_name)
{
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	PM_DB		*pm_db;
	PM_GRP		*pm_group;
	PM_LIST		*l, *i;
	PM_PKG		*pm_pkg = NULL;
	PM_PKG		*pm_lpkg = NULL;
	GdkPixbuf	*icon_yes = NULL;
	GdkPixbuf	*icon_no = NULL;
	GdkPixbuf	*icon_up = NULL;
	gboolean	check = FALSE;
	gint		r = 0;

	if (!strcmp(repo,"local"))
		pm_db = local_db;
	else
	{
		pm_db = sync_db;
		r = 1;
	}

	gfpm_update_status (_("Loading package list ..."));
	pm_group = pacman_db_readgrp (pm_db, (char*)group_name);
	l = pacman_grp_getinfo (pm_group, PM_GRP_PKGNAMES);
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	icon_yes = gtk_widget_render_icon	(gfpm_pkgs_tvw,
						GTK_STOCK_YES,
						GTK_ICON_SIZE_SMALL_TOOLBAR,
						NULL);
	icon_no = gtk_widget_render_icon	(gfpm_pkgs_tvw,
						GTK_STOCK_NO,
						GTK_ICON_SIZE_SMALL_TOOLBAR,
						NULL);
	icon_up = gtk_widget_render_icon	(gfpm_pkgs_tvw,
						GTK_STOCK_GO_UP,
						GTK_ICON_SIZE_SMALL_TOOLBAR,
						NULL);

	// display status
	for (i=l;i;i=pacman_list_next(i))
	{
		gboolean up = FALSE;
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		if (r == 1)
		{
			pm_pkg = pacman_db_readpkg (pm_db, pacman_list_getdata(i));
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			if (pacman_pkg_getinfo(pm_lpkg,PM_PKG_VERSION) == NULL)
				check = FALSE;
			else
			{
				check = TRUE;
				/* check if package needs updating */
				if (!strcmp((char*)pacman_pkg_getinfo(pm_pkg,PM_PKG_VERSION),
					(char*)pacman_pkg_getinfo(pm_lpkg,PM_PKG_VERSION)))
					up = FALSE;
				else
					up = TRUE;
			}

			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, check,
						1, (up==TRUE)?icon_up:(check==TRUE)?icon_yes:icon_no,
						2, (char*)pacman_list_getdata (i),
						3, (check==TRUE)?(char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION) : NULL,
						4, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
						5, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC),
						-1);
		}
		else if (r == 0)
		{
			gboolean up = FALSE;
			pm_pkg = pacman_db_readpkg (sync_db, pacman_list_getdata(i));
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			if (!strcmp((char*)pacman_pkg_getinfo(pm_pkg,PM_PKG_VERSION),
				(char*)pacman_pkg_getinfo(pm_lpkg,PM_PKG_VERSION)))
				up = FALSE;
			else
				up = TRUE;
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, TRUE,
						1, (up==TRUE)?icon_up:icon_yes,
						2, (char*)pacman_list_getdata (i),
						3, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION),
						4, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
						5, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_DESC),
						-1);
		}
		pacman_pkg_free (pm_pkg);
		pacman_pkg_free (pm_lpkg);
	}
	gfpm_update_status (_("Loading package list ...DONE"));

	g_object_unref (icon_yes);
	g_object_unref (icon_no);
	g_object_unref (icon_up);

	return;
}

void
gfpm_load_info_tvw (const char *pkg_name)
{
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	PM_PKG		*pm_pkg = NULL;
	PM_PKG		*pm_lpkg = NULL;
	PM_LIST		*temp, *i;
	gint		r = 0;
	gboolean	inst = FALSE;
	GString		*str;
	float		size;
	char		*st = NULL;

	if (!pkg_name)
		return;
	if (!strcmp(repo,"local"))
		pm_pkg = pacman_db_readpkg (local_db, (char*)pkg_name);
	else
	{	
		pm_pkg = pacman_db_readpkg (sync_db, (char*)pkg_name);
		r = 1;
	}

	/* if the package is in a remote repo, check if it's installed or not */
	if (r == 1)
	{
		pm_lpkg = pacman_db_readpkg (local_db, (char*)pkg_name);
		if (pacman_pkg_getinfo(pm_lpkg, PM_PKG_VERSION)!=NULL)
			inst = TRUE;
	}
	else if (r == 0)
	{
		pm_lpkg = pacman_db_readpkg (local_db, (char*)pkg_name);
		inst = TRUE;
	}

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_info_tvw));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	gtk_list_store_append (GTK_LIST_STORE(model), &iter); 
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, _("Name:"),
						1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_NAME),
						-1);
	gtk_list_store_append (GTK_LIST_STORE(model), &iter); 
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, _("Version:"),
						1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
						-1);
	gtk_list_store_append (GTK_LIST_STORE(model), &iter); 
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, _("Description:"),
						1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC),
						-1);

	/* populate depends */
	temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_DEPENDS);
	str = g_string_new ("");
	for (i=temp;i;i=pacman_list_next(i))
	{
		str = g_string_append (str, (char*)pacman_list_getdata(i));
		str = g_string_append (str, " ");
	}
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, _("Depends:"),
						1, (char*)str->str,
						-1);
	g_string_free (str, TRUE);

	/* populate provides */
	temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_PROVIDES);
	str = g_string_new ("");
	for (i=temp;i;i=pacman_list_next(i))
	{
		str = g_string_append (str, (char*)pacman_list_getdata(i));
		str = g_string_append (str, " ");
	}
	if (str->len)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("Provides:"),
					1, (char*)str->str,
					-1);
	}
	g_string_free (str, TRUE);
	
	/* populate conflicts */
	temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_CONFLICTS);
	str = g_string_new ("");
	for (i=temp;i;i=pacman_list_next(i))
	{
		str = g_string_append (str, (char*)pacman_list_getdata(i));
		str = g_string_append (str, " ");
	}
	if (str->len)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("Conflicts:"),
					1, (char*)str->str,
					-1);
	}
	g_string_free (str, TRUE);

	if (inst == TRUE)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("URL:"),
					1, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_URL),
					-1);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("Packager:"),
					1, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_PACKAGER),
					-1);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("Install Date:"),
					1, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_INSTALLDATE),
					-1);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		size = (float)((long)pacman_pkg_getinfo (pm_pkg, PM_PKG_USIZE)/1024)/1024,
		asprintf (&st, "%0.2f MB", size);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("Size:"),
					1, (char*)st,
					-1);
		g_free (st);
	}
	if (inst == FALSE)
	{
		size = (float)((long)pacman_pkg_getinfo (pm_pkg, PM_PKG_SIZE)/1024)/1024,
		asprintf (&st, "%0.2f MB", size);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("Size (compressed):"),
					1, (char*)st,
					-1);
		g_free (st);
		size = (float)((long)pacman_pkg_getinfo (pm_pkg, PM_PKG_USIZE)/1024)/1024,
		asprintf (&st, "%0.2f MB", size);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("Size (Uncompressed):"),
					1, (char*)st,
					-1);
		g_free (st);
	}
	if (r == 1)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, _("SHA1SUM:"),
					1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_SHA1SUM),
					-1);
	}
	if (inst == TRUE)
	{
		temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_REQUIREDBY);
		str = g_string_new ("");
		for (i=temp;i;i=pacman_list_next(i))
		{
			str = g_string_append (str, (char*)pacman_list_getdata(i));
			str = g_string_append (str, " ");
		}
		if (str->len)
		{
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, _("Required By:"),
						1, (char*)str->str,
						-1);
		}
		g_string_free (str, TRUE);
	}

	return;
}

void
gfpm_load_files_txtvw (const char *pkg_name, gboolean inst)
{
	GtkTextBuffer	*buffer;
	GtkTextIter	iter;
	PM_LIST		*i;
	PM_PKG		*pkg;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(gfpm_files_txtvw));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
	if (inst == TRUE)
	{
		pkg = pacman_db_readpkg (local_db, (char*)pkg_name);
		gtk_text_buffer_insert (buffer, &iter, _("Files in package:\n"), -1);		
		for (i = pacman_pkg_getinfo(pkg, PM_PKG_FILES); i; i = pacman_list_next(i))
		{
			gtk_text_buffer_insert (buffer, &iter, "   /", -1);
			gtk_text_buffer_insert (buffer, &iter, (char *)pacman_list_getdata(i), -1);
			gtk_text_buffer_insert (buffer, &iter, "\n", -1);
		}
		pacman_pkg_free (pkg);
	}
	else
		gtk_text_buffer_insert (buffer, &iter, _("Package is not installed.\n"), -1);

	gtk_text_view_set_buffer (GTK_TEXT_VIEW(gfpm_files_txtvw), buffer);

	return;
}


/* CALLBACKS */

static void
cb_gfpm_repos_combo_changed (GtkComboBox *combo, gpointer data)
{
	gint index;

	index = gtk_combo_box_get_active (combo);
	switch (index)
	{
		case 0: /* frugalware-Current */
				g_free (repo);
				asprintf (&repo, "frugalware-current");
				gfpm_load_groups_tvw ("frugalware-current");
				break;

		case 1:	/* local */
				g_free (repo);
				asprintf (&repo, "local");
				gfpm_load_groups_tvw ("local");
				break;

		default: break;
	}

	return;
}


static void
cb_gfpm_groups_tvw_selected (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	gchar		*group;

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 0, &group, -1);
		gfpm_load_pkgs_tvw (group);
		g_free (group);
	}

	return;
}

static void
cb_gfpm_pkgs_tvw_selected (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	gchar		*pkgname = NULL;
	gboolean	inst;

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 0, &inst, 2, &pkgname, -1);
		gfpm_load_info_tvw (pkgname);
		gfpm_load_files_txtvw (pkgname, inst);
		g_free (pkgname);
	}

	return;
}

static void
cb_gfpm_search_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	GtkListStore	*store;
	GdkPixbuf	*icon_yes;
	GdkPixbuf	*icon_no;
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	PM_PKG		*pm_pkg;
	PM_LIST		*l, *i;
	gchar		*search_str;
	gint		r = 0;

	if (event->keyval != GDK_Return)
		return;
	search_str = gtk_entry_get_text (GTK_ENTRY(widget));
	if (search_str == NULL)
		return;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	store = (GtkListStore*) model;
	pacman_set_option (PM_OPT_NEEDLES, (long)search_str);
	if (!strcmp("local", repo))
	{
		l = pacman_db_search (local_db);
		r = 0;
	}
	else if (!strcmp("frugalware-current", repo))
	{
		l = pacman_db_search (sync_db);
		r = 1;
	}
	if (l == NULL)
	{
		gfpm_update_status (_("Search Complete"));
		gfpm_error (_("No package found"));
		return;
	}
	icon_yes = gtk_widget_render_icon (gfpm_pkgs_tvw,
					GTK_STOCK_YES,
					GTK_ICON_SIZE_SMALL_TOOLBAR,
					NULL);
	icon_no = gtk_widget_render_icon (gfpm_pkgs_tvw,
					GTK_STOCK_NO,
					GTK_ICON_SIZE_SMALL_TOOLBAR,
					NULL);
	gfpm_update_status (_("Searching for packages ..."));
	if (r == 0)
	{
		PM_PKG	*pm_spkg;
		for (i=l;i;i=pacman_list_next(i))
		{
			pm_pkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			pm_spkg = pacman_db_readpkg (sync_db, pacman_list_getdata(i));
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
					0, TRUE,
					1, icon_yes,
					2, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_NAME),
					3, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
					4, (char*)pacman_pkg_getinfo (pm_spkg, PM_PKG_VERSION),
					5, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC),
					-1);
			pacman_pkg_free (pm_pkg);
			pacman_pkg_free (pm_spkg);
		}
	}
	else
	if (r == 1)
	{
		PM_PKG		*pm_lpkg;
		gboolean	inst = FALSE;
		for (i=l;i;i=pacman_list_next(i))
		{
			pm_pkg = pacman_db_readpkg (sync_db, pacman_list_getdata(i));
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			if (pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION)!=NULL)
					inst = TRUE;
				else
					inst = FALSE;

			gtk_list_store_append (store, &iter);
			if (inst == TRUE)
				gtk_list_store_set (store, &iter, 3, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION), -1);
			else
				gtk_list_store_set (store, &iter, 3, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION), -1); 

			gtk_list_store_set (store, &iter,
					0, inst,
					1, (inst==TRUE)?icon_yes:icon_no,
					2, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_NAME),
					4, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
					5, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC),
					-1);
			pacman_pkg_free (pm_pkg);
			pacman_pkg_free (pm_lpkg);
		}
	}
	pacman_set_option (PM_OPT_NEEDLES, (long)NULL);
	gfpm_update_status (_("Searching for packages ...DONE"));

	g_object_unref (icon_yes);
	g_object_unref (icon_no);
	return;
}

static void
cb_gfpm_pkg_selection_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data)
{
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	GtkTreePath	*path;
	gchar		*pkgname = NULL;
	gchar		*pkg = NULL;
	gboolean	check;
	gboolean	inst;
	PM_PKG		*pm_pkg = NULL;

	model = (GtkTreeModel *)data;
	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, 0, &check, 2, &pkgname, -1);

	/* check if the package is installed or not */
	pm_pkg = pacman_db_readpkg (local_db, pkgname);
	if (pm_pkg == NULL)
		inst = FALSE;
	else
		inst = TRUE;

	/* manually toggle the toggle button */
	check ^= 1;
	gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, check, -1);

	pkg = g_strdup (pkgname);
	if (check == TRUE)
	{
		if (inst == FALSE)
			gfpm_package_list_add (GFPM_INSTALL_LIST, pkg);
		else
			gfpm_package_list_del (GFPM_INSTALL_LIST, pkg);

		gfpm_package_list_del (GFPM_REMOVE_LIST, pkg);
	}
	else
	{
		if (inst == TRUE)
		{
			gfpm_package_list_add (GFPM_REMOVE_LIST, pkg);
			g_print ("adding to remove list\n");
		}
		else
			gfpm_package_list_del (GFPM_REMOVE_LIST, pkg);

		gfpm_package_list_del (GFPM_INSTALL_LIST, pkg);
	}

	/* remove the following snippet after testing */
	g_print ("Contents of INSTALL LIST\n");
	gfpm_package_list_print (GFPM_INSTALL_LIST);

	g_print ("Contents of REMOVE LIST\n");
	gfpm_package_list_print (GFPM_REMOVE_LIST);

	g_free (pkg);
	gtk_tree_path_free (path);

	return;
}
