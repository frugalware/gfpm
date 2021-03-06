/*
 *  gfpm-interface.c for gfpm
 *
 *  Copyright (C) 2006-2009 by Priyank Gosalia <priyankmg@gmail.com>
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
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define ICON_INSTALLED		"gfpm-installed"
#define ICON_NINSTALLED		"gfpm-ninstalled"
#define ICON_NEEDUPDATE		"gfpm-nupdate"
#define ICON_LOCALNEWER		"gfpm-lnewer"

#include "gfpm.h"
#include "gfpm-interface.h"
#include "gfpm-config.h"
#include "gfpm-messages.h"
#include "gfpm-packagelist.h"
#include "gfpm-progress.h"
#include "gfpm-optimizedb.h"
#include "gfpm-quickpane.h"
#include "gfpm-icmonitor.h"
#include "gfpm-repomanager.h"
#include "gfpm-logviewer.h"
#include "gfpm-util.h"
#include "gfpm-about.h"
#include "gfpm-db.h"
#include "gfpm-prefs.h"

extern GtkBuilder *gb;
extern PM_DB	*sync_db;
extern PM_DB	*local_db;
extern GfpmList *install_list;
extern GfpmList *remove_list;
extern char	*repo;
extern gchar	*quickpane_pkg;

/* current argument */
static guint garg = 0;

/* current group the user is browsing */
/* used for refreshing the views after a package update */
gchar *current_group = NULL;

/* The GFPM main window */
GtkWidget *gfpm_mw;
GtkWidget *gfpm_pkgs_tvw = NULL;

/* init flag */
/* indicates that the repos were initied atleast once */
gboolean init = FALSE;

/* indicates gfpm is doing a package install/remove operation */
gboolean running = FALSE;

static GMutex *search_mutex = NULL;

enum gfpm_cols {
	COL_PKG_STATUS,
	COL_PKG_ICON,
	COL_PKG_NAME,
	COL_PKG_VER_INSTALLED,
	COL_PKG_VER_LATEST,
	COL_PKG_SIZE_COMPRESSED,
	COL_PKG_SIZE_UNCOMPRESSED
};

static GtkWidget *gfpm_splash = NULL;
static GtkWidget *gfpm_statusbar = NULL;
static GtkWidget *gfpm_groups_tvw = NULL;
static GtkWidget *gfpm_info_tvw = NULL;
static GtkWidget *gfpm_files_txtvw = NULL;
static GtkWidget *gfpm_clog_txtvw;
static GtkWidget *gfpm_clrall_opt;
static GtkWidget *gfpm_clrold_opt;
static GtkWidget *gfpm_inst_from_file_dlg;
static GtkWidget *gfpm_inst_filechooser;
static GtkWidget *gfpm_inst_upgcheck;
static GtkWidget *gfpm_inst_depcheck;
static GtkWidget *gfpm_inst_forcheck;
static GtkWidget *gfpm_inst_infoframe;
static GtkWidget *gfpm_inst_optframe;
static GtkWidget *gfpm_inst_infotvw;
static GtkWidget *gfpm_apply_inst_depcheck;
static GtkWidget *gfpm_apply_inst_dwocheck;
static GtkWidget *gfpm_apply_rem_depcheck;
static GtkWidget *gfpm_search_combo;
static GtkWidget *gfpm_repos_combo;
static GtkWidget *gfpm_compressed_size_col;
static GtkWidget *gfpm_uncompressed_size_col;

static guint gfpm_populate_repos_combobox (GtkComboBox *combo);
static void cb_gfpm_repos_combo_changed (GtkComboBox *combo, gpointer data);
static void cb_gfpm_groups_tvw_selected (GtkTreeSelection *selection, gpointer data);
static void cb_gfpm_groups_tvw_right_click (GtkTreeView *treeview, GdkEventButton *event);
static void cb_gfpm_pkgs_tvw_selected (GtkTreeSelection *selection, gpointer data);
static void cb_gfpm_pkgs_tvw_right_click (GtkTreeView *treeview, GdkEventButton *event);
static void cb_gfpm_search_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data);
static void cb_gfpm_search_buttonpress (GtkButton *button, gpointer data);
static void cb_gfpm_remove_group_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_pkg_selection_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data);
static void cb_gfpm_install_file_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_install_file_close_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_install_file_selection_changed (GtkFileChooser *chooser, gpointer data);
static void cb_gfpm_clear_cache_apply_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_refresh_button_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_mark_for_install (GtkButton *button, gpointer data);
static void cb_gfpm_mark_for_reinstall (GtkButton *button, gpointer data);
static void cb_gfpm_mark_for_removal (GtkButton *button, gpointer data);
static void cb_gfpm_mark_for_upgrade (GtkButton *button, gpointer data);
static void cb_gfpm_reinstall (GtkButton *button, gpointer data);
static gint gfpm_trans_prepare (PM_LIST *list);
static gint gfpm_trans_commit (PM_LIST **list);
static void gfpm_search (GtkWidget *widget);

static guint
gfpm_populate_repos_combobox (GtkComboBox *combo)
{
	GList			*rlist = NULL;
	GtkListStore		*store = NULL;
	GtkCellRenderer 	*renderer = NULL;
	GtkTreeIter		iter;
	gint			c_index = 0;
	gboolean		found = FALSE;
	
	if (init == FALSE)
	{
		store = gtk_list_store_new (1, G_TYPE_STRING);
		gtk_combo_box_set_model (GTK_COMBO_BOX(combo), GTK_TREE_MODEL(store));
		renderer = gtk_cell_renderer_text_new ();
		gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(combo), renderer, TRUE);
		gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT(combo), renderer, "text", 0, NULL);
	}
	else
	{
		store = GTK_LIST_STORE (gtk_combo_box_get_model(combo));
		if (store != NULL)
		{
			gtk_list_store_clear (store);
		}
	}	
	
	rlist = gfpm_db_get_repolist ();
	for (;rlist != NULL;rlist=rlist->next)
	{
		char *c_repo = (char *)pacman_db_getinfo ((PM_DB *)rlist->data, PM_DB_TREENAME);
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, c_repo, -1);
		if (found == FALSE)
		{
			if (!strcmp(c_repo, FW_CURRENT) || !strcmp(c_repo, FW_STABLE))
			{
				found = TRUE;
			}
			else
			{
				c_index++;
			}
		}
	}
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, 0, (char*)_("Installed Packages"), -1);

	return c_index;
}

void
gfpm_interface_setup_repo_combos (void)
{
	if (gfpm_db_populate_repolist() == 0)
	{
		guint active;
		active = gfpm_populate_repos_combobox (GTK_COMBO_BOX(gfpm_repos_combo));
		g_signal_connect (G_OBJECT(gfpm_repos_combo), "changed", G_CALLBACK(cb_gfpm_repos_combo_changed), NULL);
		gtk_combo_box_set_active (GTK_COMBO_BOX(gfpm_repos_combo), active);
		gfpm_populate_repos_combobox (GTK_COMBO_BOX(gfpm_search_combo));
		gtk_combo_box_set_active (GTK_COMBO_BOX(gfpm_search_combo), active);
	}
	else
	{
		gfpm_error (_("Error"), _("Error parsing repository information from configuration file."));
	}
	
	return;
}

