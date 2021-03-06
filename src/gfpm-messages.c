/*
 *  gfpm-messages.c for gfpm
 *
 *  This code is borrowed from gnetconfig.
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
#include "gfpm.h"
#include "gfpm-messages.h"
#include "gfpm-packagelist.h"

extern GtkWidget	*gfpm_mw;
extern GtkBuilder	*gb;
extern PM_DB		*sync_db;
extern PM_DB		*local_db;
extern GfpmList		*install_list;
extern GfpmList		*remove_list;

static GtkWidget *gfpm_apply_dlg;
static GtkWidget *gfpm_apply_inst_tvw;
static GtkWidget *gfpm_apply_inst_box;
static GtkWidget *gfpm_apply_rem_tvw;
static GtkWidget *gfpm_apply_rem_box;
static GtkWidget *gfpm_apply_inst_sizelbl;
static GtkWidget *gfpm_apply_rem_sizelbl;

static void cb_gfpm_plist_question_upgrade_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data);

void
gfpm_messages_init (void)
{
	GtkCellRenderer *ren = NULL;
	GtkListStore	*store = NULL;
	GtkWidget	*button = NULL;

	/* lookup necessary widgets */
	gfpm_apply_dlg = gfpm_get_widget ("apply_dlg");
	gfpm_apply_inst_tvw = gfpm_get_widget ("insttvw");
	gfpm_apply_inst_box = gfpm_get_widget ("instbox");
	gfpm_apply_rem_tvw = gfpm_get_widget ("remtvw");
	gfpm_apply_rem_box = gfpm_get_widget ("rembox");
	gfpm_apply_inst_sizelbl = gfpm_get_widget ("instsizelbl");
	gfpm_apply_rem_sizelbl = gfpm_get_widget ("remsizelbl");

	/* setup apply dialog */
	button = gtk_button_new_from_stock (GTK_STOCK_OK);
	gtk_dialog_add_action_widget (GTK_DIALOG(gfpm_apply_dlg), button, GTK_RESPONSE_OK);
	gtk_widget_show (button);
	button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
	gtk_dialog_add_action_widget (GTK_DIALOG(gfpm_apply_dlg), button, GTK_RESPONSE_CANCEL);
	gtk_widget_show (button);

	/* setup treeviews */
	ren = gtk_cell_renderer_text_new ();
	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_apply_inst_tvw), -1, _("Package"), ren, "text", 0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_apply_inst_tvw), -1, _("Size"), ren, "text", 1, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_apply_inst_tvw), GTK_TREE_MODEL(store));
	ren = gtk_cell_renderer_text_new ();
	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_apply_rem_tvw), -1, _("Package"), ren, "text", 0, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(gfpm_apply_rem_tvw), -1, _("Size"), ren, "text", 1, NULL);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_apply_rem_tvw), GTK_TREE_MODEL(store));

}

static void
gfpm_apply_dlg_populate (void)
{
	gboolean inst = FALSE;
	gboolean rem = FALSE;

	if (gfpm_package_list_is_empty(GFPM_INSTALL_LIST))
	{
		gfpm_apply_dlg_show_inst_box (TRUE);
		inst = TRUE;
	}
	else
		gfpm_apply_dlg_show_inst_box (FALSE);
	if (gfpm_package_list_is_empty(GFPM_REMOVE_LIST))
	{
		gfpm_apply_dlg_show_rem_box (TRUE);
		rem = TRUE;
	}
	else
		gfpm_apply_dlg_show_rem_box (FALSE);

	/* populate package lists */
	if (inst)
	{
		GtkListStore	*store;
		GList		*i;
		GtkTreeIter	iter;
		float		totalisize = 0;
		gchar		*totaltext = NULL;

		store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gfpm_apply_inst_tvw)));
		for (i=(GList*)install_list;i;i=g_list_next(i))
		{
			PM_PKG	*pkg = NULL;
			char	*size = NULL;
			float	s = 0;
			pkg = pacman_db_readpkg (sync_db, (char*)i->data);
			if (pkg)
			{
				s = (float)((long)pacman_pkg_getinfo (pkg, PM_PKG_SIZE)/1024)/1024;
				totalisize += s;
			}
			asprintf (&size, "%0.2f MB", s);
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter, 0, (char*)i->data, 1, size, -1);
			pacman_pkg_free (pkg);
			if (size) g_free (size);
		}
		totaltext = g_strdup_printf (_("Total package size: %0.2f MB"), totalisize);
		gtk_label_set_text (GTK_LABEL(gfpm_apply_inst_sizelbl), totaltext);
		g_free (totaltext);
	}
	if (rem)
	{
		GtkListStore	*store;
		GList		*i;
		GtkTreeIter	iter;
		float		totalrsize = 0;
		gchar		*totaltext = NULL;

		store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gfpm_apply_rem_tvw)));
		for (i=(GList*)remove_list;i;i=g_list_next(i))
		{
			PM_PKG	*pkg = NULL;
			char	*size = NULL;
			float	s = 0;
			pkg = pacman_db_readpkg (local_db, (char*)i->data);
			if (pkg)
			{
				s = (float)((long)pacman_pkg_getinfo (pkg, PM_PKG_SIZE)/1024)/1024;
				totalrsize += s;
				asprintf (&size, "%0.2f MB", s);
				pacman_pkg_free (pkg);
			}
			
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter, 0, (char*)i->data, 1, size, -1);
			if (size)
				g_free (size);
		}
		totaltext = g_strdup_printf (_("Total package size: %0.2f MB"), totalrsize);
		gtk_label_set_text (GTK_LABEL(gfpm_apply_rem_sizelbl), totaltext);
		g_free (totaltext);
	}

	return;
}

