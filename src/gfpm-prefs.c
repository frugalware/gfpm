/*
 *  gfpm-prefs.c for gfpm
 *
 *  Copyright (C) 2008-2009 by Priyank Gosalia <priyankmg@gmail.com>
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
#include "gfpm-repomanager.h"
#include "gfpm-logviewer.h"
#include "gfpm-config.h"

static GtkWidget *gfpm_prefs_log_check;
static GtkWidget *gfpm_prefs_log_location;
static GtkWidget *gfpm_prefs_cache_dir_entry;
static GtkWidget *gfpm_prefs_database_path_entry;
static GtkWidget *gfpm_prefs_maxtries_spin;
static GtkWidget *gfpm_prefs_proxy_enable_check;
static GtkWidget *gfpm_prefs_proxy_server_entry;

static GtkWidget *gfpm_prefs_holdpkg_tvw;
static GtkWidget *gfpm_prefs_ignorepkg_tvw;

static GList *gfpm_prefs_holdpkg_list = NULL;
static GList *gfpm_prefs_ignorepkg_list = NULL;
static gchar *gfpm_prefs_logfile_path = NULL;
static gchar *gfpm_prefs_database_path = NULL;
static gchar *gfpm_prefs_cache_dir = NULL;
static gchar *gfpm_prefs_proxy_server = NULL;
static guint gfpm_prefs_max_tries = 0;

extern gchar *current_group;

static void gfpm_prefs_populate_holdpkg (void);
static void gfpm_prefs_populate_ignorepkg (void);
static void gfpm_prefs_populate_holdpkg_tvw (void);
static void gfpm_prefs_populate_ignorepkg_tvw (void);
static gboolean gfpm_prefs_write_config (void);
static char *gfpm_prefs_get_value_for_key (const char *key);

static void cb_gfpm_prefs_holdpkg_add_btn_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_prefs_ignorepkg_add_btn_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_prefs_holdpkg_remove_btn_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_prefs_ignorepkg_remove_btn_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_prefs_edit_logpath_btn_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_prefs_edit_cachedir_btn_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_prefs_edit_database_btn_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_prefs_compressed_size_toggled (GtkToggleButton *button, gpointer data);
static void cb_gfpm_prefs_uncompressed_size_toggled (GtkToggleButton *button, gpointer data);
static void cb_gfpm_prefs_logging_enable_toggled (GtkToggleButton *button, gpointer data);
static void cb_gfpm_prefs_proxy_enable_toggled (GtkToggleButton *button, gpointer data);
static void cb_gfpm_prefs_maxtries_value_changed (GtkSpinButton *button, gpointer data);

#define DEFAULT_CACHEDIR	"/var/cache/pacman"
#define DEFAULT_DBPATH		"/var/lib/pacman"

void
gfpm_prefs_init (void)
{
	GtkListStore		*store = NULL;
	GtkCellRenderer		*renderer = NULL;
	GtkTreeViewColumn	*column = NULL;
	char			*temp = NULL;
	
	gfpm_prefs_holdpkg_tvw = gfpm_get_widget ("gfpm_prefs_holdpkg_tvw");
	gfpm_prefs_ignorepkg_tvw = gfpm_get_widget ("gfpm_prefs_ignorepkg_tvw");
	gfpm_prefs_log_check = gfpm_get_widget ("prefs_enable_log_tgl");
	gfpm_prefs_log_location = gfpm_get_widget ("prefs_log_file_path");
	gfpm_prefs_cache_dir_entry = gfpm_get_widget ("prefs_cache_dir_path");
	gfpm_prefs_database_path_entry = gfpm_get_widget ("prefs_db_path");
	gfpm_prefs_proxy_enable_check = gfpm_get_widget ("prefs_proxy_enable_chk");
	gfpm_prefs_proxy_server_entry = gfpm_get_widget ("prefs_proxy_server_entry");
	
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
	
	/* load default values */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gfpm_get_widget("prefs_show_compressed_size_tgl")),
					gfpm_config_get_value_bool("show_compressed_size"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gfpm_get_widget("prefs_show_uncompressed_size_tgl")),
					gfpm_config_get_value_bool("show_uncompressed_size"));

	/* Log file related information */
	gfpm_prefs_logfile_path = gfpm_prefs_get_value_for_key ("LogFile");
	if (gfpm_prefs_logfile_path)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gfpm_prefs_log_check),TRUE);
		gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_log_location), gfpm_prefs_logfile_path);
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gfpm_prefs_log_check),FALSE);
		gtk_widget_set_sensitive (gfpm_prefs_log_location, FALSE);
	}
	
	/* get package database path */
	gfpm_prefs_database_path = gfpm_prefs_get_value_for_key ("DBPath");
	if (gfpm_prefs_database_path == NULL)
	{
		gfpm_prefs_database_path = g_strdup (DEFAULT_DBPATH);
	}
	gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_database_path_entry), gfpm_prefs_database_path);
	
	/* get package cache path */
	gfpm_prefs_cache_dir = gfpm_prefs_get_value_for_key ("CacheDir");
	if (gfpm_prefs_cache_dir == NULL)
	{
		gfpm_prefs_cache_dir = g_strdup (DEFAULT_CACHEDIR);
	}
	gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_cache_dir_entry), gfpm_prefs_cache_dir);

	/* get max tries */
	gfpm_prefs_maxtries_spin = gfpm_get_widget ("prefs_max_retries");
	temp = gfpm_prefs_get_value_for_key ("MaxTries");
	if (temp)
	{
		gfpm_prefs_max_tries = atoi (temp);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON(gfpm_prefs_maxtries_spin), gfpm_prefs_max_tries);
		g_free (temp);
	}
	
	/* get proxy info */
	gfpm_prefs_proxy_server = gfpm_prefs_get_value_for_key ("ProxyServer");
	if (gfpm_prefs_proxy_server)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(gfpm_prefs_proxy_enable_check), TRUE);
		gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_proxy_server_entry), gfpm_prefs_proxy_server);
	}
	else
	{
		gtk_widget_set_sensitive (gfpm_prefs_proxy_server_entry, FALSE);
	}
	
	/* some signals */
	g_signal_connect (G_OBJECT(gfpm_get_widget("prefs_holdpkg_add_btn")), "clicked",
			G_CALLBACK(cb_gfpm_prefs_holdpkg_add_btn_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_get_widget("prefs_ignorepkg_add_btn")), "clicked",
			G_CALLBACK(cb_gfpm_prefs_ignorepkg_add_btn_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_get_widget("prefs_holdpkg_remove_btn")), "clicked",
			G_CALLBACK(cb_gfpm_prefs_holdpkg_remove_btn_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_get_widget("prefs_ignorepkg_remove_btn")), "clicked",
			G_CALLBACK(cb_gfpm_prefs_ignorepkg_remove_btn_clicked), NULL);
	g_signal_connect (gfpm_get_widget("prefs_show_compressed_size_tgl"), "toggled",
			G_CALLBACK(cb_gfpm_prefs_compressed_size_toggled), NULL);
	g_signal_connect (gfpm_get_widget("prefs_show_uncompressed_size_tgl"), "toggled",
			G_CALLBACK(cb_gfpm_prefs_uncompressed_size_toggled), NULL);
	g_signal_connect (gfpm_get_widget("prefs_enable_log_tgl"), "toggled",
			G_CALLBACK(cb_gfpm_prefs_logging_enable_toggled), NULL);
	g_signal_connect (gfpm_prefs_proxy_enable_check, "toggled",
			G_CALLBACK(cb_gfpm_prefs_proxy_enable_toggled), NULL);
	g_signal_connect (gfpm_prefs_maxtries_spin, "value-changed",
			G_CALLBACK(cb_gfpm_prefs_maxtries_value_changed), NULL);
	g_signal_connect (G_OBJECT(gfpm_get_widget("prefs_logpath_edit_btn")), "clicked",
			G_CALLBACK(cb_gfpm_prefs_edit_logpath_btn_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_get_widget("prefs_database_edit_btn")), "clicked",
			G_CALLBACK(cb_gfpm_prefs_edit_database_btn_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_get_widget("prefs_cache_edit_btn")), "clicked",
			G_CALLBACK(cb_gfpm_prefs_edit_cachedir_btn_clicked), NULL);

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
	{
		g_list_free (gfpm_prefs_holdpkg_list);
		gfpm_prefs_holdpkg_list = NULL;
	}
	
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

