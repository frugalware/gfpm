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
#include "gfpm-progress.h"
#include "gfpm-util.h"
#include "gfpm-about.h"
#include "gfpm-db.h"

extern GladeXML *xml;
extern PM_DB	*sync_db;
extern PM_DB	*local_db;
extern GfpmList *install_list;
extern GfpmList *remove_list;
extern char	*repo;

/* The GFPM main window */
GtkWidget *gfpm_mw;

static GtkWidget *gfpm_statusbar;
static GtkWidget *gfpm_groups_tvw;
static GtkWidget *gfpm_pkgs_tvw;
static GtkWidget *gfpm_info_tvw;
static GtkWidget *gfpm_files_txtvw;
static GtkWidget *gfpm_clrall_opt;
static GtkWidget *gfpm_clrold_opt;
static GtkWidget *gfpm_inst_from_file_dlg;
static GtkWidget *gfpm_inst_filechooser;
static GtkWidget *gfpm_inst_upgcheck;
static GtkWidget *gfpm_inst_depcheck;
static GtkWidget *gfpm_inst_forcheck;
static GtkWidget *gfpm_apply_inst_depcheck;
static GtkWidget *gfpm_apply_inst_dwocheck;
static GtkWidget *gfpm_apply_rem_depcheck;

static void cb_gfpm_repos_combo_changed (GtkComboBox *combo, gpointer data);
static void cb_gfpm_groups_tvw_selected (GtkTreeSelection *selection, gpointer data);
static void cb_gfpm_pkgs_tvw_selected (GtkTreeSelection *selection, gpointer data);
static void cb_gfpm_search_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data);
static void cb_gfpm_pkg_selection_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data);
static void cb_gfpm_apply_btn_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_install_file_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_clear_cache_apply_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_refresh_button_clicked (GtkButton *button, gpointer data);

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
	gtk_widget_show (gfpm_splash);
	while (gtk_events_pending())
		gtk_main_iteration ();
	
	sleep (2);
	gfpm_groups_tvw = glade_xml_get_widget (xml, "grouptreeview");
	gfpm_pkgs_tvw	= glade_xml_get_widget (xml, "pkgstreeview");
	gfpm_info_tvw	= glade_xml_get_widget (xml, "infotreeview");
	gfpm_files_txtvw= glade_xml_get_widget (xml, "filestextview");
	gfpm_clrold_opt = glade_xml_get_widget (xml, "rem_old_opt");
	gfpm_clrall_opt = glade_xml_get_widget (xml, "rem_all_opt");
	gfpm_inst_from_file_dlg = glade_xml_get_widget (xml, "inst_from_file_dlg");
	gfpm_inst_filechooser = glade_xml_get_widget (xml, "gfpm_inst_filechooser");
	gfpm_inst_depcheck = glade_xml_get_widget (xml, "depcheck");
	gfpm_inst_upgcheck = glade_xml_get_widget (xml, "upgcheck");
	gfpm_inst_forcheck = glade_xml_get_widget (xml, "forcheck");
	gfpm_apply_inst_depcheck = glade_xml_get_widget (xml, "applyinstdepcheck");
	gfpm_apply_rem_depcheck = glade_xml_get_widget (xml, "applyremdepcheck");
	gfpm_apply_inst_dwocheck = glade_xml_get_widget (xml, "applyinstdwcheck");

	/* Setup repository combobox */
	widget = glade_xml_get_widget (xml, "combobox_repos");
	store = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(widget)));
	gtk_combo_box_set_active (GTK_COMBO_BOX(widget), 0);
	g_signal_connect (G_OBJECT(widget), "changed", G_CALLBACK(cb_gfpm_repos_combo_changed), NULL);

	/* Setup groups treeview */
	store = gtk_list_store_new (1, G_TYPE_STRING);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_groups_tvw), -1, "Groups", renderer, "text", 0, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_groups_tvw), GTK_TREE_MODEL(store));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_groups_tvw));
	g_signal_connect (selection, "changed", G_CALLBACK(cb_gfpm_groups_tvw_selected), NULL);

	/* Setup pkgs treeview */
	store = gtk_list_store_new (5,
				G_TYPE_BOOLEAN,  /* Install status */
				GDK_TYPE_PIXBUF, /* Status icon */
				G_TYPE_STRING,   /* Package name */
				G_TYPE_STRING,   /* Installed version */
				G_TYPE_STRING);   /* Latest version */
				//G_TYPE_STRING);  /* Package Description */

	GtkTreeViewColumn *column;
	renderer = gtk_cell_renderer_toggle_new ();
	g_object_set (G_OBJECT(renderer), "activatable", TRUE, NULL);
	g_signal_connect (renderer, "toggled", G_CALLBACK(cb_gfpm_pkg_selection_toggled), store);
	column = gtk_tree_view_column_new_with_attributes (_("S"),
							renderer,
							"active", 0,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Status"),
							renderer,
							"pixbuf", 1,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Package Name"),
							renderer,
							"text", 2,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 140);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Installed Version"),
							renderer,
							"text", 3,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Latest Version"),
							renderer,
							"text", 4,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	/*
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Description"),
							renderer,
							"text", 5,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);
	*/
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_pkgs_tvw), GTK_TREE_MODEL(store));

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	g_signal_connect(selection, "changed", G_CALLBACK(cb_gfpm_pkgs_tvw_selected), NULL);

	/* Setup info treeview */
	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_info_tvw), -1, "Info", renderer, "markup", 0, NULL);

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
	
	/* aply */
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml, "button_apply")), "clicked", G_CALLBACK(cb_gfpm_apply_btn_clicked), NULL);

	/* refresh db */
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml, "button_refresh1")), "clicked", G_CALLBACK(cb_gfpm_refresh_button_clicked), NULL);

	/* clear cache dialog */
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml, "rem_apply")), "clicked", G_CALLBACK(cb_gfpm_clear_cache_apply_clicked), NULL);
	
	/* install from file */
	g_signal_connect (G_OBJECT(glade_xml_get_widget(xml, "inst_from_file_install")), "clicked", G_CALLBACK(cb_gfpm_install_file_clicked), NULL);


	/* Disable Apply, Refresh and File buttons if user is not root */
	if ( geteuid() != 0 )
	{
		gtk_widget_set_sensitive (GTK_WIDGET(glade_xml_get_widget(xml, "button_apply")), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(glade_xml_get_widget(xml, "button_refresh1")), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET(glade_xml_get_widget(xml, "button_file1")), FALSE);
	}


	/* initialize modules */
	gfpm_db_init ();
	gfpm_messages_init ();
	gfpm_progress_init ();

	/* load default repo  */
	gfpm_load_groups_tvw ("frugalware-current");
	gtk_widget_hide (gfpm_splash);
	gtk_widget_show_all (gfpm_mw);

	/* unref the glade xml object */
	g_object_unref (xml);

	return;
}