void
gfpm_apply_dlg_reset (void)
{
	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gfpm_apply_inst_tvw))));
	gtk_list_store_clear (GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gfpm_apply_rem_tvw))));
	gfpm_apply_dlg_show_inst_box (FALSE);
	gfpm_apply_dlg_show_rem_box (FALSE);
	gtk_widget_hide (gfpm_apply_dlg);

	return;
}

gint
gfpm_apply_dlg_show (void)
{
	gint res;

	gfpm_apply_dlg_populate ();
	res = gtk_dialog_run (GTK_DIALOG(gfpm_apply_dlg));

	return res;
}

void
gfpm_apply_dlg_hide (void)
{
	gtk_widget_hide (gfpm_apply_dlg);

	return;
}

void
gfpm_apply_dlg_show_inst_box (gboolean show)
{
	if (show)
		gtk_widget_show (gfpm_apply_inst_box);
	else
		gtk_widget_hide (gfpm_apply_inst_box);

	return;
}

void
gfpm_apply_dlg_show_rem_box (gboolean show)
{
	if (show)
		gtk_widget_show (gfpm_apply_rem_box);
	else
		gtk_widget_hide (gfpm_apply_rem_box);

	return;
}


void
gfpm_error (const char *message_title, const char *error_str)
{
	GtkWidget *error_dlg = NULL;

	if (!strlen(error_str))
		return;

	error_dlg = gtk_message_dialog_new (GTK_WINDOW(gfpm_mw),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s",
					error_str);
	gtk_window_set_resizable (GTK_WINDOW(error_dlg), FALSE);
	gtk_window_set_title (GTK_WINDOW(error_dlg), message_title);
	gtk_dialog_run (GTK_DIALOG(error_dlg));
	gtk_widget_destroy (error_dlg);

	return;
}

void
gfpm_message (const char *message_title, const char *message_str)
{
	GtkWidget *message_dlg;

	message_dlg = gtk_message_dialog_new (GTK_WINDOW(gfpm_mw),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					"%s",
					message_str);
	gtk_window_set_resizable (GTK_WINDOW(message_dlg), FALSE);
	gtk_window_set_title (GTK_WINDOW(message_dlg), message_title);
	gtk_dialog_run (GTK_DIALOG(message_dlg));
	gtk_widget_destroy (message_dlg);

	return;
}

gint
gfpm_question (const char *message_title, const char *message_str)
{
	GtkWidget 	*dialog;
	gint 		ret;

	dialog = gtk_message_dialog_new (GTK_WINDOW(gfpm_mw),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					"%s",
					message_str);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	gtk_window_set_title (GTK_WINDOW(dialog), message_title);
	ret = gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);

	return ret;
}