static gboolean
gfpm_prefs_write_config (void)
{
	FILE	*fp = NULL;
	FILE	*tp = NULL;
	char	line[PATH_MAX+1];
	
	gboolean has_logfile = FALSE;
	gboolean has_cachedir = FALSE;
	gboolean has_dbpath = FALSE;
	gboolean has_holdpkg = FALSE;
	gboolean has_ignorepkg = FALSE;
	gboolean has_proxy = FALSE;
	gboolean has_maxtries = FALSE;
	
	fp = fopen (CONF_FILE, "r");
	if (fp == NULL)
		return FALSE;
	tp = tmpfile ();
	if (tp == NULL)
	{
		g_error (_("Could not open temporary file"));
		return FALSE;
	}
	while (fgets(line,PATH_MAX,fp))
	{
		if (line[0] == '#')
			continue;
		if (g_str_has_prefix(line,"LogFile"))
			has_logfile = TRUE;
		else
		if (g_str_has_prefix(line,"CacheDir"))
			has_cachedir = TRUE;
		else
		if (g_str_has_prefix(line,"DBPath"))
			has_dbpath = TRUE;
		else
		if (g_str_has_prefix(line,"HoldPkg"))
			has_holdpkg = TRUE;
		else
		if (g_str_has_prefix(line,"IgnorePkg"))
			has_ignorepkg = TRUE;
		else
		if (g_str_has_prefix(line,"ProxyServer"))
			has_proxy = TRUE;
		else
		if (g_str_has_prefix(line,"MaxTries"))
			has_maxtries = TRUE;
		continue;
	}
	rewind (fp);
	while (fgets(line,PATH_MAX,fp))
	{
		if (line[0]=='#')
			goto down;
		if (g_str_has_prefix(line,"[options]"))
		{
			fprintf (tp, line);
			if (!has_logfile && gfpm_prefs_logfile_path!=NULL)
			{
				fprintf (tp, "LogFile = %s\n", gfpm_prefs_logfile_path);
				continue;
			}
			if (!has_cachedir && gfpm_prefs_cache_dir!=NULL)
			{
				fprintf (tp, "CacheDir = %s\n", gfpm_prefs_cache_dir);
				continue;
			}
			if (!has_dbpath && gfpm_prefs_database_path!=NULL)
			{
				fprintf (tp, "DBPath = %s\n", gfpm_prefs_database_path);
				continue;
			}
			if (!has_holdpkg && gfpm_prefs_holdpkg_list!=NULL)
			{
				GList	*list = NULL;
				list = gfpm_prefs_holdpkg_list;
				fprintf (tp, "HoldPkg = ");
				while (list != NULL)
				{
					fprintf (tp, "%s ", (char*)list->data);
					list = g_list_next (list);
				}
				fprintf (tp, "\n");
				continue;
			}
			if (!has_ignorepkg && gfpm_prefs_ignorepkg_list!=NULL)
			{
				GList	*list = NULL;
				list = gfpm_prefs_ignorepkg_list;
				fprintf (tp, "IgnorePkg = ");
				while (list != NULL)
				{
					fprintf (tp, "%s ", (char*)list->data);
					list = g_list_next (list);
				}
				fprintf (tp, "\n");
				continue;
			}
			if (!has_proxy && gfpm_prefs_proxy_server!=NULL)
			{
				fprintf (tp, "ProxyServer = %s\n", gfpm_prefs_proxy_server);
				fprintf (tp, "\n");
				continue;
			}
			if (!has_maxtries)
			{
				fprintf (tp, "MaxTries = %d\n", gfpm_prefs_max_tries);
				fprintf (tp, "\n");
				continue;
			}
			continue;
		}
		if (g_str_has_prefix(line,"LogFile"))
		{
			if (gfpm_prefs_logfile_path != NULL)
				fprintf (tp, "LogFile = %s\n", gfpm_prefs_logfile_path);
			continue;
		}
		if (g_str_has_prefix(line,"CacheDir"))
		{
			if (gfpm_prefs_cache_dir != NULL)
				fprintf (tp, "CacheDir = %s\n", gfpm_prefs_cache_dir);
			continue;
		}
		if (g_str_has_prefix(line,"DBPath"))
		{
			if (gfpm_prefs_database_path != NULL)
				fprintf (tp, "DBPath = %s\n", gfpm_prefs_database_path);
			continue;
		}
		else
		if (g_str_has_prefix(line,"ProxyServer"))
		{
			if (gfpm_prefs_proxy_server != NULL)
				fprintf (tp, "ProxyServer = %s\n", gfpm_prefs_proxy_server);
			continue;
		}
		else
		if (g_str_has_prefix(line,"HoldPkg"))
		{
			GList	*list = NULL;
			list = gfpm_prefs_holdpkg_list;
			if (list == NULL)
				goto hskip;
			fprintf (tp, "HoldPkg = ");
			while (list != NULL)
			{
				fprintf (tp, "%s ", (char*)list->data);
				list = g_list_next (list);
			}
			hskip:fprintf (tp, "\n");
			continue;
		}
		else
		if (g_str_has_prefix(line,"IgnorePkg"))
		{
			GList	*list = NULL;
			list = gfpm_prefs_ignorepkg_list;
			if (list == NULL)
				goto iskip;
			fprintf (tp, "IgnorePkg = ");
			while (list != NULL)
			{
				fprintf (tp, "%s ", (char*)list->data);
				list = g_list_next (list);
			}
			iskip:fprintf (tp, "\n");
			continue;
		}
		else
		if (g_str_has_prefix(line,"MaxTries"))
		{
			fprintf (tp, "MaxTries = %d\n", gfpm_prefs_max_tries);
			continue;
		}
		down:fprintf (tp,line);
	}
	rewind (tp);
	fclose (fp);
	fp = fopen (CONF_FILE, "w");
	if (fp == NULL)
	{
		g_error (_("Error saving configuration"));
		return FALSE;
	}
	while (fgets(line,PATH_MAX,tp))
		fprintf (fp, line);
	fclose (fp);
	fclose (tp);
	
	return TRUE;
}

