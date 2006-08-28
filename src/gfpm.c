/***************************************************************************
 *  gfpm.c
 *
 *  Sat Aug 26 22:36:56 2006
 *  Copyright	2006  Frugalware Developer Team
 *  Authors		Christian Hamar (krix) & Miklos Nemeth (desco)
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

#include <stdio.h>

#include <gtk/gtk.h>
#include <glade/glade.h>

#include "gfpm.h"

GtkWidget *group_treeview;


int main(int argc, char *argv[])
{
    GladeXML *xml;

    gtk_init(&argc, &argv);

    xml = glade_xml_new("glade/gfpm.glade", NULL, NULL);

    glade_xml_signal_autoconnect(xml);
    
    group_treeview = glade_xml_get_widget(xml,"grouptreeview");
    
    gfpm_init_treeview();

    gtk_main();

    return 0;
}

void gfpm_init_treeview(void)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (group_treeview),
                                               -1,      
                                               "Groups",  
                                               renderer,
                                               "text", 0,
                                               NULL);
	store = gtk_list_store_new (1, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(group_treeview),GTK_TREE_MODEL(store));



	return;
}