static void
cb_gfpm_apply_btn_clicked (GtkButton *button, gpointer data)
{
	GString *errorstr = g_string_new ("");
	
	if (!gfpm_package_list_is_empty(GFPM_INSTALL_LIST) && !gfpm_package_list_is_empty(GFPM_REMOVE_LIST))
	{	
		gfpm_message (_("No changes to apply."));
		return;
	}
	if (gfpm_apply_dlg_show() != GTK_RESPONSE_OK)
	{	
		if (gfpm_question(_("Are you sure you want to cancel this operation ? \nNote: All changes made till now will be reverted."))==GTK_RESPONSE_YES)
		{	
			/* revert all changes */
			gfpm_package_list_free (GFPM_INSTALL_LIST);
			gfpm_package_list_free (GFPM_REMOVE_LIST);
			gfpm_apply_dlg_reset ();
			return;
		}
	}

	gfpm_apply_dlg_hide ();
	/* process remove list first */
	if (gfpm_package_list_is_empty(GFPM_REMOVE_LIST))
	{
		gint flags = 0;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_apply_rem_depcheck)))
			flags |= PM_TRANS_FLAG_NODEPS;

		/* create transaction */
		if (pacman_trans_init(PM_TRANS_TYPE_REMOVE, flags, gfpm_progress_event, NULL, gfpm_progress_install) == -1)
		{
			gchar *str;
			str = g_strdup_printf (_("Failed to init transaction (%s)\n"), pacman_strerror(pm_errno));
			errorstr = g_string_append (errorstr, str);
			if (pm_errno == PM_ERR_HANDLE_LOCK)
				errorstr = g_string_append (errorstr,
							_("If you're sure a package manager is not already running, you can delete /tmp/pacman-g2.lck"));
			gfpm_error (errorstr->str);
			return;
		}
		
		gfpm_progress_show (TRUE);
		GList *i = NULL;
		PM_LIST *data, *pkgs;
		for (i = (GList*)remove_list; i; i = i->next)
		{
			char *target = i->data;
			pacman_trans_addtarget (target);
		}
		if (pacman_trans_prepare(&data) == -1)
			g_print ("failed to prepare transaction (%s)\n", pacman_strerror(pm_errno));
		pkgs = pacman_trans_getinfo (PM_TRANS_PACKAGES);
		if (pkgs == NULL) g_print ("pkgs is null.. bad bad bad!\n");
		
		/* commit transaction */
		if (pacman_trans_commit(&data) == -1)
		{	
			char *str = g_strdup_printf ("Failed to commit transaction (%s)", pacman_strerror(pm_errno));
			errorstr = g_string_append (errorstr, str);
			gfpm_error (errorstr->str);
			g_free (str);
			g_string_free (errorstr, FALSE);
			return;
		}

		/* release the transaction */
		pacman_trans_release ();
		/* clear list */
		gfpm_package_list_free (GFPM_REMOVE_LIST);
		gfpm_apply_dlg_reset ();
	}
	if (gfpm_package_list_is_empty(GFPM_INSTALL_LIST))
	{
		gint flags = 0;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_apply_inst_depcheck)))
			flags |= PM_TRANS_FLAG_NODEPS;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_apply_inst_dwocheck)))
			flags |= PM_TRANS_FLAG_DOWNLOADONLY;
		/* create transaction */
		if (pacman_trans_init(PM_TRANS_TYPE_SYNC, flags, gfpm_progress_event, NULL, gfpm_progress_install) == -1)
		{
			gchar *str;
			str = g_strdup_printf (_("Failed to init transaction (%s)\n"), pacman_strerror(pm_errno));
			errorstr = g_string_append (errorstr, str);
			if (pm_errno == PM_ERR_HANDLE_LOCK)
				errorstr = g_string_append (errorstr,
							_("If you're sure a package manager is not already running, you can delete /tmp/pacman-g2.lck"));
			gfpm_error (errorstr->str);
			return;
		}
		
		gfpm_progress_show (TRUE);
		GList *i = NULL;
		PM_LIST *data, *pkgs;
		for (i = (GList*)install_list; i; i = i->next)
		{
			char *target = i->data;
			pacman_trans_addtarget (target);
		}
		if (pacman_trans_prepare(&data) == -1)
			g_print ("failed to prepare transaction (%s)\n", pacman_strerror(pm_errno));
		pkgs = pacman_trans_getinfo (PM_TRANS_PACKAGES);
		if (pkgs == NULL) g_print ("pkgs is null.. bad bad bad!\n");
		
		/* commit transaction */
		if (pacman_trans_commit(&data) == -1)
		{	
			char *str = g_strdup_printf ("Failed to commit transaction (%s)", pacman_strerror(pm_errno));
			errorstr = g_string_append (errorstr, str);
			gfpm_error (errorstr->str);
			g_free (str);
			g_string_free (errorstr, FALSE);
			return;
		}

		/* release the transaction */
		pacman_trans_release ();
		/* clear list */
		gfpm_package_list_free (GFPM_INSTALL_LIST);
		gfpm_apply_dlg_reset ();
	}
	gfpm_progress_show (FALSE);

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


	for (l=pacman_db_getgrpcache(db); l; l=pacman_list_next(l))
	{
		while (gtk_events_pending())
			gtk_main_iteration ();
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
	while (gtk_events_pending())
		gtk_main_iteration ();
	pm_group = pacman_db_readgrp (pm_db, (char*)group_name);
	l = pacman_grp_getinfo (pm_group, PM_GRP_PKGNAMES);
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	gtk_list_store_clear (GTK_LIST_STORE(model));

	icon_yes = gtk_widget_render_icon	(gfpm_pkgs_tvw,
						GTK_STOCK_YES,
						GTK_ICON_SIZE_MENU,
						NULL);
	icon_no = gtk_widget_render_icon	(gfpm_pkgs_tvw,
						GTK_STOCK_NO,
						GTK_ICON_SIZE_MENU,
						NULL);
	icon_up = gtk_widget_render_icon	(gfpm_pkgs_tvw,
						GTK_STOCK_GO_UP,
						GTK_ICON_SIZE_MENU,
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
						2, g_strstrip((char*)pacman_list_getdata (i)),
						3, (check==TRUE)?(char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION) : NULL,
						4, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
						//5, g_strstrip((char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC)),
						-1);
		}
		else if (r == 0)
		{
			gboolean up = FALSE;
			pm_pkg = pacman_db_readpkg (sync_db, pacman_list_getdata(i));
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			char *v1 = (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION);
			char *v2 = (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION);
			if (v1!=NULL && v2!=NULL)
			{	
				gint ret = gfpm_vercmp (v1, v2);
				if (!ret)
					up = FALSE;
				else if (ret == -1)
					up = TRUE;
				else
					up = FALSE;
			}
			else
			{	
				up = FALSE;
			}
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, TRUE,
						1, (up==TRUE)?icon_up:icon_yes,
						2, g_strstrip((char*)pacman_list_getdata (i)),
						3, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION),
						4, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
						//5, g_strstrip((char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_DESC)),
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
	char		*st, *tmp = NULL;

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
	st = (char*)gfpm_bold (_("Name:"));
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, st,
						1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_NAME),
						-1);
	g_free (st);
	gtk_list_store_append (GTK_LIST_STORE(model), &iter); 
	st = (char*)gfpm_bold (_("Version:"));
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, st,
						1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
						-1);
	g_free (st);
	gtk_list_store_append (GTK_LIST_STORE(model), &iter); 
	st = (char*)gfpm_bold (_("Description:"));
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, st,
						1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC),
						-1);
	g_free (st);
	/* populate license */
	if (inst == TRUE)
	{
		temp = pacman_pkg_getinfo (pm_lpkg, PM_PKG_LICENSE);
		str = g_string_new ("");
		for (i=temp;i;i=pacman_list_next(i))
		{
			str = g_string_append (str, (char*)pacman_list_getdata(i));
			str = g_string_append (str, " ");
		}
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("License:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
							0, st,
							1, (char*)str->str,
							-1);
		g_free (st);
		g_string_free (str, TRUE);
	}
	/* populate depends */
	temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_DEPENDS);
	str = g_string_new ("");
	for (i=temp;i;i=pacman_list_next(i))
	{
		str = g_string_append (str, (char*)pacman_list_getdata(i));
		str = g_string_append (str, " ");
	}
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	st = (char*)gfpm_bold (_("Depends:"));
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, st,
						1, (char*)str->str,
						-1);
	g_free (st);
	g_string_free (str, TRUE);
	
	/* populate groups */
	temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_GROUPS);
	str = g_string_new ("");
	for (i=temp;i;i=pacman_list_next(i))
	{
		str = g_string_append (str, (char*)pacman_list_getdata(i));
		str = g_string_append (str, " ");
	}
	gtk_list_store_append (GTK_LIST_STORE(model), &iter);
	st = (char*)gfpm_bold (_("Group(s):"));
	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, st,
						1, (char*)str->str,
						-1);
	g_free (st);
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
		st = (char*)gfpm_bold (_("Provides:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)str->str,
					-1);
		g_free (st);
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
		st = (char*)gfpm_bold (_("Conflicts:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)str->str,
					-1);
		g_free (st);
	}
	g_string_free (str, TRUE);

	if (inst == TRUE)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("URL:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_URL),
					-1);
		g_free (st);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("Packager:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_PACKAGER),
					-1);
		g_free (st);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("Install Date:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_INSTALLDATE),
					-1);
		g_free (st);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		size = (float)((long)pacman_pkg_getinfo (pm_lpkg, PM_PKG_SIZE)/1024)/1024;
		asprintf (&tmp, "%0.2f MB", size);
		st = (char*)gfpm_bold (_("Size:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)tmp,
					-1);
		g_free (st);
		g_free (tmp);
	}
	if (inst == FALSE)
	{
		size = (float)((long)pacman_pkg_getinfo (pm_pkg, PM_PKG_SIZE)/1024)/1024;
		asprintf (&tmp, "%0.2f MB", size);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("Size (Compressed):"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)tmp,
					-1);
		g_free (st);
		g_free (tmp);
		size = (float)((long)pacman_pkg_getinfo (pm_pkg, PM_PKG_USIZE)/1024)/1024,
		asprintf (&tmp, "%0.2f MB", size);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("Size (Uncompressed):"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)tmp,
					-1);
		g_free (st);
		g_free (tmp);
	}
	if (r == 1)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("SHA1SUM:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_SHA1SUM),
					-1);
		g_free (st);
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
			st = (char*)gfpm_bold (_("Required By:"));
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, st,
						1, (char*)str->str,
						-1);
			g_free (st);
		}
		g_string_free (str, TRUE);
		
		st = (char*)gfpm_bold (_("Reason:"));
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		switch ((int)pacman_pkg_getinfo (pm_lpkg, PM_PKG_REASON))
		{
			case PM_PKG_REASON_EXPLICIT:	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
										0, st,
										1, _("Explicitly Installed"),
										-1);
							break;
			case PM_PKG_REASON_DEPEND:	gtk_list_store_set (GTK_LIST_STORE(model), &iter,
										0, st,
										1, _("Installed as a dependency for another package"),
										-1);
							break;
			default:			break;
		}
		g_free (st);
		
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
cb_gfpm_refresh_button_clicked (GtkButton *button, gpointer data)
{
	gint ret;
	PM_PKG *pm_lpkg, *pm_spkg;
	PM_LIST *packages;
	static gchar *updatestr = 
		("Gfpm has detected a newer version of the pacman-g2 package. "
		 "It is recommended that you allow gfpm to upgrade pacman-g2 first. "
		 "Do you want to continue upgrading pacman-g2 ?");

	gfpm_progress_set_main_text (_("Synchronizing package databases"));
	gfpm_progress_show (TRUE);
	ret = pacman_db_update (0, sync_db);
	gfpm_progress_show (FALSE);
	/* check for a pacman-g2 update */
	pm_lpkg = pacman_db_readpkg (local_db, "pacman-g2");
	pm_spkg = pacman_db_readpkg (sync_db, "pacman-g2");
	if (strcmp((char*)pacman_pkg_getinfo(pm_lpkg, PM_PKG_VERSION),
				(char*)pacman_pkg_getinfo(pm_spkg, PM_PKG_VERSION)))
	{
		if (gfpm_question (updatestr) == GTK_RESPONSE_YES)
		{	
			gfpm_package_list_add (GFPM_INSTALL_LIST, "pacman-g2");
			cb_gfpm_apply_btn_clicked (NULL, NULL);
			goto cleanup;
		}
	}
	
	if (pacman_trans_init(PM_TRANS_TYPE_SYNC, 0, gfpm_progress_event, NULL, gfpm_progress_install) == -1)
	{
		gchar *str;
		GString *errorstr = g_string_new ("");
		str = g_strdup_printf (_("Failed to init transaction (%s)\n"), pacman_strerror(pm_errno));
		errorstr = g_string_append (errorstr, str);
		if (pm_errno == PM_ERR_HANDLE_LOCK)
			errorstr = g_string_append (errorstr,
						_("If you're sure a package manager is not already running, you can delete /tmp/pacman-g2.lck"));
		gfpm_error (errorstr->str);
		return;
	}
	if (pacman_trans_sysupgrade()==-1)
	{	
		g_print ("error %s", pacman_strerror(pm_errno));
	}
	packages = pacman_trans_getinfo (PM_TRANS_PACKAGES);	
	if (gfpm_plist_question(_("Following packages will be upgraded. Do you want to continue ?"), gfpm_pmlist_to_glist(packages)) == GTK_RESPONSE_YES)
	{
	
		PM_LIST *i = NULL;
		gfpm_package_list_free (GFPM_INSTALL_LIST);
		gfpm_package_list_free (GFPM_REMOVE_LIST);
		for (i=pacman_list_first(packages);i;i=pacman_list_next(i))
		{
			PM_SYNCPKG *sync = pacman_list_getdata (i);
			PM_PKG *pk = pacman_sync_getinfo (sync, PM_SYNC_PKG);
			gfpm_package_list_add (GFPM_INSTALL_LIST, pacman_pkg_getinfo(pk, PM_PKG_NAME));
		}
		pacman_trans_release ();
		cb_gfpm_apply_btn_clicked (NULL, NULL);
	}

cleanup:
	pacman_pkg_free (pm_lpkg);
	pacman_pkg_free (pm_spkg);
	pacman_trans_release ();

	return;
}

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
	GdkPixbuf	*icon_up;
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	PM_PKG		*pm_pkg;
	PM_LIST		*l = NULL;
	PM_LIST		*i = NULL;
	gchar		*search_str;
	gint		r = 0;

	if (event->keyval != GDK_Return)
		return;
	search_str = (gchar*)gtk_entry_get_text (GTK_ENTRY(widget));
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
					GTK_ICON_SIZE_MENU,
					NULL);
	icon_no = gtk_widget_render_icon (gfpm_pkgs_tvw,
					GTK_STOCK_NO,
					GTK_ICON_SIZE_MENU,
					NULL);
	icon_up = gtk_widget_render_icon (gfpm_pkgs_tvw,
					GTK_STOCK_GO_UP,
					GTK_ICON_SIZE_MENU,
					NULL);
	gfpm_update_status (_("Searching for packages ..."));
	if (r == 0)
	{
		PM_PKG	*pm_spkg;
		PM_PKG	*pm_lpkg;
		gboolean up = FALSE;
			
		while (gtk_events_pending())
			gtk_main_iteration ();
		for (i=l;i;i=pacman_list_next(i))
		{
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			pm_spkg = pacman_db_readpkg (sync_db, pacman_list_getdata(i));
			char *v1 = (char*)pacman_pkg_getinfo (pm_spkg, PM_PKG_VERSION);
			char *v2 = (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION);
			if (v1!=NULL && v2!=NULL)
			{
				gint ret = gfpm_vercmp (v1, v2);
				if (!ret)
					up = FALSE;
				else if (ret == -1)
					up = TRUE;
				else
					up = FALSE;
			}
			else
			{
				up = FALSE;
			}
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
					0, TRUE,
					1, (up==FALSE)?icon_yes:icon_up,
					2, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_NAME),
					3, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION),
					4, (char*)pacman_pkg_getinfo (pm_spkg, PM_PKG_VERSION),
					//5, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_DESC),
					-1);
			pacman_pkg_free (pm_lpkg);
			pacman_pkg_free (pm_spkg);
		}
	}
	else
	if (r == 1)
	{
		PM_PKG		*pm_lpkg;
		gboolean	inst = FALSE;
		gboolean	up = FALSE;
		for (i=l;i;i=pacman_list_next(i))
		{
			pm_pkg = pacman_db_readpkg (sync_db, pacman_list_getdata(i));
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			if (pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION)!=NULL)
			{	
				inst = TRUE;
				char *v1 = (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION);
				char *v2 = (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION);
				if (v1!=NULL && v2!=NULL)
				{
					if (!strcmp(v1,v2))
						up = FALSE;
					else up = TRUE;
				}
			}
			else
				inst = FALSE;
			gtk_list_store_append (store, &iter);
			if (inst == TRUE)
				gtk_list_store_set (store, &iter, 3, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION), -1);
			else
				gtk_list_store_set (store, &iter, 3, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION), -1); 

			gtk_list_store_set (store, &iter,
					0, inst,
					1, (inst==TRUE)?(up==TRUE)?icon_up:icon_yes:icon_no,
					2, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_NAME),
					4, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
					//5, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC),
					-1);
			pacman_pkg_free (pm_pkg);
			pacman_pkg_free (pm_lpkg);
		}
	}
	pacman_set_option (PM_OPT_NEEDLES, (long)NULL);
	gfpm_update_status (_("Searching for packages ...DONE"));

	g_object_unref (icon_yes);
	g_object_unref (icon_no);
	g_object_unref (icon_up);
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

	g_free (pkg);
	gtk_tree_path_free (path);

	return;
}

