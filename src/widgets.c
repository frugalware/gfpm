/***************************************************************************
 *  widgets.c
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
#include <string.h>
#include "widgets.h"
#include "gfpm.h"

/* Create combobox_repos widget */
void gfpm_create_combobox_repos(void)
{
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkTreeIter iter;

    store = gtk_list_store_new(1, G_TYPE_STRING);

    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox_repos), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox_repos), renderer,
                                   "text", 0, NULL);

    gtk_combo_box_set_model(GTK_COMBO_BOX(combobox_repos), GTK_TREE_MODEL(store));

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       0, "frugalware-current", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
                       0, "local", -1);

    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_repos), 0);

    g_signal_connect(combobox_repos, "changed", G_CALLBACK(_combobox_repos_select), NULL);

    return;
}

/* Create treeview widget for *groups* */
void gfpm_create_group_treeview(void)
{
    GtkCellRenderer *renderer;
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
void gfpm_create_pkgs_treeview(void)
{
    GtkCellRenderer *renderer;
    GtkListStore *store;
    GtkTreeSelection *selection;

    store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pkgs_treeview),
            -1,
            "Package Name", renderer, "text", 0,
            NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pkgs_treeview),
            -1,
            "Available", renderer, "text", 1,
            NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pkgs_treeview),
            -1,
            "Installed", renderer, "text", 2,
            NULL);


    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(pkgs_treeview),
            -1,
            "Description", renderer, "text", 3,
            NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(pkgs_treeview), GTK_TREE_MODEL(store));

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pkgs_treeview));
    g_signal_connect(selection, "changed", G_CALLBACK(_pkgs_treeview_select), NULL);

    return;
}

/* Create treeview widget for *infobox* */
void gfpm_create_info_treeview(void)
{
    GtkCellRenderer *renderer;
    GtkListStore *store;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(info_treeview),
            -1,
            "Info", renderer, "text", 0,
            NULL);

    renderer = gtk_cell_renderer_text_new();
    g_object_set(renderer, "wrap-width", 350, NULL);
    g_object_set(renderer, "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(info_treeview),
            -1,
            "Value", renderer, "text", 1,
            NULL);

    gtk_tree_view_set_model(GTK_TREE_VIEW(info_treeview), GTK_TREE_MODEL(store));
    g_object_set(info_treeview, "hover-selection", TRUE, NULL);

    //    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(pkgs_treeview));
    //    g_signal_connect(selection, "changed", G_CALLBACK(_pkgs_treeview_select), NULL);

    return;
}

/* Callback function for group_treeview (catch clicks in treeview) */
int _group_treeview_select()
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *s;
    char *groupname;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(group_treeview));
    s = gtk_tree_view_get_selection(GTK_TREE_VIEW(group_treeview));

    if (gtk_tree_selection_get_selected(s, &model, &iter))
    {
        gtk_tree_model_get(model, &iter, 0, &groupname, -1);
        _update_pkgs_treeview(groupname);
        return(0);
    }
    else
    {
        return(1);
    }
}

/* Callback function for selection in combobox_repos and fresh the groups */
int _combobox_repos_select()
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    char *reponame;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox_repos));
    gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combobox_repos), &iter);
    gtk_tree_model_get(model, &iter, 0, &reponame, -1);

    _clear_treeviews(); /* Need this to clean the old pkgs_ infos_ files_ views */
    _load_groups_treeview(reponame); /* Refresh the groups_treeview with new repo */
    return(0);
}

/* Callback function for pkgs_treeview (catch clicks in treeview) */
int _pkgs_treeview_select()
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *s;
    char *pkgname;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(pkgs_treeview));
    s = gtk_tree_view_get_selection(GTK_TREE_VIEW(pkgs_treeview));

    if (gtk_tree_selection_get_selected(s, &model, &iter))
    {
        gtk_tree_model_get(model, &iter, 0, &pkgname, -1);
        _load_files_textview(pkgname);
        _load_info_treeview(pkgname);
        return(0);
    }
    else
    {
        return(1);
    }
}

/* Updates the pkgs_treeview when clicked on something in group_treeview */
void _update_pkgs_treeview(char *gn)
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    PM_LIST *pkgnames, *i;
    PM_GRP *grp = alpm_db_readgrp(local, gn);
    PM_PKG *pkg;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(pkgs_treeview));
    gtk_list_store_clear(GTK_LIST_STORE(model));

    pkgnames = alpm_grp_getinfo(grp, PM_GRP_PKGNAMES);

    for (i = pkgnames; i; i = alpm_list_next(i))
    {
        pkg = alpm_db_readpkg(local, alpm_list_getdata(i));

        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           0, (char *)alpm_list_getdata(i),
                           1, (char *)alpm_pkg_getinfo(pkg, PM_PKG_VERSION),
                           3, (char *)alpm_pkg_getinfo(pkg, PM_PKG_DESC),
                           -1);
    }
}

/* Loads groups to group_treeview */
void _load_groups_treeview(char *repo)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    PM_LIST *i;
    guint ci;
    char *tmp;

    if(local != NULL)
    {
        alpm_db_unregister(local);
        local = alpm_db_register(repo);
        asprintf(&repository, "%s", repo);
    }
    else
    {
        local = alpm_db_register(repo);
        asprintf(&repository, "%s", repo);
    }

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(group_treeview));
    gtk_list_store_clear(GTK_LIST_STORE(model));

    ci = gtk_statusbar_get_context_id(GTK_STATUSBAR(statusbar), "-");
    gtk_statusbar_push(GTK_STATUSBAR(statusbar), ci, "Loading groups ...");
    gtk_widget_show(statusbar);

    /* Need this to fresh up the gui while loading pkgs */
    while(gtk_events_pending())
        gtk_main_iteration();

    for (i = alpm_db_getgrpcache(local); i; i = alpm_list_next(i))
    {
        asprintf(&tmp, "Loading groups ... [%s]", (char *)alpm_list_getdata(i));
        gtk_statusbar_push(GTK_STATUSBAR(statusbar), ci, tmp);
        while(gtk_events_pending())
            gtk_main_iteration();
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           0, (char *)alpm_list_getdata(i),
                           -1);
    }

    gtk_statusbar_push(GTK_STATUSBAR(statusbar), ci, "Loading groups ... DONE");
}