void
gfpm_setup_pkgs_tvw (void)
{
	guint			cols = 7;
	gboolean		show_compressed = FALSE;
	gboolean		show_uncompressed = FALSE;
	GtkListStore		*store = NULL;
	GtkTreeViewColumn	*column = NULL;
	GtkCellRenderer		*renderer = NULL;
	
	if (gfpm_config_get_value_bool("show_compressed_size"))
	{
		show_compressed = TRUE;
	}
	if (gfpm_config_get_value_bool("show_uncompressed_size"))
	{
		show_uncompressed = TRUE;
	}
	store = gtk_list_store_new (cols,
				G_TYPE_BOOLEAN,  /* Install status */
				GDK_TYPE_PIXBUF, /* Status icon */
				G_TYPE_STRING,   /* Package name */
				G_TYPE_STRING,   /* Installed version */
				G_TYPE_STRING,	 /* Latest version */
				G_TYPE_STRING,	 /* Compressed size */
				G_TYPE_STRING);	 /* Uncompressed size */
	
	renderer = gtk_cell_renderer_toggle_new ();
	g_object_set (G_OBJECT(renderer), "activatable", TRUE, NULL);
	g_signal_connect (renderer, "toggled", G_CALLBACK(cb_gfpm_pkg_selection_toggled), store);
	column = gtk_tree_view_column_new_with_attributes (_("S"),
							renderer,
							"active", COL_PKG_STATUS,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Status"),
							renderer,
							"pixbuf", COL_PKG_ICON,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Package Name"),
							renderer,
							"text", COL_PKG_NAME,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_column_set_min_width (column, 140);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);
	gtk_tree_view_column_set_sort_column_id (column, COL_PKG_NAME);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Installed Version"),
							renderer,
							"text", COL_PKG_VER_INSTALLED,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Latest Version"),
							renderer,
							"text", COL_PKG_VER_LATEST,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Compressed Size"),
							renderer,
							"text", COL_PKG_SIZE_COMPRESSED,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_sort_column_id (column, COL_PKG_SIZE_COMPRESSED);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);
	gfpm_compressed_size_col = (GtkWidget*) column;
	gfpm_pkgs_show_compressed_size (show_compressed);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Uncompressed Size"),
							renderer,
							"text", COL_PKG_SIZE_UNCOMPRESSED,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
//	gtk_tree_view_column_set_sort_column_id (column, (show_compressed)?6:5);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_pkgs_tvw), column);
	gfpm_uncompressed_size_col = (GtkWidget*) column;
	gfpm_pkgs_show_uncompressed_size (show_uncompressed);

	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_pkgs_tvw), GTK_TREE_MODEL(store));

	return;
}

static void
_gfpm_groups_tvw_init (void)
{
	GtkListStore		*store = NULL;
	GtkCellRenderer		*renderer = NULL;
	GtkTreeSelection	*selection = NULL;

	gfpm_groups_tvw = gfpm_get_widget ("grouptreeview");
	store = gtk_list_store_new (1, G_TYPE_STRING);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_groups_tvw), -1, "Groups", renderer, "text", 0, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_groups_tvw), GTK_TREE_MODEL(store));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_groups_tvw));
	g_signal_connect (selection, "changed", G_CALLBACK(cb_gfpm_groups_tvw_selected), NULL);

	return;
}

static void
_gfpm_packages_tvw_init (void)
{
	GtkTreeSelection *selection = NULL;

	gfpm_pkgs_tvw	= gfpm_get_widget ("pkgstreeview");
	gfpm_setup_pkgs_tvw ();
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	g_signal_connect(selection, "changed", G_CALLBACK(cb_gfpm_pkgs_tvw_selected), NULL);
	g_signal_connect (gfpm_pkgs_tvw, "button-release-event", G_CALLBACK(cb_gfpm_pkgs_tvw_right_click), NULL);
	g_signal_connect (gfpm_groups_tvw, "button-release-event", G_CALLBACK(cb_gfpm_groups_tvw_right_click), NULL);

	return;
}

static void
_gfpm_info_tvw_init (void)
{
	GtkListStore		*store = NULL;
	GtkCellRenderer		*renderer = NULL;

	gfpm_info_tvw	= gfpm_get_widget ("infotreeview");
	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_info_tvw), -1, "Info", renderer, "markup", 0, NULL);

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "wrap-width", 300, NULL);
	g_object_set (renderer, "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_info_tvw), -1, "Value", renderer, "text", 1, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_info_tvw), GTK_TREE_MODEL(store));
	g_object_set (gfpm_info_tvw, "hover-selection", TRUE, NULL);
}

static void
_gfpm_inst_from_file_dlg_init (void)
{
	gfpm_inst_from_file_dlg = gfpm_get_widget ("inst_from_file_dlg");
	gfpm_inst_filechooser = gfpm_get_widget ("gfpm_inst_filechooser");
	gfpm_inst_depcheck = gfpm_get_widget ("depcheck");
	gfpm_inst_upgcheck = gfpm_get_widget ("upgcheck");
	gfpm_inst_forcheck = gfpm_get_widget ("forcheck");
	gfpm_inst_infoframe = gfpm_get_widget ("gfpm_inst_from_file_dlg_info_frame");
	gfpm_inst_optframe = gfpm_get_widget ("gfpm_inst_from_file_dlg_opt_frame");
	gfpm_inst_infotvw = gfpm_get_widget ("gfpm_inst_from_file_dlg_info_tvw");
	
	/* setup the package information treeview */
	GtkListStore		*store = NULL;
	GtkCellRenderer		*renderer = NULL;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_inst_infotvw), -1, "Info", renderer, "markup", 0, NULL);
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "wrap-width", 300, NULL);
	g_object_set (renderer, "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_inst_infotvw), -1, "Value", renderer, "text", 1, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_inst_infotvw), GTK_TREE_MODEL(store));
	g_object_set (gfpm_inst_infotvw, "hover-selection", TRUE, NULL);
	
	/* hide the info frame by default 
	 * we'll show it when the user loads a package */
	gtk_widget_hide (gfpm_inst_infoframe);

	/* signal */
	g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "inst_from_file_install")),
					"clicked",
					G_CALLBACK(cb_gfpm_install_file_clicked),
					NULL);
	g_signal_connect (G_OBJECT(gfpm_inst_filechooser),
					"selection-changed",
					G_CALLBACK(cb_gfpm_install_file_selection_changed),
					NULL);
	g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "inst_from_file_close")),
					"clicked",
					G_CALLBACK(cb_gfpm_install_file_close_clicked),
					NULL);

	return;
}

static void
_gfpm_misc_widgets_init (void)
{
	gfpm_statusbar	= gfpm_get_widget ("statusbar");
	gfpm_files_txtvw = gfpm_get_widget ("filestextview");
	gfpm_clog_txtvw = gfpm_get_widget ("changelogtextview");
	gfpm_clrold_opt = gfpm_get_widget ("rem_old_opt");
	gfpm_clrall_opt = gfpm_get_widget ("rem_all_opt");
	gfpm_apply_inst_depcheck = gfpm_get_widget ("applyinstdepcheck");
	gfpm_apply_rem_depcheck = gfpm_get_widget ("applyremdepcheck");
	gfpm_apply_inst_dwocheck = gfpm_get_widget ("applyinstdwcheck");
	
	/* Setup repository combobox */
	gfpm_repos_combo = gfpm_get_widget ("combobox_repos");
	gfpm_interface_setup_repo_combos ();
	if (!init) init = TRUE;

	return;
}

static void
_gfpm_search_init (void)
{
	gfpm_search_combo = gfpm_get_widget ("search_repocombo");
	g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "search_entry1")), "key-release-event", G_CALLBACK(cb_gfpm_search_keypress), NULL);
	g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "gfpm_searchbtn")),
				"clicked",
				G_CALLBACK(cb_gfpm_search_buttonpress),
				(gpointer)gtk_builder_get_object(gb, "search_entry1"));

	return;
}

static void
_gfpm_main_window_init (void)
{
	gchar		*title = NULL;
	GdkPixbuf	*icon = NULL;

	gfpm_mw	= gfpm_get_widget ("mainwindow");
	/* set application icon */
	icon = gfpm_get_icon ("gfpm", 32);
	gtk_window_set_icon (GTK_WINDOW(gfpm_mw), icon);
	g_object_unref (icon);
	/* set window title */
	title = g_strdup_printf ("%s (%s)", PACKAGE_STRING, GFPM_RELEASE_NAME);
	gtk_window_set_title (GTK_WINDOW(gfpm_mw), title);
	g_free (title);

	return;
}