static void
cb_gfpm_clear_cache_apply_clicked (GtkButton *button, gpointer data)
{
	int ret;
	gchar *errstr = NULL;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_clrold_opt)) == TRUE)
	{
		if (gfpm_question(_("Are you sure you want to remove old packages from cache ?")) == GTK_RESPONSE_YES)
		{
			while (gtk_events_pending())
				gtk_main_iteration ();
			ret = pacman_sync_cleancache (0);
			if (!ret)
				gfpm_message (_("Finished clearing the cache"));
			else
			{
				errstr = g_strdup_printf (_("Failed to clean the cache (%s)"), pacman_strerror(pm_errno));
				gfpm_message (errstr);
				g_free (errstr);
			}
		}
		return;
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_clrall_opt)) == TRUE)
	{
		if (gfpm_question(_("Are you sure you want to remove all packages from cache ?")) == GTK_RESPONSE_YES)
		{
			while (gtk_events_pending())
				gtk_main_iteration ();
			ret = pacman_sync_cleancache (1);
			if (!ret)
				gfpm_message (_("Finished clearing the cache"));
			else
			{
				errstr = g_strdup_printf (_("Failed to clean the cache (%s)"), pacman_strerror(pm_errno));
				gfpm_message (errstr);
				g_free (errstr);
			}
		}
		return;
	}
	return;
}

