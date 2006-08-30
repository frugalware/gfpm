/***************************************************************************
 *  widgets.c
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
#include "widgets.h"
#include "gfpm.h"

/* Create treeview widget for *groups* */
void gfpm_create_group_treeview(void) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;
    GtkTreeSelection *selection;

    store = gtk_list_store_new(1, G_TYPE_STRING);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(group_treeview),
	-1,
	"Groups", renderer, "text", 0,
	NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(group_treeview), GTK_TREE_MODEL(store));

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(group_treeview));
    g_signal_connect(selection, "changed", G_CALLBACK(_group_treeview_select), NULL);
    
    return;
}

/* Create treeview widget for *packages* */
void gfpm_create_pkgs_treeview(void) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;
    GtkTreeSelection *selection;

    store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pkgs_treeview),
	-1,
	"Package Name", renderer, "text", 0,
	NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pkgs_treeview),
	-1,
	"Version", renderer, "text", 1,
	NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pkgs_treeview),
	-1,
	"Description", renderer, "text", 2,
	NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(pkgs_treeview), GTK_TREE_MODEL(store));

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pkgs_treeview));
    g_signal_connect(selection, "changed", G_CALLBACK(_pkgs_treeview_select), NULL);
    
    return;
}

/* Callback function for group_treeview (catch clicks in treeview) */
int _group_treeview_select() {
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeSelection *s;
    char *groupname;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(group_treeview));
    s = gtk_tree_view_get_selection(GTK_TREE_VIEW(group_treeview));

    if (gtk_tree_selection_get_selected(s, &model, &iter)) {
	gtk_tree_model_get(model, &iter, 0, &groupname, -1);
	_update_pkgs_treeview(groupname);
	return(0);
    } else {
	return(1);
    }
}

/* Callback function for pkgs_treeview (catch clicks in treeview) */
int _pkgs_treeview_select() {
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeSelection *s;
    char *pkgname;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(pkgs_treeview));
    s = gtk_tree_view_get_selection(GTK_TREE_VIEW(pkgs_treeview));

    if (gtk_tree_selection_get_selected(s, &model, &iter)) {
	gtk_tree_model_get(model, &iter, 0, &pkgname, -1);
	_load_files_textview(pkgname);
	_load_info_textview(pkgname);
	return(0);
    } else {
	return(1);
    }
}

/* Updates the pkgs_treeview when clicked on something in group_treeview */
void _update_pkgs_treeview(char *gn) {
    GtkTreeIter iter;
    GtkTreeModel *model;

    PM_LIST *pkgnames, *i;
    PM_GRP *grp = alpm_db_readgrp(local, gn);
    PM_PKG *pkg;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(pkgs_treeview));
    gtk_list_store_clear(GTK_LIST_STORE(model));

    pkgnames = alpm_grp_getinfo(grp, PM_GRP_PKGNAMES);

    for (i = pkgnames; i; i = alpm_list_next(i)) {
	pkg = alpm_db_readpkg(local, alpm_list_getdata(i));

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
	    0, (char *)alpm_list_getdata(i),
	    1, (char *)alpm_pkg_getinfo(pkg, PM_PKG_VERSION),
	    2, (char *)alpm_pkg_getinfo(pkg, PM_PKG_DESC),
	-1);
    }
}

void _load_groups_treeview(char *repo) {
    GtkTreeModel *model;
    GtkTreeIter iter;
    PM_LIST *i;
    guint ci;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(group_treeview));

    ci = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "-");
    gtk_statusbar_push(GTK_STATUSBAR(statusbar), ci, "Loading groups data...");
    gtk_widget_show(statusbar);

    /* Need this to fresh up the gui while loading pkgs */
    while(gtk_events_pending())
	gtk_main_iteration();

    for (i = alpm_db_getgrpcache(local); i; i = alpm_list_next(i)) {
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter,
	    0, alpm_list_getdata(i),
	-1);
    }

    gtk_statusbar_push(GTK_STATUSBAR(statusbar), ci, "Loading groups data... DONE");
}

/* Load files list into textview */
void _load_files_textview(char *pkgname) {
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    PM_LIST *i;
    PM_PKG *pkg;

    buffer = gtk_text_view_get_buffer(filesview);

    gtk_text_buffer_set_text (buffer, "", 0);
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
    gtk_text_buffer_insert(buffer, &iter, "Files in package:\n", -1);

    pkg = alpm_db_readpkg(local, pkgname);

    for (i = alpm_pkg_getinfo(pkg, PM_PKG_FILES); i; i = alpm_list_next(i)) {
	gtk_text_buffer_insert(buffer, &iter, "   /", -1);
	gtk_text_buffer_insert(buffer, &iter, (char *)alpm_list_getdata(i), -1);
	gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    }

    gtk_text_view_set_buffer(filesview, buffer);
}

/* Load all other stuff about packages into textview */
void _load_info_textview(char *pkgname) {
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    PM_LIST *i;
    PM_PKG *pkg;

    buffer = gtk_text_view_get_buffer(infoview);
    
    gtk_text_buffer_set_text (buffer, "", 0);
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
    gtk_text_buffer_insert(buffer, &iter, "Information:\n", -1);

    pkg = alpm_db_readpkg(local, pkgname);

    gtk_text_buffer_insert(buffer, &iter, "Name:          ", -1);
    gtk_text_buffer_insert(buffer, &iter, (char *)alpm_pkg_getinfo(pkg, PM_PKG_NAME), -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "Version:       ", -1);
    gtk_text_buffer_insert(buffer, &iter, (char *)alpm_pkg_getinfo(pkg, PM_PKG_VERSION), -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
//    gtk_text_buffer_insert(buffer, &iter, "Groups:        ", -1);
//    gtk_text_buffer_insert(buffer, &iter, (char *)alpm_pkg_getinfo(pkg, PM_PKG_GROUPS), -1);
//    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "Packager:      ", -1);
    gtk_text_buffer_insert(buffer, &iter, (char *)alpm_pkg_getinfo(pkg, PM_PKG_PACKAGER), -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "URL:           ", -1);
    gtk_text_buffer_insert(buffer, &iter, (char *)alpm_pkg_getinfo(pkg, PM_PKG_URL), -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
//    gtk_text_buffer_insert(buffer, &iter, "License:       ", -1);
//    gtk_text_buffer_insert(buffer, &iter, (char *)alpm_pkg_getinfo(pkg, PM_PKG_LICENSE), -1);
//    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "Architecture:   ", -1);
    gtk_text_buffer_insert(buffer, &iter, (char *)alpm_pkg_getinfo(pkg, PM_PKG_ARCH), -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
    gtk_text_buffer_insert(buffer, &iter, "Description:   ", -1);
    gtk_text_buffer_insert(buffer, &iter, (char *)alpm_pkg_getinfo(pkg, PM_PKG_DESC), -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);

    if (!strcmp(REPO, "local")) {
        gtk_text_buffer_insert(buffer, &iter, "LOCALBA VAGYUNK\n", -1);
    } else {
        gtk_text_buffer_insert(buffer, &iter, "NEM VAGYUNK LOCALBA\n", -1);
    }

    gtk_text_view_set_buffer(infoview, buffer);
}