void
gfpm_interface_init (ARGS arg, void* argdata)
{
	if (!arg)
	{
		gfpm_splash	= gfpm_get_widget ("splash_window");

		/* initialize main window */
		_gfpm_main_window_init ();

		gtk_widget_show (gfpm_splash);
		while (gtk_events_pending())
			gtk_main_iteration ();

		sleep (1);

		/* initialize search */
		_gfpm_search_init ();

		/* Setup groups treeview */
		_gfpm_groups_tvw_init ();

		/* Setup pkgs treeview */
		_gfpm_packages_tvw_init ();

		/* Setup info treeview */
		_gfpm_info_tvw_init ();
	
		/* Setup install from file dialog */
		_gfpm_inst_from_file_dlg_init ();

		/* about */
		g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "about_gfpm1")),
						"activate",
						G_CALLBACK(gfpm_about),
						NULL);
	
		/* syslog */
		g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "syslog1")),
						"activate",
						G_CALLBACK(gfpm_logviewer_show),
						NULL);
	
		/* repository manager */
		g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "menu_edit_repos")),
						"activate",
						G_CALLBACK(gfpm_repomanager_show),
						NULL);

		/* aply */
		g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "button_apply")),
						"clicked",
						G_CALLBACK(cb_gfpm_apply_btn_clicked),
						NULL);

		/* refresh db */
		g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "button_refresh1")),
						"clicked",
						G_CALLBACK(cb_gfpm_refresh_button_clicked),
						NULL);

		/* clear cache dialog */
		g_signal_connect (G_OBJECT(gtk_builder_get_object(gb, "rem_apply")),
						"clicked",
						G_CALLBACK(cb_gfpm_clear_cache_apply_clicked),
						(gpointer)gtk_builder_get_object(gb,"clear_cache_dlg"));

		/* Disable Apply, Refresh and File buttons if user is not root */
		if ( geteuid() != 0 )
		{
			/* disable some widgets */
			gtk_widget_set_sensitive (gfpm_get_widget("button_apply"), FALSE);
			gtk_widget_set_sensitive (gfpm_get_widget("button_refresh1"), FALSE);
			gtk_widget_set_sensitive (gfpm_get_widget("button_file1"), FALSE);
			gtk_widget_set_sensitive (gfpm_get_widget("button_preferences"), FALSE);
			gtk_widget_set_sensitive (gfpm_get_widget("menu_edit_repos"), FALSE);
			gtk_widget_set_sensitive (gfpm_get_widget("menu_edit_prefs"), FALSE);
			gtk_widget_set_sensitive (gfpm_get_widget("clr1"), FALSE);
			gtk_widget_set_sensitive (gfpm_get_widget("menu_tools_optimize"), FALSE);
		}
		else
		{
			/* init repomanager only if gfpm is run as root user */
			gfpm_repomanager_init ();
		}
		/* miscellanous widgets */
		_gfpm_misc_widgets_init ();
		/* optimize database dialog */
		gfpm_optimize_db_dlg_init ();
		/* quick pane */
		gfpm_quickpane_init ();
		#ifdef HAVE_ICMONITOR
		gfpm_icmonitor_init ();
		#endif
		/* log viewer */
		gfpm_logviewer_init ();
		/* preferences subsystem */
		gfpm_prefs_init ();
	}
	
	gfpm_messages_init ();
	gfpm_progress_init ();
	
	/* initialize modules */
	if (gfpm_db_init())
	{
		gfpm_error (_("Error"), _("Failed to initialize local package database."));
	}
	
	if (!arg)
	{
		/* init search mutex */
		search_mutex = g_mutex_new ();
		gtk_widget_hide (gfpm_splash);
		/* finally show the main window */
		gtk_widget_show (gfpm_mw);
		gtk_window_present (GTK_WINDOW(gfpm_mw));
	}
	else
	{
		switch (arg)
		{
			case ARG_ADD:
			{
				garg = ARG_ADD;
				/* Setup install from file dialog */
				_gfpm_inst_from_file_dlg_init ();
				if (geteuid()!=0)
				{
					gfpm_error (_("Insufficient privileges"),
								_("You need to be root in order to install packages"));
					gtk_widget_set_sensitive (gfpm_get_widget("inst_from_file_install"), FALSE);
					gtk_widget_hide (gfpm_inst_optframe);
				}
				//g_print ("argdata: %s\n", (char*)argdata);	
				if (gtk_file_chooser_select_filename((GtkFileChooser*)gfpm_inst_filechooser,(char*)argdata))
				{
					gtk_widget_show (gfpm_inst_from_file_dlg);
				}
				else
				{
					/* handle error */
					gfpm_error (_("Error"),
							_("Unknown error"));
				}
				break;
			}
		}
	}

	/* unref the glade xml object */
	g_object_unref (gb);

	return;
}

void
cb_gfpm_apply_btn_clicked (GtkButton *button, gpointer data)
{
	static gchar *lck_error = ("Gfpm has detected that another instance of a package manager is already running. "
			"If you're sure that a package manager is not already running, you can delete /tmp/pacman-g2.lck\n"
			"Do you want gfpm to delete it and continue with the operation ?\n");
	GString *errorstr = g_string_new ("");

	if (!gfpm_package_list_is_empty(GFPM_INSTALL_LIST) && !gfpm_package_list_is_empty(GFPM_REMOVE_LIST))
	{
		gfpm_message (_("Gfpm"), _("No changes to apply."));
		return;
	}
	if (gfpm_apply_dlg_show() != GTK_RESPONSE_OK)
	{
		if (gfpm_question(_("Cancel Operation"), _("Are you sure you want to cancel this operation ? \nNote: All changes made till now will be reverted."))==GTK_RESPONSE_YES)
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
		try: if (pacman_trans_init(PM_TRANS_TYPE_REMOVE,
					flags,
					gfpm_progress_event,
					cb_gfpm_trans_conv,
					gfpm_progress_install) == -1)
		{
			gchar *str;
			gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
			str = g_strdup_printf (_("Failed to init transaction (%s)\n"), p_error_utf8);
			errorstr = g_string_append (errorstr, str);
			if (p_error_utf8)
				g_free (p_error_utf8);
			if (pm_errno == PM_ERR_HANDLE_LOCK)
				g_string_printf (errorstr, "%s", lck_error);
			if (gfpm_question (_("Error"), errorstr->str) == GTK_RESPONSE_YES)
			{
				g_remove (PM_LOCK);
				goto try;
			}
			else
			{
				return;
			}
		}

		gfpm_progress_show (TRUE);
		GList *i = NULL;
		gint ret = 0;
		PM_LIST *pdata, *pkgs;
		for (i = (GList*)remove_list; i; i = i->next)
		{
			char *target = i->data;
			pacman_trans_addtarget (target);
			while (gtk_events_pending())
				gtk_main_iteration();
		}
		if (gfpm_trans_prepare(pdata) == -1)
			goto down;
		pkgs = pacman_trans_getinfo (PM_TRANS_PACKAGES);
		if (pkgs == NULL) g_print ("pkgs is null.. bad bad bad!\n");

		#ifdef HAVE_ICMONITOR
		// start iconcache montior
		gfpm_icmonitor_start_monitor ();
		#endif
		
		/* set running flag and prepare to commit transaction */
		running = TRUE;
		ret = gfpm_trans_commit (&pdata);

		down:
		/* release the transaction */
		pacman_trans_release ();
		/* not running */
		running = FALSE;
		/* Ask the user before clearing the package list */
		if ((!ret) || ((ret == -1) && gfpm_question(_("Remember selection"),
					_("GFpm has detected that a failure has occurred. Do you want GFpm to remember your current package selection?")) == GTK_RESPONSE_NO))
		{
			gfpm_package_list_free (GFPM_REMOVE_LIST);
		}
		gfpm_apply_dlg_reset ();
		/* close the progress dialog if commit failed */
		if (ret == -1)
			gfpm_progress_show (FALSE);
	}
	if (gfpm_package_list_is_empty(GFPM_INSTALL_LIST))
	{
		gint flags = 0;
		gint ret = 0;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_apply_inst_depcheck)))
			flags |= PM_TRANS_FLAG_NODEPS;
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_apply_inst_dwocheck)))
			flags |= PM_TRANS_FLAG_DOWNLOADONLY;
		/* create transaction */