static void
gfpm_prefs_populate_ignorepkg (void)
{
	PM_LIST	*list = NULL;
	long	data;
	
	/* free the old list */
	if (gfpm_prefs_ignorepkg_list != NULL)
	{
		g_list_free (gfpm_prefs_ignorepkg_list);
		gfpm_prefs_ignorepkg_list = NULL;
	}
	
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

static gchar*
gfpm_prefs_get_value_for_key (const char *key)
{
	FILE	*fp = NULL;
	char	*ret = NULL;
	char	line[PATH_MAX+1] = "";

	fp = fopen (CONF_FILE, "r");
	if (fp != NULL)
	{
		while (fgets(line,PATH_MAX,fp))
		{
			if (line[0] == '#')
				continue;
			if (g_str_has_prefix(line,key))
			{
				char *lf = NULL;
				lf = strtok (line, "=");
				lf = strtok (NULL, "=");
				ret = g_strdup (g_strstrip(lf));
			}
		}
		fclose (fp);
	}

	return ret;
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

void
gfpm_prefs_cleanup (void)
{
	g_list_free (gfpm_prefs_holdpkg_list);
	g_list_free (gfpm_prefs_ignorepkg_list);
	if (gfpm_prefs_logfile_path)
		g_free (gfpm_prefs_logfile_path);
	if (gfpm_prefs_proxy_server)
		g_free (gfpm_prefs_logfile_path);
	if (gfpm_prefs_cache_dir)
		g_free (gfpm_prefs_cache_dir);
	if (gfpm_prefs_database_path)
		g_free (gfpm_prefs_database_path);

	return;
}

static void
cb_gfpm_prefs_holdpkg_add_btn_clicked (GtkButton *button, gpointer data)
{
	char	*pkg = NULL;
	GList	*list = NULL;
	
	pkg = gfpm_input (_("Hold Package"), _("Enter the name of the package :"));
	list = gfpm_prefs_holdpkg_list;
	if (list == NULL)
	{
		gfpm_prefs_holdpkg_list = g_list_append (gfpm_prefs_holdpkg_list,
							pkg);
	}
	else
	{
		while (list != NULL)
		{
			if (!strcmp((char*)list->data,pkg))
			{
				gfpm_error (_("Duplicate"), _("This package already exists in the list"));
				g_free (pkg);
				return;
			}
			list = g_list_next (list);
		}
		gfpm_prefs_holdpkg_list = g_list_append (gfpm_prefs_holdpkg_list,
							pkg);
	}
	gfpm_prefs_write_config ();
	gfpm_prefs_populate_holdpkg_tvw ();

	return;
}

static void
cb_gfpm_prefs_ignorepkg_add_btn_clicked (GtkButton *button, gpointer data)
{
	char	*pkg = NULL;
	GList	*list = NULL;

	pkg = gfpm_input (_("Ignore Package"), _("Enter the name of the package :"));
	list = gfpm_prefs_ignorepkg_list;
	if (list == NULL)
	{
		gfpm_prefs_ignorepkg_list = g_list_append (gfpm_prefs_ignorepkg_list,
							pkg);
	}
	else
	{
		while (list != NULL)
		{
			if (!strcmp((char*)list->data,pkg))
			{
				gfpm_error (_("Duplicate"), _("This package already exists in the list"));
				g_free (pkg);
				return;
			}
			list = g_list_next (list);
		}
		gfpm_prefs_ignorepkg_list = g_list_append (gfpm_prefs_ignorepkg_list,
							pkg);
	}
	gfpm_prefs_write_config ();
	gfpm_prefs_populate_ignorepkg_tvw ();

	return;
}

static void
cb_gfpm_prefs_holdpkg_remove_btn_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter		iter;
	gchar			*pkg = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_prefs_holdpkg_tvw));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 0, &pkg, -1);
		if (pkg!=NULL)
		{
			GList	*list = NULL;
			list = gfpm_prefs_holdpkg_list;
			while (list != NULL)
			{
				/* delete the item from the list if found */
				if (!strcmp((char*)list->data,pkg))
				{
					gfpm_prefs_holdpkg_list = g_list_delete_link (gfpm_prefs_holdpkg_list,
											list);
					gfpm_prefs_write_config ();
					gfpm_prefs_populate_holdpkg_tvw ();
					break;
				}
				list = g_list_next (list);
			}
		}
	}
	
	return;
}