gint
gfpm_plist_question (const char *message_title, const char *main_msg, GList *packages)
{
	GtkWidget		*dialog;
	GtkListStore		*store;
	GtkScrolledWindow	*swindow;
	GtkCellRenderer		*r;
	GtkTreeIter		iter;
	GtkWidget		*tvw;
	gint			ret;
	GList			*l;
	GtkTreeViewColumn	*column;

	dialog = gtk_message_dialog_new (GTK_WINDOW(gfpm_mw),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					"%s",
					main_msg);
	swindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	gtk_scrolled_window_set_policy (swindow, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (swindow, GTK_SHADOW_ETCHED_OUT);
	tvw = gtk_tree_view_new ();
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(tvw), TRUE);
	gtk_container_add (GTK_CONTAINER(swindow), tvw);
	store = gtk_list_store_new (4, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	r = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Upgrade"), r, "active", 0, NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(tvw), column);
	g_signal_connect (r, "toggled", G_CALLBACK(cb_gfpm_plist_question_upgrade_toggled), store);

	r = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Package"), r, "text", 1, NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(tvw), column);

	r = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Version"), r, "text", 2, NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_min_width (column, 70);
	gtk_tree_view_append_column (GTK_TREE_VIEW(tvw), column);

	r = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Size"), r, "text", 3, NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(tvw), column);

	for (l=g_list_first(packages);l;l=g_list_next(l))
	{
		char *pkgname = NULL;
		char *pkgver = NULL;
		char *size = NULL;
		float s = 0;
		PM_SYNCPKG *sync = l->data;
		PM_PKG *pkg = pacman_sync_getinfo (sync, PM_SYNC_PKG);
		pkgname = pacman_pkg_getinfo (pkg, PM_PKG_NAME);
		pkgver = pacman_pkg_getinfo (pkg, PM_PKG_VERSION);
		s = (float)((long)pacman_pkg_getinfo (pkg, PM_PKG_SIZE)/1024)/1024;
		asprintf (&size, "%0.2f MB", s);
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, TRUE, 1, pkgname, 2, pkgver, 3, size, -1);
		g_free (size);
	}
	gtk_tree_view_set_model (GTK_TREE_VIEW(tvw), GTK_TREE_MODEL(store));
	gtk_widget_set_size_request (tvw, 230, 120);
	gtk_widget_show (tvw);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox), GTK_WIDGET(swindow), FALSE, FALSE, 0);
	gtk_widget_show_all (GTK_DIALOG(dialog)->vbox);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	gtk_window_set_title (GTK_WINDOW(dialog), message_title);
	ret = gtk_dialog_run (GTK_DIALOG(dialog));

	if (ret == GTK_RESPONSE_YES)
	{
		gfpm_package_list_free (GFPM_INSTALL_LIST);
		gfpm_package_list_free (GFPM_REMOVE_LIST);
		GtkTreeIter	i;
		GtkTreeModel	*model;
		gboolean	v;
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(tvw));
		v = gtk_tree_model_get_iter_first (model, &i);
		while (v!=FALSE)
		{
			gchar *pkgname = NULL;
			gboolean sel;
			gtk_tree_model_get (model, &i, 0, &sel, 1, &pkgname, -1);
			if ((sel==TRUE) && (pkgname!=NULL))
				gfpm_package_list_add (GFPM_INSTALL_LIST, pkgname);
			g_free (pkgname);
			v = gtk_tree_model_iter_next (model, &i);
		}
	}
	gtk_widget_destroy (dialog);

	return ret;
}

static void
cb_gfpm_plist_question_upgrade_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data)
{
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	GtkTreePath	*path;
	gboolean	check;

	model = (GtkTreeModel *)data;
	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, 0, &check, -1);
	/* manually toggle the toggle button */
	check ^= 1;
	gtk_list_store_set (GTK_LIST_STORE(model), &iter, 0, check, -1);
}

void
gfpm_plist_message (const char *message_title, const char *main_msg, GtkMessageType type, GList *packages)
{
	GtkWidget		*dialog;
	GtkListStore		*store;
	GtkScrolledWindow	*swindow;
	GtkCellRenderer		*r;
	GtkTreeIter		iter;
	GtkWidget		*tvw;
	GList			*l;

	if (packages == NULL)
		return;
	dialog = gtk_message_dialog_new (GTK_WINDOW(gfpm_mw),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					type,
					GTK_BUTTONS_CLOSE,
					"%s",
					main_msg);
	swindow = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new (NULL, NULL));
	gtk_scrolled_window_set_policy (swindow, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (swindow, GTK_SHADOW_ETCHED_IN);
	tvw = gtk_tree_view_new ();
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(tvw), FALSE);
	gtk_container_add (GTK_CONTAINER(swindow), tvw);
	store = gtk_list_store_new (1, G_TYPE_STRING);
	r = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(tvw), -1, _("Package"), r, "text", 0, NULL);
	for (l=g_list_first(packages);l;l=g_list_next(l))
	{
		char *pkgstring = (char*)l->data;
		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 0, pkgstring, -1);
		g_free (pkgstring);
	}
	gtk_tree_view_set_model (GTK_TREE_VIEW(tvw), GTK_TREE_MODEL(store));
	gtk_widget_set_size_request (tvw, 330, 120);
	gtk_widget_show (tvw);
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dialog)->vbox), GTK_WIDGET(swindow), FALSE, FALSE, 0);
	gtk_widget_show_all (GTK_DIALOG(dialog)->vbox);
	gtk_window_set_resizable (GTK_WINDOW(dialog), TRUE);
	gtk_window_set_title (GTK_WINDOW(dialog), message_title);
	gtk_dialog_run (GTK_DIALOG(dialog));

	gtk_widget_destroy (dialog);

	return;
}