itry:	if (pacman_trans_init(PM_TRANS_TYPE_SYNC, flags, gfpm_progress_event, cb_gfpm_trans_conv, gfpm_progress_install) == -1)
		{
			gchar *str;
			gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
			str = g_strdup_printf (_("Failed to init transaction (%s)\n"), p_error_utf8);
			errorstr = g_string_append (errorstr, str);
			if (p_error_utf8)
				g_free (p_error_utf8);
			if (pm_errno == PM_ERR_HANDLE_LOCK)
				g_string_printf (errorstr, "%s", lck_error);
			if (gfpm_question (_("Error"), errorstr->str) == GTK_RESPONSE_YES)
			{
				g_remove (PM_LOCK);
				goto itry;
			}
			else
			{
				return;
			}
		}

		gfpm_progress_show (TRUE);
		GList *i = NULL;
		PM_LIST *pdata, *pkgs;
		pdata = NULL;
		for (i = (GList*)install_list; i; i = i->next)
		{
			char *target = i->data;
			pacman_trans_addtarget (target);
			while (gtk_events_pending())
				gtk_main_iteration();
		}
		if (gfpm_trans_prepare(pdata) == -1)
			goto cleanup;
		pkgs = pacman_trans_getinfo (PM_TRANS_PACKAGES);
		if (pkgs == NULL) gfpm_error (_("Error"), _("Error getting transaction info"));

		#ifdef HAVE_ICMONITOR
		/* start iconcache montior if it wasn't already started */
		if (gfpm_icmonitor_is_running()==FALSE)
			gfpm_icmonitor_start_monitor ();
		#endif
		/* commit transaction */
		running = TRUE;
		ret = gfpm_trans_commit (&pdata);


		cleanup:
		/* release the transaction */
		pacman_trans_release ();
		running = FALSE;

		/* Ask the user before clearing the package list */
		if ((!ret) || ((ret == -1) && gfpm_question(_("Remember selection"),
					_("GFpm has detected that a failure has occurred. Do you want GFpm to remember your current package selection?")) == GTK_RESPONSE_NO))
		{
			gfpm_package_list_free (GFPM_INSTALL_LIST);
		}
		gfpm_apply_dlg_reset ();
		/* hide the progress dialog if commit fails */
		if (ret == -1)
			gfpm_progress_show (FALSE);
	}
	gfpm_db_reset_localdb ();

	#ifdef HAVE_ICMONITOR
	gfpm_icmonitor_stop_monitor ();
	if (gfpm_icmonitor_is_ic_changed() == TRUE)
	{
		gfpm_icmonitor_reset_ic ();
		gfpm_update_iconcache ();
	}
	#endif
	
	if (gfpm_progress_is_autoclose_checkbtn_set())
		gfpm_progress_show (FALSE);
	
	/* Fix for bug #3893 */
	// Quick pane status for the currently selected package is not updated after
	// a package install/remove operation
	// The following code simply reselects the selected package causing the quickpane
	// status to update
	char *sel_pkgname = NULL;
	GtkTreeModel *model = NULL;
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	GtkTreeSelection *selection = NULL;
	
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_pkgs_tvw));	
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	gtk_tree_selection_get_selected (selection, &model, &iter);
	gtk_tree_selection_select_iter (selection, &iter);
	path = gtk_tree_model_get_path (model, &iter);
	
	// Also reload the package info treeview
	if (current_group != NULL)
		gfpm_load_pkgs_tvw ((const char*)current_group);
	gfpm_load_info_tvw (sel_pkgname, gfpm_info_tvw);

	// Okay, now reselect the package
	gtk_tree_selection_select_path (selection, path);
	
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

	if (strcmp(repo_name,"local"))
		db = sync_db;
	else
		db = local_db;

	//display status here
	// clear tvws
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_groups_tvw));
	gtk_list_store_clear (GTK_LIST_STORE(model));

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
		while (gtk_events_pending()) gtk_main_iteration ();
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
	GdkPixbuf	*icon_ln = NULL;
	gboolean	check = FALSE;
	gint		r = 0;
	float		size = 0;
	gchar		*tmp = NULL;
	gboolean	show_compressed = FALSE;
	gboolean	show_uncompressed = FALSE;

	if (!strcmp(repo,"local"))
		pm_db = local_db;
	else
	{
		pm_db = sync_db;
		r = 1;
	}

	show_compressed = gfpm_config_get_value_bool ("show_compressed_size");
	show_uncompressed = gfpm_config_get_value_bool ("show_uncompressed_size");
	gfpm_update_status (_("Loading package list ..."));
	pm_group = pacman_db_readgrp (pm_db, (char*)group_name);
	l = pacman_grp_getinfo (pm_group, PM_GRP_PKGNAMES);
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	icon_yes = gfpm_get_icon (ICON_INSTALLED, 16);
	icon_no = gfpm_get_icon (ICON_NINSTALLED, 16);
	//icon_yes = gtk_widget_render_icon (gfpm_pkgs_tvw, GTK_STOCK_YES, GTK_ICON_SIZE_MENU, NULL);
	//icon_no = gtk_widget_render_icon (gfpm_pkgs_tvw, GTK_STOCK_NO, GTK_ICON_SIZE_MENU, NULL);
	icon_up = gfpm_get_icon (ICON_NEEDUPDATE, 16);
	icon_ln = gfpm_get_icon (ICON_LOCALNEWER, 16);
	while (gtk_events_pending()) gtk_main_iteration ();
	// display status
	for (i=l;i;i=pacman_list_next(i))
	{
		gboolean up = FALSE;
		gboolean ln = FALSE;
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
				gint ret;
				char *v1 = (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION);
				char *v2 = (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION);
				ret = pacman_pkg_vercmp (v1, v2);
				if (!ret)
				{
					up = FALSE;
				}
				else if (ret == -1)
				{
					up = FALSE;
					ln = TRUE;
				}
				else
				{
					up = TRUE;
				}
			}

			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, check,
						1, (up==TRUE)?icon_up:(ln==TRUE)?icon_ln:(check==TRUE)?icon_yes:icon_no,
						2, g_strstrip((char*)pacman_list_getdata (i)),
						3, (check==TRUE)?(char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION) : NULL,
						4, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
						//5, g_strstrip((char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC)),
						-1);
		}
		else if (r == 0)
		{
			pm_pkg = pacman_db_readpkg (sync_db, pacman_list_getdata(i));
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			char *v1 = (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION);
			char *v2 = (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION);
			if (v1!=NULL && v2!=NULL)
			{
				gint ret = pacman_pkg_vercmp (v1, v2);
					if (!ret)
						up = FALSE;
					else if (ret == -1)
					{
						up = FALSE;
						ln = TRUE;
					}
					else
					{
						up = TRUE;
					}
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
		pm_pkg = pacman_db_readpkg (sync_db, g_strstrip((char*)pacman_list_getdata(i)));
		
		size = (float)((long)pacman_pkg_getinfo (pm_pkg, PM_PKG_SIZE)/1024)/1024;
		asprintf (&tmp, "%0.2f MB", size);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
							COL_PKG_SIZE_COMPRESSED, tmp,
						-1);
		g_free (tmp);
		
		size = (float)((long)pacman_pkg_getinfo (pm_pkg, PM_PKG_USIZE)/1024)/1024,
		asprintf (&tmp, "%0.2f MB", size);
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					COL_PKG_SIZE_UNCOMPRESSED, tmp,
					-1);
		g_free (tmp);

		pacman_pkg_free (pm_pkg);
		pacman_pkg_free (pm_lpkg);
		while (gtk_events_pending()) gtk_main_iteration ();
	}
	gfpm_update_status (_("Loading package list ...DONE"));

	g_object_unref (icon_yes);
	g_object_unref (icon_no);
	g_object_unref (icon_up);
	g_object_unref (icon_ln);

	return;
}

