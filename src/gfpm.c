/***************************************************************************
 *  gfpm.c
 *
 *  Sat Aug 26 22:36:56 2006
 *  Copyright 2006  Frugalware Developer Team
 *  Authors  Christian Hamar (krix) & Miklos Nemeth (desco)
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
#include <glade/glade.h>

#include "gfpm.h"
#include "widgets.h"

int main(int argc, char *argv[])
{
	GladeXML *xml;
	GtkWidget *mainwindow;

	alpm_initialize("/");
	local = alpm_db_register(REPO);

	gtk_init(&argc, &argv);

	xml = glade_xml_new("glade/gfpm.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);

	mainwindow = glade_xml_get_widget(xml, "mainwindow");
	group_treeview = glade_xml_get_widget(xml,"grouptreeview");
	pkgs_treeview = glade_xml_get_widget(xml,"pkgstreeview");
	info_treeview = glade_xml_get_widget(xml,"infotreeview");
	combobox_repos = glade_xml_get_widget(xml,"combobox_repos");
	statusbar = glade_xml_get_widget(xml, "statusbar");
	filesview = glade_xml_get_widget(xml, "textview1");

	gfpm_create_group_treeview();
	gfpm_create_pkgs_treeview();
	gfpm_create_info_treeview();
	gfpm_create_combobox_repos();

	asprintf(&repository, "%s", REPO);
	load_groups_treeview(REPO);

	gtk_main();

	alpm_release();
	return(0);
}

/* Do 'clean' exit when clicked on exit button */
void exit_cleanup()
{
	alpm_release();
	gtk_main_quit();
	exit(0);
}