static void
cb_gfpm_prefs_ignorepkg_remove_btn_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter		iter;
	gchar			*pkg = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_prefs_ignorepkg_tvw));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 0, &pkg, -1);
		if (pkg!=NULL)
		{
			GList	*list = NULL;
			list = gfpm_prefs_ignorepkg_list;
			while (list != NULL)
			{
				/* delete the item from the list if found */
				if (!strcmp((char*)list->data,pkg))
				{
					gfpm_prefs_ignorepkg_list = g_list_delete_link (gfpm_prefs_ignorepkg_list,
											list);
					gfpm_prefs_write_config ();
					gfpm_prefs_populate_ignorepkg_tvw ();
					break;
				}
				list = g_list_next (list);
			}
		}
	}
	
	return;
}

static void
cb_gfpm_prefs_compressed_size_toggled (GtkToggleButton *button, gpointer data)
{
	gboolean	check;

	check = gtk_toggle_button_get_active (button);

	/* write settings to config file */
	gfpm_config_set_value_bool ("show_compressed_size", check);
	gfpm_config_save ();
	gfpm_pkgs_show_compressed_size (check);

	return;
}

static void
cb_gfpm_prefs_uncompressed_size_toggled (GtkToggleButton *button, gpointer data)
{
	gboolean	check;

	check = gtk_toggle_button_get_active (button);

	/* write settings to config file */
	gfpm_config_set_value_bool ("show_uncompressed_size", check);
	gfpm_config_save ();
	gfpm_pkgs_show_uncompressed_size (check);

	return;
}