void
gfpm_load_info_tvw (const char *pkg_name, GtkTreeView *tvw)
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
	gboolean	pkg_is_file = FALSE;

	if (!pkg_name)
		return;
	
	/* check if it's a package file because we don't need any repo checking for a fpm */
	if (pkg_name[0] != '/')
	{
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
	}
	else
	{
		if (g_file_test(pkg_name,G_FILE_TEST_EXISTS))
		{
			if (pacman_pkg_load((char*)pkg_name,&pm_pkg))
			{
				gfpm_error (_("Error"), _("Error loading package information from file"));		
				return;
			}
			pkg_is_file = TRUE;
		}
	}

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(tvw));
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
	if (pkg_is_file || inst == TRUE)
	{
		temp = pacman_pkg_getinfo (pm_lpkg, PM_PKG_LICENSE);
		str = g_string_new ("");
		for (i=temp;i;i=pacman_list_next(i))
		{
			str = g_string_append (str, (char*)pacman_list_getdata(i));
			str = g_string_append (str, " ");
		}
		if (str->len > 1)
		{
			st = (char*)gfpm_bold (_("License:"));
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, st,
						1, (char*)str->str,
						-1);
			g_free (st);
			g_string_free (str, TRUE);
		}
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

	/* populate replaces */
	temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_REPLACES);
	str = g_string_new ("");
	for (i=temp;i;i=pacman_list_next(i))
	{
		str = g_string_append (str, (char*)pacman_list_getdata(i));
		str = g_string_append (str, " ");
	}
	if (str->len)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("Replaces:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)str->str,
					-1);
		g_free (st);
	}
	g_string_free (str, TRUE);
	
	if (pkg_is_file || inst == TRUE)
	{
		PM_PKG *pkgt = NULL;
		
		if (pkg_is_file)
			pkgt = pm_pkg;
		else
			pkgt = pm_lpkg;
		
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("URL:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)pacman_pkg_getinfo (pkgt, PM_PKG_URL),
					-1);
		g_free (st);
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("Packager:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)pacman_pkg_getinfo (pkgt, PM_PKG_PACKAGER),
					-1);
		g_free (st);
		if (!pkg_is_file)
		{
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			st = (char*)gfpm_bold (_("Install Date:"));
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
						0, st,
						1, (char*)pacman_pkg_getinfo (pkgt, PM_PKG_INSTALLDATE),
						-1);
			g_free (st);
		}
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		size = (float)((long)pacman_pkg_getinfo (pkgt, PM_PKG_SIZE)/1024)/1024;
		asprintf (&tmp, "%0.2f MB", size);
		st = (char*)gfpm_bold (_("Size:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)tmp,
					-1);
		g_free (st);
		g_free (tmp);
	}
	if (inst == FALSE && !pkg_is_file)
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
	if (r == 1 && !pkg_is_file)
	{
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		st = (char*)gfpm_bold (_("SHA1SUM:"));
		gtk_list_store_set (GTK_LIST_STORE(model), &iter,
					0, st,
					1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_SHA1SUM),
					-1);
		g_free (st);
	}
	if (inst == TRUE || pkg_is_file)
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

		if (pkg_is_file)
		{
			pm_lpkg = pm_pkg;
		}
		st = (char*)gfpm_bold (_("Reason:"));
		gtk_list_store_append (GTK_LIST_STORE(model), &iter);
		switch ((long)pacman_pkg_getinfo (pm_lpkg, PM_PKG_REASON))
		{
			case PM_PKG_REASON_EXPLICIT:
					gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								0, st,
								1, _("Explicitly Installed"),
								-1);
					break;
			case PM_PKG_REASON_DEPEND:
					gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								0, st,
								1, _("Installed as a dependency for another package"),
								-1);
					break;
			default:
					break;
		}
		g_free (st);
	}
	
	if (pkg_is_file && pm_pkg)
	{
		pacman_pkg_free (pm_pkg);
	}
	else if (pm_lpkg)
	{
		pacman_pkg_free (pm_lpkg);
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

void
gfpm_load_changelog_txtvw (const char *pkg_name, gboolean inst)
{
	GtkTextBuffer	*buffer;
	GtkTextIter	iter;
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(gfpm_clog_txtvw));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);

	if (inst == TRUE)
	{
		PM_PKG *pkg = NULL;
		long dbpath;
		gchar logpath[PATH_MAX];
		FILE *fp = NULL;
		gchar line[PATH_MAX+1];
		pkg = pacman_db_readpkg (local_db, (char*)pkg_name);
		pacman_get_option (PM_OPT_DBPATH, &dbpath);
		snprintf (logpath, PATH_MAX, "/%s/%s/%s-%s/changelog",
				(char*)dbpath,
				(char*)pacman_db_getinfo (local_db, PM_DB_TREENAME),
				(char*)pacman_pkg_getinfo (pkg, PM_PKG_NAME),
				(char*)pacman_pkg_getinfo (pkg, PM_PKG_VERSION));
		pacman_pkg_free (pkg);
		if ((fp = fopen(logpath, "r")) == NULL)
		{
			gtk_text_buffer_insert (buffer, &iter, _("No changelog available for this package"), -1);
		}
		else
		{
			while (!feof(fp))
			{
				fgets (line, PATH_MAX, fp);
				gtk_text_buffer_insert (buffer, &iter, line, -1);
				line[0] = 0;
			}
			fclose (fp);
		}
	}
	else
	{
		gtk_text_buffer_insert (buffer, &iter, _("Package is not installed"), -1);
	}

	return;
}

static gint
gfpm_trans_prepare (PM_LIST *list)
{
	if (pacman_trans_prepare(&list)==-1)
	{
		PM_LIST *i;
		GList	*pkgs = NULL;
		gchar	*str = NULL;
		gchar	*p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
		str = g_strdup_printf (_("Failed to prepare transaction (%s)\n"), p_error_utf8);
		gfpm_error (_("Error"), str);
		g_free (str);
		g_free (p_error_utf8);
		int t = (long)pacman_trans_getinfo (PM_TRANS_TYPE);
		switch ((long)pm_errno)
		{
			case PM_ERR_UNSATISFIED_DEPS:
				for (i=pacman_list_first(list);i;i=pacman_list_next(i))
				{
					GString	*depstring = g_string_new ("");
					PM_DEPMISS *m = pacman_list_getdata (i);
					gchar *val = NULL;
					if (t == PM_TRANS_TYPE_REMOVE)
					{
						val = g_strdup_printf ("%s : %s %s",
									(char*)pacman_dep_getinfo (m, PM_DEP_TARGET),
									_("is required by"),
									(char*)pacman_dep_getinfo (m, PM_DEP_NAME));
						depstring = g_string_append (depstring, val);
					}
					else
					{
						depstring = g_string_append (depstring, (char*)pacman_dep_getinfo(m,PM_DEP_TARGET));
						depstring = g_string_append (depstring, _(" : requires "));
						depstring = g_string_append (depstring, (char*)pacman_dep_getinfo(m,PM_DEP_NAME));
						switch ((long)pacman_dep_getinfo(m, PM_DEP_MOD))
						{
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
					}
					pkgs = g_list_append (pkgs, (char*)g_strdup(depstring->str));
					g_free (val);
					g_string_free (depstring, FALSE);
				}
				pacman_list_free (list);
				if ((t == PM_TRANS_TYPE_ADD) || (t == PM_TRANS_TYPE_UPGRADE))
					gfpm_plist_message (_("Unsatisfied dependencies"), _("Could not satisfy the following dependencies."), GTK_MESSAGE_WARNING, pkgs);
				else
					gfpm_plist_message (_("Unsatisfied dependencies"), _("The packages you are trying to remove are required by the following packages:"), GTK_MESSAGE_WARNING, pkgs);
				break;
			case PM_ERR_CONFLICTING_DEPS:
				for (i=pacman_list_first(list);i;i=pacman_list_next(i))
				{
					GString	*depstring = g_string_new ("");
					PM_DEPMISS *m = pacman_list_getdata (i);
					depstring = g_string_append (depstring, (char*)pacman_dep_getinfo(m, PM_DEP_NAME));
					pkgs = g_list_append (pkgs, (char*)depstring->str);
					g_string_free (depstring, FALSE);
				}
				pacman_list_free (list);
				gfpm_plist_message (_("Package conflict"), _("This package conflicts with the following packages"), GTK_MESSAGE_WARNING, pkgs);
				break;
		}
		return -1;
	}
	else
	{
		return 0;
	}
}


static gint
gfpm_trans_commit (PM_LIST **list)
{
	int error; 
	
	error = pacman_trans_commit (list);
	if (error == -1)
	{
		PM_LIST *i;
		GList	*pkgs = NULL;
		gchar	*str = NULL;
		gchar	*p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
		str = g_strdup_printf (_("Failed to commit transaction (%s)\n"), p_error_utf8);
		gfpm_error (_("Error"), str);
		g_free (str);
		g_free (p_error_utf8);
		
		switch ((long)pm_errno)
		{
			case PM_ERR_FILE_CONFLICTS:
			{
				for (i=pacman_list_first(*list);i;i=pacman_list_next(i))
				{
					PM_CONFLICT *cnf = pacman_list_getdata (i);
					switch ((long)pacman_conflict_getinfo(cnf,PM_CONFLICT_TYPE))
					{
						case PM_CONFLICT_TYPE_FILE:
						{
							gchar* cstr = g_strdup_printf ("%s: /%s",
											(char*)pacman_conflict_getinfo (cnf, PM_CONFLICT_TARGET),
											(char*)pacman_conflict_getinfo (cnf, PM_CONFLICT_FILE));
							pkgs = g_list_append (pkgs, cstr);
						}
					}
				}
				gfpm_plist_message (_("Conflicting Files"),
						_("The file(s) provided by the following package(s) already exist on the system"),
						GTK_MESSAGE_WARNING,
						pkgs);
			}
			case PM_ERR_PKG_CORRUPTED:
			{
				for (i=pacman_list_first(*list);i;i=pacman_list_next(i))
					pkgs = g_list_append (pkgs, g_strdup (pacman_list_getdata(i)));
				gfpm_plist_message (_("Corrupted package(s)"),
							_("The package(s) you're trying to install are corrupted"),
							GTK_MESSAGE_ERROR,
							pkgs);
			}
			case PM_ERR_RETRIEVE:
			{
				gfpm_error (_("Download Failed"), _("Failed to retrieve packages."));
			}
		}
		pacman_trans_release ();
		return -1;
	}
	else
	{
		return 0;
	}
}

void
gfpm_pkgs_show_compressed_size (gboolean check)
{
	gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(gfpm_compressed_size_col),
					check);	
}

