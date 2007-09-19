/*
 *  gfpm-quickpane.c for gfpm
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

#include "gfpm-quickpane.h"
#include "gfpm-packagelist.h"
#include "gfpm-interface.h"
#include <glade/glade.h>

gchar *quickpane_pkg = NULL;

extern GladeXML *xml;
extern GfpmList *install_list;
extern GfpmList *remove_list;
extern GtkWidget *gfpm_pkgs_tvw;

static GtkWidget *quick_pane;
static GtkWidget *quick_pane_install_btn;
static GtkWidget *quick_pane_remove_btn;
static GtkWidget *quick_pane_upgrade_btn;

static void cb_gfpm_quickpane_install_clicked (GtkWidget *button, gpointer data);
static void cb_gfpm_quickpane_remove_clicked (GtkWidget *button, gpointer data);
static void cb_gfpm_quickpane_upgrade_clicked (GtkWidget *button, gpointer data);

void
gfpm_quickpane_init (void)
{
	quick_pane_install_btn = glade_xml_get_widget (xml, "quick_install");
	quick_pane_remove_btn = glade_xml_get_widget (xml, "quick_remove");
	quick_pane_upgrade_btn = glade_xml_get_widget (xml, "quick_upgrade");
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
	}
	else
	{
		gtk_widget_hide (quick_pane);
	}

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