char *
gfpm_input (const char *title, const char *message)
{
	GtkWidget	*dialog;
	GtkWidget	*entry;
	GtkWidget	*label;
	char		*ret = NULL;
	gint		response;

	ret = NULL;

	dialog = gtk_dialog_new_with_buttons (title,
						GTK_WINDOW(gfpm_mw),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_STOCK_OK,
                        GTK_RESPONSE_ACCEPT,
                        GTK_STOCK_CANCEL,
                        GTK_RESPONSE_REJECT,
                        NULL);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	label = gtk_label_new (message);
	entry = gtk_entry_new ();

	gtk_misc_set_padding (GTK_MISC(label), 5, 5);
	gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER((GTK_DIALOG(dialog))->vbox), 10);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), entry);
	gtk_widget_show_all (GTK_WIDGET(dialog));
	response = gtk_dialog_run (GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_ACCEPT)
		ret = (char*)g_strdup(gtk_entry_get_text (GTK_ENTRY(entry)));
	gtk_widget_destroy (GTK_WIDGET(dialog));

	return ret;
}

void
cb_gfpm_trans_conv (unsigned char event, void *data1, void *data2, void *data3, int *response)
{
	switch (event)
	{
		char *str = NULL;

		case PM_TRANS_CONV_REPLACE_PKG:
			str = g_strdup_printf (_("Do you want to replace %s with %s/%s ?"),
						(char*)pacman_pkg_getinfo (data1, PM_PKG_NAME),
						(char*)data3,
						(char*)pacman_pkg_getinfo (data2, PM_PKG_NAME));
			if (gfpm_question(_("Replace package"), str) == GTK_RESPONSE_YES)
				*response = 1;
			else
				*response = 0;
			break;
		case PM_TRANS_CONV_INSTALL_IGNOREPKG:
			str = g_strdup_printf (_("%s requires %s, but it is IgnorePkg. Install anyway?"),
									(char*)pacman_pkg_getinfo (data1, PM_PKG_NAME),
									(char*)pacman_pkg_getinfo (data2, PM_PKG_NAME));
			if (gfpm_question(_("Install anyway?"), str) == GTK_RESPONSE_YES)
				*response = 1;
			else
				*response = 0;
			break;
		case PM_TRANS_CONV_REMOVE_HOLDPKG:
			str = g_strdup_printf (_("%s is designated as HoldPkg. Remove anyway?"),
									(char*)pacman_pkg_getinfo (data1, PM_PKG_NAME));
			if (gfpm_question(_("Remove anyway?"), str) == GTK_RESPONSE_YES)
				*response = 1;
			else
				*response = 0;
			break;
		case PM_TRANS_CONV_CONFLICT_PKG:
			str = g_strdup_printf (_("%s conflicts with %s. Remove %s?"),
									(char*)data1,
									(char*)data2,
									(char*)data2);
			if (gfpm_question(_("Conflict"), str) == GTK_RESPONSE_YES)
				*response = 1;
			else
				*response = 0;
			break;
		case PM_TRANS_CONV_LOCAL_NEWER:
			str = g_strdup_printf (_("%s-%s: local version is newer. Upgrade anyway?"),
									(char*)pacman_pkg_getinfo (data1, PM_PKG_NAME),
									(char*)pacman_pkg_getinfo (data1, PM_PKG_VERSION));
			if (gfpm_question(_("Local version newer"), str) == GTK_RESPONSE_YES)
				*response = 1;
			else
				*response = 0;
			break;
		case PM_TRANS_CONV_LOCAL_UPTODATE:
			str = g_strdup_printf (_("%s-%s: local version is up to date. Upgrade anyway?"),
									(char*)pacman_pkg_getinfo (data1, PM_PKG_NAME),
									(char*)pacman_pkg_getinfo (data1, PM_PKG_VERSION));
			if (gfpm_question(_("Local version up to date"), str) == GTK_RESPONSE_YES)
				*response = 1;
			else
				*response = 0;
			break;
		case PM_TRANS_CONV_CORRUPTED_PKG:
			str = g_strdup_printf (_("Archive %s is corrupted. Do you want to delete it?"),
									(char*)pacman_pkg_getinfo (data1, PM_PKG_NAME));
			if (gfpm_question(_("Package corrupted"), str) == GTK_RESPONSE_YES)
				*response = 1;
			else
				*response = 0;
			break;
	}
	return;
}