void
gfpm_pkgs_show_uncompressed_size (gboolean check)
{
	gtk_tree_view_column_set_visible (GTK_TREE_VIEW_COLUMN(gfpm_uncompressed_size_col),
					check);
}

/* CALLBACKS */

static void
cb_gfpm_refresh_button_clicked (GtkButton *button, gpointer data)
{
	gint 	ret;
	GList	*dbs = NULL;
	PM_LIST *packages;
	gchar 	*updatestr = NULL;

	gfpm_progress_set_main_text (_("Synchronizing package databases"), 1);
	gfpm_progress_show (TRUE);
	dbs = gfpm_db_get_repolist ();
	while (dbs != NULL)
	{
		PM_DB *tdb = dbs->data;
		ret = pacman_db_update (0, tdb);
		dbs = g_list_next (dbs);
	}
	gfpm_progress_show (FALSE);
	/* check for a pacman-g2 update */
	if (gfpm_check_if_package_updatable ("pacman-g2"))
	{
		updatestr = g_strdup_printf (_("Gfpm has detected a newer version of the pacman-g2 package. It is recommended that you allow gfpm to update pacman-g2 first. Do you wish to continue upgrading pacman-g2 ?"));
		if (gfpm_question (_("Update pacman-g2"), updatestr) == GTK_RESPONSE_YES)
		{
				gfpm_package_list_add (GFPM_INSTALL_LIST, "pacman-g2");
				cb_gfpm_apply_btn_clicked (NULL, NULL);
				pacman_trans_release ();
		}
		g_free (updatestr);
	}
	/* check for a gfpm update */
	if (gfpm_check_if_package_updatable ("gfpm"))
	{
		updatestr = g_strdup_printf (_("Gfpm has detected a newer version of the gfpm package. It is recommended that you allow gfpm to update itself first. Do you wish to continue upgrading gfpm ?"));
		if (gfpm_question (_("Update Gfpm"), updatestr) == GTK_RESPONSE_YES)
		{
				gfpm_package_list_add (GFPM_INSTALL_LIST, "gfpm");
				cb_gfpm_apply_btn_clicked (NULL, NULL);
				pacman_trans_release ();
		}
		g_free (updatestr);
	}
	
	if (pacman_trans_init(PM_TRANS_TYPE_SYNC, 0, gfpm_progress_event, cb_gfpm_trans_conv, gfpm_progress_install) == -1)
	{
		gchar *str;
		GString *errorstr = g_string_new ("");
		gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
		str = g_strdup_printf (_("Failed to init transaction (%s)\n"), p_error_utf8);
		errorstr = g_string_append (errorstr, str);
		if (pm_errno == PM_ERR_HANDLE_LOCK)
			errorstr = g_string_append (errorstr,
						_("If you're sure a package manager is not already running, you can delete /tmp/pacman-g2.lck"));
		gfpm_error (_("Error"), errorstr->str);
		g_free (p_error_utf8);
		return;
	}
	if (pacman_trans_sysupgrade()==-1)
	{
		gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
		g_print ("Error: %s\n", p_error_utf8);
		g_free (p_error_utf8);
	}
	packages = pacman_trans_getinfo (PM_TRANS_PACKAGES);

	if (packages == NULL)
	{
		gfpm_message (_("No updates"), _("No new package updates are available. The system is up to date."));
		goto cleanup;
	}

	if (gfpm_plist_question(_("Package upgrade"), _("Following packages will be upgraded. Do you want to continue ?"), gfpm_pmlist_to_glist(packages)) == GTK_RESPONSE_YES)
	{
		pacman_trans_release ();
		cb_gfpm_apply_btn_clicked (NULL, NULL);
	}

cleanup:
	pacman_trans_release ();

	return;
}

static void
cb_gfpm_repos_combo_changed (GtkComboBox *combo, gpointer data)
{
	gchar *text = NULL;

	text = gtk_combo_box_get_active_text (combo);
	if (text == NULL) return;
	if (!strcmp(text, _("Installed Packages")))
	{
		g_free (text);
		text = g_strdup ("local");
	}
	gfpm_db_register (text);
	gfpm_load_groups_tvw (text);
	g_free (text);

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
		g_free (current_group);
		current_group = g_strdup (group);
		gfpm_load_pkgs_tvw (current_group);
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
	gchar		*v1 = NULL;
	gchar		*v2 = NULL;
	gboolean	inst = FALSE;
	gboolean	up = FALSE;

	/* free the old quickpane package name */
	if (quickpane_pkg != NULL)
	{
		g_free (quickpane_pkg);
		quickpane_pkg = NULL;
	}
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 2, &pkgname, 3, &v1, 4, &v2, -1);
		quickpane_pkg = g_strdup (pkgname);
		gfpm_load_info_tvw (pkgname,(GtkTreeView*)gfpm_info_tvw);
		if (v1 != NULL)
			inst = TRUE;
		gfpm_load_files_txtvw (pkgname, inst);
		gfpm_load_changelog_txtvw (pkgname, inst);

		if (v1!=NULL && v2!=NULL)
		{
			gint ret = pacman_pkg_vercmp (v1, v2);
			if (!ret)
				up = FALSE;
			else if (ret < 0)
				up = TRUE;
			else
				up = FALSE;
			if (!strlen((char*)v1))
				up = FALSE;
		}
		else
		{
			up = FALSE;
		}
		gfpm_quickpane_show (TRUE, inst, up);
		/* show the 'View Readme' button for packages that have a README.Frugalware */
		if (inst == TRUE)
		{
			char *readme_path = NULL;
			/* strip the pkgrel from version string */
			char *ver = g_strdup (v1);
			char *pkgver;
			pkgver = strrchr (ver, '-');
			*pkgver = 0;
			/* generate the path for the README.Frugalware file */
			readme_path = g_strdup_printf ("/usr/share/doc/%s-%s/README.Frugalware", pkgname, ver);
			if (g_file_test (readme_path, G_FILE_TEST_EXISTS))
			{
				gfpm_quickpane_readme_btn_show ();
				gfpm_quickpane_readme_dlg_populate (readme_path);
			}
			g_free (readme_path);
			g_free (ver);
		}
	}
	else
	{
		g_free (pkgname);
	}

	return;
}

static void
cb_gfpm_groups_tvw_right_click (GtkTreeView *treeview, GdkEventButton *event)
{
	GtkWidget 		*menu;
	GtkWidget 		*menu_item;
	GtkTreeModel 		*model;
	GtkTreeSelection 	*selection;
	GtkTreeIter 		iter;
	gchar 			*groupname = NULL;
	
	if (event->button != 3)
		return;
		
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_groups_tvw));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_groups_tvw));
	if (FALSE == gtk_tree_selection_get_selected(selection, &model, &iter))
		return;
	gtk_tree_model_get (model, &iter, 0, &groupname, -1);
	menu = gtk_menu_new ();
	menu_item = gtk_image_menu_item_new_from_stock ("gtk-remove", NULL);
	g_signal_connect (G_OBJECT(menu_item), "activate", G_CALLBACK(cb_gfpm_remove_group_clicked), (gpointer)groupname);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);
	gtk_widget_show (menu_item);
	gtk_widget_show (menu);
	gtk_menu_popup (GTK_MENU(menu),
			NULL,
			NULL,
			NULL,
			NULL,
			3,
			gtk_get_current_event_time());
	
	return;
}

