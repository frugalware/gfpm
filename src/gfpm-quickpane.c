/*
 *  gfpm-quickpane.c for gfpm
 *
 *  Copyright (C) 2006-2008 by Priyank Gosalia <priyankmg@gmail.com>
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

#include "gfpm.h"
#include "gfpm-quickpane.h"
#include "gfpm-packagelist.h"
#include "gfpm-interface.h"

gchar *quickpane_pkg = NULL;

extern GladeXML *xml;
extern GfpmList *install_list;
extern GfpmList *remove_list;
extern GtkWidget *gfpm_pkgs_tvw;
extern GtkWidget *gfpm_mw;

static GtkWidget *quick_pane;
static GtkWidget *quick_pane_install_btn;
static GtkWidget *quick_pane_remove_btn;
static GtkWidget *quick_pane_upgrade_btn;
static GtkWidget *quick_pane_readme_btn;
static GtkWidget *quick_pane_readme_dlg;
static GtkWidget *quick_pane_readme_dlg_txtvw;
static GtkWidget *quick_pane_readme_dlg_label;

static void cb_gfpm_quickpane_install_clicked (GtkWidget *button, gpointer data);
static void cb_gfpm_quickpane_remove_clicked (GtkWidget *button, gpointer data);
static void cb_gfpm_quickpane_upgrade_clicked (GtkWidget *button, gpointer data);
static void cb_gfpm_quickpane_readme_clicked (GtkWidget *button, gpointer data);

void
gfpm_quickpane_init (void)
{
	quick_pane_install_btn = glade_xml_get_widget (xml, "quick_install");
	quick_pane_remove_btn = glade_xml_get_widget (xml, "quick_remove");
	quick_pane_upgrade_btn = glade_xml_get_widget (xml, "quick_upgrade");
	quick_pane_readme_btn = glade_xml_get_widget (xml, "quick_readme");
	quick_pane_readme_dlg = glade_xml_get_widget (xml, "readme_dlg");
	quick_pane_readme_dlg_label = glade_xml_get_widget (xml, "readme_dlg_label");
	quick_pane_readme_dlg_txtvw = glade_xml_get_widget (xml, "readme_dlg_txtvw");
	quick_pane = glade_xml_get_widget (xml, "quick_pane");
	gfpm_quickpane_show (FALSE, 0, 0);
	g_signal_connect (G_OBJECT(quick_pane_install_btn),
					"clicked",
					G_CALLBACK(cb_gfpm_quickpane_install_clicked),
					NULL);
	g_signal_connect (G_OBJECT(quick_pane_remove_btn),
					"clicked",
					G_CALLBACK(cb_gfpm_quickpane_remove_clicked),
					NULL);
	g_signal_connect (G_OBJECT(quick_pane_upgrade_btn),
					"clicked",
					G_CALLBACK(cb_gfpm_quickpane_upgrade_clicked),
					NULL);
	g_signal_connect (G_OBJECT(quick_pane_readme_btn),
					"clicked",
					G_CALLBACK(cb_gfpm_quickpane_readme_clicked),
					NULL);

	return;
}

void
gfpm_quickpane_readme_btn_show (void)
{
	gtk_widget_show (quick_pane_readme_btn);

	return;
}

void
gfpm_quickpane_show (gboolean show, gboolean what, gboolean upgrade)
{
	if (show)
	{
		gtk_widget_show (quick_pane);
		if (what)
		{
			gtk_widget_show (quick_pane_remove_btn);
			gtk_widget_hide (quick_pane_install_btn);
			if (upgrade)
				gtk_widget_show (quick_pane_upgrade_btn);
			else
				gtk_widget_hide (quick_pane_upgrade_btn);
		}
		else
		{
			gtk_widget_hide (quick_pane_remove_btn);
			gtk_widget_hide (quick_pane_upgrade_btn);
			gtk_widget_show (quick_pane_install_btn);
		}
		gtk_widget_hide (quick_pane_readme_btn);
	}
	else
	{
		gtk_widget_hide (quick_pane);
	}

	return;
}

void
gfpm_quickpane_readme_dlg_populate (const char *pathname)
{
	GtkTextBuffer	*buffer;
	GtkTextIter		iter;
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(quick_pane_readme_dlg_txtvw));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
	gtk_label_set_text (GTK_LABEL(quick_pane_readme_dlg_label), pathname);
	if (g_file_test (pathname, G_FILE_TEST_EXISTS))
	{
		FILE *fp = NULL;
		gchar line[PATH_MAX+1];
		if ((fp = fopen(pathname, "r")) == NULL)
		{
			gtk_text_buffer_insert (buffer, &iter, _("No Readme available for this package"), -1);
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

void
gfpm_quickpane_readme_dlg_show (void)
{
	gtk_window_set_transient_for (GTK_WINDOW(quick_pane_readme_dlg), GTK_WINDOW(gfpm_mw));
	gtk_widget_show (quick_pane_readme_dlg);
	
	return;
}

/* CALLBACKS */

static void
cb_gfpm_quickpane_install_clicked (GtkWidget *button, gpointer data)
{
	GfpmList *temp = NULL;
	
	temp = install_list;
	install_list = NULL;
	gfpm_package_list_add (GFPM_INSTALL_LIST, quickpane_pkg);
	cb_gfpm_apply_btn_clicked (NULL, NULL);
	install_list = temp;

	return;
}

static void
cb_gfpm_quickpane_remove_clicked (GtkWidget *button, gpointer data)
{
	GfpmList *temp = NULL;
	
	temp = remove_list;
	remove_list = NULL;
	gfpm_package_list_add (GFPM_REMOVE_LIST, quickpane_pkg);
	cb_gfpm_apply_btn_clicked (NULL, NULL);
	remove_list = temp;
	
	return;
}

static void
cb_gfpm_quickpane_upgrade_clicked (GtkWidget *button, gpointer data)
{
	cb_gfpm_quickpane_install_clicked (NULL, NULL);

	return;
}

static void
cb_gfpm_quickpane_readme_clicked (GtkWidget *button, gpointer data)
{
	gfpm_quickpane_readme_dlg_show ();

	return;
}