static void
cb_gfpm_install_file_clicked (GtkButton *button, gpointer data)
{
	const char	*fpm = NULL;
	gchar		*str = NULL;
	GString		*errorstr = g_string_new ("");
	PM_LIST		*trans_data = NULL;
	gint		flags = 0;
	guint		type;

	fpm = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(gfpm_inst_filechooser));
	if (fpm == NULL)
	{	
		gfpm_error (_("No package selected for install. Please select a package to install."));
		return;
	}
	if (gfpm_question(_("Are you sure you want to install this package ?")) != GTK_RESPONSE_YES)
		return;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_inst_upgcheck)))
		type = PM_TRANS_TYPE_UPGRADE;
	else
		type = PM_TRANS_TYPE_ADD;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_inst_depcheck)))
		flags |= PM_TRANS_FLAG_NODEPS;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_inst_forcheck)))
		flags |= PM_TRANS_FLAG_FORCE;
	if (pacman_trans_init(type, flags, gfpm_progress_event, NULL, gfpm_progress_install) == -1)
	{
			str = g_strdup_printf (_("Failed to init transaction (%s)\n"), pacman_strerror(pm_errno));
			errorstr = g_string_append (errorstr, str);
			if (pm_errno == PM_ERR_HANDLE_LOCK)
			{	errorstr = g_string_append (errorstr,
							_("If you're sure a package manager is not already running, you can delete /tmp/pacman-g2.lck"));
				gfpm_error (errorstr->str);
			}
			return;
	}
	gfpm_progress_show (TRUE);
	/* add the target */
	pacman_trans_addtarget ((char*)fpm);
	if (pacman_trans_prepare(&trans_data) == -1)
	{	
		PM_LIST *i;
		GList	*pkgs = NULL;

		str = g_strdup_printf (_("Failed to prepare transaction (%s)\n"), pacman_strerror (pm_errno));
		gfpm_error (str);
		g_free (str);
		switch ((long)pm_errno)
		{
			case PM_ERR_UNSATISFIED_DEPS:
				for (i=pacman_list_first(trans_data);i;i=pacman_list_next(i))
				{
					GString	*depstring = g_string_new ("");	
					PM_DEPMISS *m = pacman_list_getdata (i);
					depstring = g_string_append (depstring, (char*)pacman_dep_getinfo(m,PM_DEP_NAME));
					switch ((long)pacman_dep_getinfo(m, PM_DEP_MOD))
					{
						gchar *val = NULL;
						case PM_DEP_MOD_EQ:
							val = g_strdup_printf ("=%s", (char*)pacman_dep_getinfo(m,PM_DEP_VERSION));
							depstring = g_string_append (depstring, val);
							break;
						case PM_DEP_MOD_GE:
							val = g_strdup_printf (">=%s", (char*)pacman_dep_getinfo(m,PM_DEP_VERSION));
							depstring = g_string_append (depstring, val);
							break;
						case PM_DEP_MOD_LE:
							val = g_strdup_printf ("<=%s", (char*)pacman_dep_getinfo(m,PM_DEP_VERSION));
							depstring = g_string_append (depstring, val);
							break;
						default: break;
					}
					pkgs = g_list_append (pkgs, (char*)g_strdup(depstring->str));
					g_string_free (depstring, FALSE);
				}
				pacman_list_free (trans_data);
				gfpm_plist_message (_("Following dependencies were not met. Please install these packages first."), GTK_MESSAGE_WARNING, pkgs);
				break;
			case PM_ERR_CONFLICTING_DEPS:
				for (i=pacman_list_first(trans_data);i;i=pacman_list_next(i))
				{
					GString	*depstring = g_string_new ("");	
					PM_DEPMISS *m = pacman_list_getdata (i);
					depstring = g_string_append (depstring, (char*)pacman_dep_getinfo(m, PM_DEP_NAME));
					pkgs = g_list_append (pkgs, (char*)depstring->str);
					g_string_free (depstring, FALSE);
				}
				pacman_list_free (trans_data);
				gfpm_plist_message (_("This package conflicts with the following packages"), GTK_MESSAGE_WARNING, pkgs);
				break;
		}

		goto cleanup;
	}
	if (pacman_trans_commit(&trans_data) == -1)
	{	
		str = g_strdup_printf (_("Failed to commit transaction (%s)\n"), pacman_strerror (pm_errno));
		gfpm_error (str);
		g_free (str);
		goto cleanup;
	}
	else
	{
		gfpm_message (_("Package successfully installed"));
	}

	cleanup:
	g_string_free (errorstr, FALSE);
	pacman_trans_release ();
	gtk_widget_hide (gfpm_inst_from_file_dlg);
	gfpm_progress_show (FALSE);
	return;
}