static void
cb_gfpm_pkgs_tvw_right_click (GtkTreeView *treeview, GdkEventButton *event)
{
	GtkWidget 		*menu;
	GtkWidget 		*menu_item;
	GtkWidget		*image;
	GtkTreeModel		*model = NULL;
	GtkTreeSelection	*selection = NULL;
	GtkTreeIter		iter;
	char			*iversion = NULL;
	char			*lversion = NULL;
	char			*pkgname = NULL;
	gboolean		up = FALSE;
	gboolean		inst = TRUE;

	if (event->button != 3)
		return;

	if (geteuid() != 0)
		return;

	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	if ( FALSE == gtk_tree_selection_get_selected (selection, &model, &iter) )
		return;
	gtk_tree_model_get (model, &iter, 2, &pkgname, 3, &iversion, 4, &lversion, -1);
	menu = gtk_menu_new ();
	
	menu_item = gtk_image_menu_item_new_with_label (_("Mark for upgrade"));
	image = gtk_image_new_from_stock ("gtk-apply", GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menu_item), image);
	g_signal_connect (G_OBJECT(menu_item), "activate", G_CALLBACK(cb_gfpm_mark_for_upgrade), (gpointer)pkgname);
	gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);
	if (lversion!=NULL && iversion!=NULL)
	{
		gint ret = pacman_pkg_vercmp (lversion, iversion);
		if (!ret)
			up = FALSE;
		else if (ret > 0)
			up = TRUE;
		else
			up = FALSE;
			
		inst = TRUE;
	}
	else
	{
		up = FALSE;
		inst = FALSE;
	}
	if (up)
		gtk_widget_show (menu_item);
	else
		gtk_widget_hide (menu_item);
	if (inst == FALSE)
	{
		menu_item = gtk_image_menu_item_new_with_label (_("Mark for installation"));
		image = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menu_item), image);
		g_signal_connect (G_OBJECT(menu_item), "activate", G_CALLBACK(cb_gfpm_mark_for_install), (gpointer)pkgname);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);
		gtk_widget_show (menu_item);
	}
	else
	{
		menu_item = gtk_image_menu_item_new_with_label (_("Reinstall package"));
		image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menu_item), image);
		g_signal_connect (G_OBJECT(menu_item), "activate", G_CALLBACK(cb_gfpm_reinstall), (gpointer)pkgname);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);
		gtk_widget_show (menu_item);
		
		menu_item = gtk_image_menu_item_new_with_label (_("Mark for re-installation"));
		image = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menu_item), image);
		g_signal_connect (G_OBJECT(menu_item), "activate", G_CALLBACK(cb_gfpm_mark_for_reinstall), (gpointer)pkgname);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);
		gtk_widget_show (menu_item);

		menu_item = gtk_image_menu_item_new_with_label (_("Mark for removal"));
		image = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_MENU);
		gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM(menu_item), image);
		g_signal_connect (G_OBJECT(menu_item), "activate", G_CALLBACK(cb_gfpm_mark_for_removal), (gpointer)pkgname);
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), menu_item);
		gtk_widget_show (menu_item);
	}
	gtk_widget_show (menu);
	gtk_menu_popup (GTK_MENU(menu),
			NULL,
			NULL,
			NULL,
			NULL,
			3,
			gtk_get_current_event_time());

	return;
}

static void
cb_gfpm_mark_for_install (GtkButton *button, gpointer data)
{
	char *pkgname = NULL;

	pkgname = (char*) data;	
	gfpm_package_list_add (GFPM_INSTALL_LIST, pkgname);

	return;
}

static void
cb_gfpm_mark_for_reinstall (GtkButton *button, gpointer data)
{
	gfpm_package_list_add (GFPM_INSTALL_LIST, (char*)data);

	return;
}

static void
cb_gfpm_reinstall (GtkButton *button, gpointer data)
{
	GfpmList *temp = NULL;

	temp = install_list;
	install_list = NULL;
	gfpm_package_list_add (GFPM_INSTALL_LIST, (char*)data);
	cb_gfpm_apply_btn_clicked (NULL, NULL);
	install_list = temp;

	return;
}

static void
cb_gfpm_mark_for_removal (GtkButton *button, gpointer data)
{
	char *pkgname;

	pkgname = (char*) data;	
	gfpm_package_list_add (GFPM_REMOVE_LIST, pkgname);

	return;
}

static void
cb_gfpm_mark_for_upgrade (GtkButton *button, gpointer data)
{
	char *pkgname;

	pkgname = (char*) data;	
	gfpm_package_list_add (GFPM_INSTALL_LIST, pkgname);

	return;
}

static void
cb_gfpm_remove_group_clicked (GtkButton *button, gpointer data)
{
	PM_GRP		*group = NULL;
	PM_LIST 	*list = NULL;
	PM_LIST 	*i = NULL;
	const char	*group_name = (char*) data;
	
	group = pacman_db_readgrp (sync_db, (char*)group_name);
	list = pacman_grp_getinfo (group, PM_GRP_PKGNAMES);
	if (list == NULL)
	{
		gfpm_error (_("Error"), _("There was an error while populating packages"));
		return;
	}
	gfpm_package_list_free (GFPM_REMOVE_LIST);
	for (i = list; i != NULL; i = pacman_list_next(i))
	{
		PM_PKG *lpkg = NULL;
		lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
		if (lpkg != NULL)
			gfpm_package_list_add (GFPM_REMOVE_LIST, pacman_list_getdata(i));
	}
	if (!gfpm_package_list_is_empty(GFPM_REMOVE_LIST))
	{
		gfpm_error (_("Error"), _("There are no installed packages in this group"));
		return;
	}
	if (gfpm_question(_("Remove Group"), _("Are you sure you want to remove the entire group ?")) == GTK_RESPONSE_YES)
		cb_gfpm_apply_btn_clicked (NULL, NULL);
		
	gfpm_package_list_free (GFPM_REMOVE_LIST);
}

static void
cb_gfpm_search_buttonpress (GtkButton *button, gpointer data)
{
	cb_gfpm_search_keypress (GTK_WIDGET(data), NULL, NULL);

	return;
}

static void
cb_gfpm_search_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	GError	*error = NULL;

	if (event!=NULL)
	{
		if (event->keyval != GDK_Return)
			return;
	}
	if (!g_thread_create ((GThreadFunc)gfpm_search,
			(gpointer)widget,
			FALSE,
			&error))
	{
		g_print ("Error creating search thread: %s\n", error->message); 
	}
	else
	{
		g_print ("created search thread\n");
	}
	return;
}