static void
cb_gfpm_prefs_logging_enable_toggled (GtkToggleButton *button, gpointer data)
{
	gboolean	check;

	check = gtk_toggle_button_get_active (button);

	/* write settings to config file */
	if (check==FALSE)
	{
		gtk_widget_set_sensitive (gfpm_prefs_log_location, FALSE);
		if (gfpm_prefs_logfile_path!=NULL)
		{
			g_free (gfpm_prefs_logfile_path);
			gfpm_prefs_logfile_path = NULL;
		}
	}
	else
	{
		gtk_widget_set_sensitive (gfpm_prefs_log_location, TRUE);
		gfpm_prefs_logfile_path = g_strdup (LOG_FILE);
		gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_log_location), gfpm_prefs_logfile_path);
	}
	gfpm_prefs_write_config ();

	return;
}

static void
cb_gfpm_prefs_proxy_enable_toggled (GtkToggleButton *button, gpointer data)
{
	gboolean	check;

	check = gtk_toggle_button_get_active (button);

	if (check)
	{
		char	*proxy = NULL;
		
		/* get url of the proxy server from user */
		proxy = gfpm_input (_("Enter proxy server url"), _("Enter the URL of the proxy server :"));
		if (proxy)
		{
			gtk_widget_set_sensitive (gfpm_prefs_proxy_server_entry, TRUE);
			gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_proxy_server_entry), proxy);
			/* say goodbye to the old proxy server, if any */
			if (gfpm_prefs_proxy_server)
			{
				g_free (gfpm_prefs_proxy_server);
			}
			/* set the new proxy server */
			gfpm_prefs_proxy_server = proxy;
		}
	}
	else
	{
		if (gfpm_prefs_proxy_server)
		{
			gtk_widget_set_sensitive (gfpm_prefs_proxy_server_entry, FALSE);
			gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_proxy_server_entry), "");
			g_free (gfpm_prefs_proxy_server);
			gfpm_prefs_proxy_server = NULL;
		}
	}
	/* write settings to config file */
	gfpm_prefs_write_config ();

	return;
}