/* Load files list into textview */
void _load_files_textview(char *pkgname)
{
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    PM_LIST *i;
    PM_PKG *pkg;
    int r;

    if (!strcmp(repository, "local"))
        r = 1; /* in 'local' repo */
    else
        r = 0; /* in 'remote' repo */

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(filesview));

    gtk_text_buffer_set_text (buffer, "", 0);
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
    gtk_text_buffer_insert(buffer, &iter, "Files in package:\n", -1);
    if (r != 1)
    {
        gtk_text_buffer_insert(buffer, &iter, "We are in a remote repository. No file lists here. Only in local DB.\n", -1);
    }

    if (r != 0)
    {
        pkg = alpm_db_readpkg(local, pkgname);

        for (i = alpm_pkg_getinfo(pkg, PM_PKG_FILES); i; i = alpm_list_next(i))
        {
            gtk_text_buffer_insert(buffer, &iter, "   /", -1);
            gtk_text_buffer_insert(buffer, &iter, (char *)alpm_list_getdata(i), -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        }
    }
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(filesview), buffer);
}

/* Load all other stuff about packages into treeview */
void _load_info_treeview(char *pkgname)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    PM_LIST *i, *y;
    PM_PKG *pkg;
    GString *foo;
    char *tmp;
    int r;

    if (!strcmp(repository, "local"))
        r = 1; /* in 'local' repo */
    else
        r = 0; /* in 'remote' repo */

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(info_treeview));
    gtk_list_store_clear(GTK_LIST_STORE(model));

    pkg = alpm_db_readpkg(local, pkgname);

    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       0, "Name:", 1, (char *)alpm_pkg_getinfo(pkg, PM_PKG_NAME), -1);

    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       0, "Version:", 1, (char *)alpm_pkg_getinfo(pkg, PM_PKG_VERSION), -1);

    if (r != 0)
    {
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           0, "Packager:", 1, (char *)alpm_pkg_getinfo(pkg, PM_PKG_PACKAGER), -1);
    }

    if (r != 0)
    {
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           0, "URL:", 1, (char *)alpm_pkg_getinfo(pkg, PM_PKG_URL), -1);
    }

    if (r != 1)
    {
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        asprintf(&tmp, "%ld", (long)alpm_pkg_getinfo(pkg, PM_PKG_SIZE));
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           0, "Size (Compressed):", 1, (char *)tmp, -1);
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        asprintf(&tmp, "%ld", (long)alpm_pkg_getinfo(pkg, PM_PKG_USIZE));
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           0, "Size (Uncompressed):", 1, (char *)tmp, -1);
    }

    if (r != 1)
    {
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           0, "SHA1SUM:", 1, (char *)alpm_pkg_getinfo(pkg, PM_PKG_SHA1SUM), -1);
    }

    y = alpm_pkg_getinfo(pkg, PM_PKG_DEPENDS);
    foo = g_string_new("");
    for (i = y; i; i = alpm_list_next(i))
    {
        foo = g_string_append(foo, (char *)alpm_list_getdata(i));
        foo = g_string_append(foo, " ");
    }
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       0, "Depends:", 1, (char *)foo->str, -1);

    y = alpm_pkg_getinfo(pkg, PM_PKG_PROVIDES);
    foo = g_string_new("");
    for (i = y; i; i = alpm_list_next(i))
    {
        foo = g_string_append(foo, (char *)alpm_list_getdata(i));
        foo = g_string_append(foo, " ");
    }
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       0, "Provides:", 1, (char *)foo->str, -1);

    y = alpm_pkg_getinfo(pkg, PM_PKG_CONFLICTS);
    foo = g_string_new("");
    for (i = y; i; i = alpm_list_next(i))
    {
        foo = g_string_append(foo, (char *)alpm_list_getdata(i));
        foo = g_string_append(foo, " ");
    }
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       0, "Conflicts:", 1, (char *)foo->str, -1);

    if (r != 0)
    {
        y = alpm_pkg_getinfo(pkg, PM_PKG_REQUIREDBY);
        foo = g_string_new("");
        for (i = y; i; i = alpm_list_next(i))
        {
            foo = g_string_append(foo, (char *)alpm_list_getdata(i));
            foo = g_string_append(foo, " ");
        }
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                           0, "Required by:", 1, (char *)foo->str, -1);
    }

    gtk_list_store_append(GTK_LIST_STORE(model), &iter);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                       0, "Description:", 1, (char *)alpm_pkg_getinfo(pkg, PM_PKG_DESC), -1);

}

/* Clear treeviews except groups_treeview */
void _clear_treeviews()
{
    GtkTreeModel *model;
    GtkTextBuffer *buffer;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(info_treeview));
    gtk_list_store_clear(GTK_LIST_STORE(model));

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(pkgs_treeview));
    gtk_list_store_clear(GTK_LIST_STORE(model));

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(pkgs_treeview));
    gtk_list_store_clear(GTK_LIST_STORE(model));

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(filesview));
    gtk_text_buffer_set_text (buffer, "", 0);
}