static void
gfpm_search (GtkWidget *widget)
{
	GtkListStore	*store;
	GdkPixbuf	*icon_yes;
	GdkPixbuf	*icon_no;
	GdkPixbuf	*icon_up;
	GdkPixbuf	*icon_ln;
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	PM_PKG		*pm_pkg;
	PM_LIST		*l = NULL;
	PM_LIST		*i = NULL;
	gchar		*search_str;
	gint		r = 0;
	char		*v1 = NULL;
	char		*v2 = NULL;
	gchar		*srepo = NULL;
	PM_DB		*search_db = NULL;
	guint		nounreg = 0;
	
	g_mutex_lock (search_mutex);
	
	icon_yes = gfpm_get_icon (ICON_INSTALLED, 16);
	icon_no = gfpm_get_icon (ICON_NINSTALLED, 16);
	icon_up = gfpm_get_icon (ICON_NEEDUPDATE, 16);
	icon_ln = gfpm_get_icon (ICON_LOCALNEWER, 16);
	
	search_str = (gchar*)gtk_entry_get_text (GTK_ENTRY(widget));
	
	if (search_str == NULL)
	{	
		goto cleanup;
	}
	
	srepo = gtk_combo_box_get_active_text (GTK_COMBO_BOX(gfpm_search_combo));
	
	if (srepo == NULL)
	{
		goto cleanup;
	}
	if (!strcmp(srepo, _("Installed Packages")))
	{
		g_free (srepo);
		srepo = g_strdup ("local");
	}
	/* enter the gdk critical section */
	gdk_threads_enter ();
	
	model = gtk_tree_view_get_model (GTK_TREE_VIEW(gfpm_pkgs_tvw));
	gtk_list_store_clear (GTK_LIST_STORE(model));
	store = (GtkListStore*) model;
	gfpm_update_status (_("Searching for packages ..."));
	
	pacman_set_option (PM_OPT_NEEDLES, (long)search_str);
	if (!strcmp("local", srepo))
	{
		l = pacman_db_search (local_db);
		r = 0;
	}
	else
	{
		if (!strcmp(repo, srepo))
		{
			search_db = sync_db;
			nounreg = 1;
		}
		else
		{
			search_db = pacman_db_register (srepo);
		}	
		l = pacman_db_search (search_db);
		r = 1;
	}
	
	gfpm_update_status (_("Search Complete"));
	
	if (l == NULL)
	{
		gfpm_error (_("Package not found"), _("No such package found"));
		goto cleanup;
	}

	if (r == 0)
	{
		PM_PKG	*pm_spkg;
		PM_PKG	*pm_lpkg;
		gboolean up = FALSE;
		model = gtk_tree_view_get_model ((GtkTreeView*)gfpm_pkgs_tvw);
		store = (GtkListStore*) model;	

		for (i=l;i;i=pacman_list_next(i))
		{
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			pm_spkg = pacman_db_readpkg (sync_db, pacman_list_getdata(i));
			v1 = (char*)pacman_pkg_getinfo (pm_spkg, PM_PKG_VERSION);
			v2 = (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION);
			if (v1!=NULL && v2!=NULL)
			{
				gint ret = pacman_pkg_vercmp (v1, v2);
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

		model = gtk_tree_view_get_model ((GtkTreeView*)gfpm_pkgs_tvw);
		store = (GtkListStore*) model;
		for (i=l;i;i=pacman_list_next(i))
		{
			gboolean	ln = FALSE;
			pm_pkg = pacman_db_readpkg (search_db, pacman_list_getdata(i));
			pm_lpkg = pacman_db_readpkg (local_db, pacman_list_getdata(i));
			if (pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION)!=NULL)
			{
				inst = TRUE;
				v1 = (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION);
				v2 = (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION);
				if (v1!=NULL && v2!=NULL)
				{
					gint ret = pacman_pkg_vercmp (v1, v2);
					if (!ret)
					{
						up = FALSE;
					}
					else if (ret == -1)
					{
						up = FALSE;
						ln = TRUE;
					}
					else
					{
						up = TRUE;
					}
				}
				else
				{
					up = FALSE;
				}
			}
			else
			{
				inst = FALSE;
			}
			gtk_list_store_append (store, &iter);
			if (inst == TRUE)
				gtk_list_store_set (store, &iter, 3, (char*)pacman_pkg_getinfo (pm_lpkg, PM_PKG_VERSION), -1);
			else
				gtk_list_store_set (store, &iter, 3, NULL, -1);

			gtk_list_store_set (store, &iter,
					0, inst,
					1, (inst==TRUE)?(ln==TRUE)?icon_ln:(up==TRUE)?icon_up:icon_yes:icon_no,
					2, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_NAME),
					4, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
					-1);
			pacman_pkg_free (pm_pkg);
			pacman_pkg_free (pm_lpkg);
		}
	}
	
	cleanup:
	pacman_set_option (PM_OPT_NEEDLES, (long)NULL);
	if (search_db!=NULL && !nounreg)
	{
		pacman_db_unregister (search_db);
	}
	
	gfpm_update_status (_("Searching for packages ...DONE"));
	g_object_unref (icon_yes);
	g_object_unref (icon_no);
	g_object_unref (icon_up);
	g_object_unref (icon_ln);
	
	/* leave the gdk critical section */
	gdk_flush ();
	gdk_threads_leave ();
	g_mutex_unlock (search_mutex);

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
			gfpm_package_list_add (GFPM_REMOVE_LIST, pkg);
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

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_clrold_opt)))
	{
		if (gfpm_question(_("Clear package cache"), _("Are you sure you want to remove old packages from cache ?")) == GTK_RESPONSE_YES)
		{
			while (gtk_events_pending())
				gtk_main_iteration ();
			ret = pacman_sync_cleancache (0);
			if (!ret)
			{
				gfpm_message (_("Cache cleared"), _("Finished clearing the cache"));
				gtk_widget_hide ((GtkWidget*)data);
			}
			else
			{
				gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
				errstr = g_strdup_printf (_("Failed to clean the cache (%s)"), p_error_utf8);
				gfpm_error (_("Error clearing cache"), errstr);
				g_free (errstr);
				g_free (p_error_utf8);
			}
		}
		return;
	}
	else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_clrall_opt)))
	{
		if (gfpm_question(_("Clear package cache"), _("Are you sure you want to remove all packages from cache ?")) == GTK_RESPONSE_YES)
		{
			while (gtk_events_pending())
				gtk_main_iteration ();
			ret = pacman_sync_cleancache (1);
			if (!ret)
			{
				gfpm_message (_("Cache cleared"), _("Finished clearing the cache"));
				gtk_widget_hide ((GtkWidget*)data);
			}
			else
			{
				gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
				errstr = g_strdup_printf (_("Failed to clean the cache (%s)"), p_error_utf8);
				gfpm_error (_("Error clearing cache"), errstr);
				g_free (errstr);
				g_free (p_error_utf8);
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
		gfpm_error (_("No packages selected"), _("No package selected for installation. Please select a package to install."));
		return;
	}
	if (gfpm_question(_("Install package"), _("Are you sure you want to install this package ?")) != GTK_RESPONSE_YES)
		goto quit;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_inst_upgcheck)))
		type = PM_TRANS_TYPE_UPGRADE;
	else
		type = PM_TRANS_TYPE_ADD;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_inst_depcheck)))
		flags |= PM_TRANS_FLAG_NODEPS;
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gfpm_inst_forcheck)))
		flags |= PM_TRANS_FLAG_FORCE;
	if (pacman_trans_init(type, flags, gfpm_progress_event, cb_gfpm_trans_conv, gfpm_progress_install) == -1)
	{
			gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
			str = g_strdup_printf (_("Failed to init transaction (%s)\n"), p_error_utf8);
			errorstr = g_string_append (errorstr, str);
			if (pm_errno == PM_ERR_HANDLE_LOCK)
			{	errorstr = g_string_append (errorstr,
							_("If you're sure a package manager is not already running, you can delete /tmp/pacman-g2.lck"));
				gfpm_error (_("Error"), errorstr->str);
				g_free (p_error_utf8);
			}
			return;
	}
	gfpm_progress_show (TRUE);
	/* add the target */
	if (pacman_trans_addtarget((char*)fpm)==-1)
	{
		gfpm_progress_show (FALSE);
		gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
		str = g_strdup_printf (_("Failed to add target (%s)\n"), p_error_utf8);
		gfpm_error (_("Error"), str);
		g_free (str);
		g_free (p_error_utf8);
		goto cleanup;
	}
	if (gfpm_trans_prepare(trans_data) == -1)
	{	
		gfpm_progress_show (FALSE);
		goto cleanup;
	}
	if (pacman_trans_commit(&trans_data) == -1)
	{
		gchar *p_error_utf8 = gfpm_convert_to_utf8 (pacman_strerror(pm_errno));
		str = g_strdup_printf (_("Failed to commit transaction (%s)\n"), p_error_utf8);
		gfpm_error (_("Error"), str);
		g_free (str);
		g_free (p_error_utf8);
		goto cleanup;
	}
	else
	{
		gfpm_message (_("Success"), _("Package successfully installed"));
	}

	cleanup:
	g_string_free (errorstr, FALSE);
	pacman_trans_release ();
	gtk_widget_hide (gfpm_inst_from_file_dlg);
	if (gfpm_progress_is_autoclose_checkbtn_set())
		gfpm_progress_show (FALSE);
	quit:
	if (garg == ARG_ADD)
	{
		gtk_main_quit();
	}

	return;
}

static void
cb_gfpm_install_file_close_clicked (GtkButton *button, gpointer data)
{
	if (garg != ARG_ADD)
	{
		gtk_widget_hide (gfpm_inst_from_file_dlg);
	}
	else
	{
		gtk_main_quit ();
	}

	return;
}

static void
cb_gfpm_install_file_selection_changed (GtkFileChooser *chooser, gpointer data)
{
	char *file = NULL;

	file = gtk_file_chooser_get_filename (chooser);
	gfpm_load_info_tvw (file, (GtkTreeView*)gfpm_inst_infotvw);

	/* show the info */
	gtk_widget_show (gfpm_inst_infoframe);
}