static void
cb_gfpm_prefs_maxtries_value_changed (GtkSpinButton *button, gpointer data)
{
	gfpm_prefs_max_tries = gtk_spin_button_get_value (button);
	gfpm_prefs_write_config ();
	
	return;
}

static void
cb_gfpm_prefs_edit_logpath_btn_clicked (GtkButton *button, gpointer data)
{
	GtkFileChooserDialog *dlg;
	
	dlg = gtk_file_chooser_dialog_new (_("Select log file"),
									   NULL,
									   GTK_FILE_CHOOSER_ACTION_OPEN,
									   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									   NULL);
	if (gtk_dialog_run (GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dlg));
		if (filename)
		{
			if (gfpm_prefs_logfile_path)
				g_free (gfpm_prefs_logfile_path);
			gfpm_prefs_logfile_path = filename;
			if (gfpm_prefs_write_config())
			{
				gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_log_location), gfpm_prefs_logfile_path);
			}
		}
	}
	gtk_widget_destroy (dlg);
	
	return;
}

static void
cb_gfpm_prefs_edit_cachedir_btn_clicked (GtkButton *button, gpointer data)
{
	GtkFileChooserDialog *dlg;
	
	dlg = gtk_file_chooser_dialog_new (_("Select directory"),
									   NULL,
									   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
									   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									   NULL);
	if (gtk_dialog_run (GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dlg));
		if (filename)
		{
			if (gfpm_prefs_cache_dir)
				g_free (gfpm_prefs_cache_dir);
			gfpm_prefs_cache_dir = filename;
			if (gfpm_prefs_write_config())
			{
				gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_cache_dir_entry), gfpm_prefs_cache_dir);
			}
		}
	}
	gtk_widget_destroy (dlg);
	
	return;
}

static void
cb_gfpm_prefs_edit_database_btn_clicked (GtkButton *button, gpointer data)
{
	GtkFileChooserDialog *dlg;
	
	dlg = gtk_file_chooser_dialog_new (_("Select directory"),
									   NULL,
									   GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
									   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
									   GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
									   NULL);
	if (gtk_dialog_run (GTK_DIALOG(dlg)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dlg));
		if (filename)
		{
			if (gfpm_prefs_database_path)
				g_free (gfpm_prefs_database_path);
			gfpm_prefs_database_path = filename;
			if (gfpm_prefs_write_config())
			{
				gtk_entry_set_text (GTK_ENTRY(gfpm_prefs_database_path_entry), gfpm_prefs_database_path);
			}
		}
	}
	gtk_widget_destroy (dlg);
	
	return;
}

