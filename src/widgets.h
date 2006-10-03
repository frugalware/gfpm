/***************************************************************************
 *  widgets.h
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
 
#include <gtk/gtk.h>
#include <stdio.h>

GtkWidget *group_treeview;
GtkWidget *pkgs_treeview;
GtkWidget *info_treeview;
GtkWidget *statusbar;
GtkWidget *filesview;
GtkWidget *combobox_repos;

#define PACKAGES_CURRENT 0
#define PACKAGES_ALL 1
#define PACKAGES_INSTALLED 2
#define PACKAGES_NOTINSTALLED 3
#define PACKAGES_REMOTE 4
#define PACKAGES_LOCAL 5

/* Simple treeview layout creation */
void gfpm_create_group_treeview(void);
void gfpm_create_pkgs_treeview(void);
void gfpm_create_info_treeview(void);
void gfpm_create_combobox_repos(void);
/* Callback for group_treeview selection */
int _group_treeview_select();
/* Callback for pkgs_treeview selection */
int _pkgs_treeview_select();
/* Callback for pkgs_treeview selection */
int _combobox_repos_select();

/* Update pkgs_treeview when clicked on a group in group_treeview */
void _update_pkgs_treeview(char *gn);
/* Loads groups data into group_treeview */
void _load_groups_treeview(char *repo);
/* Load file list into textview */
void _load_files_textview(char *pkgname);
/* Load infos into treeview */
void _load_info_treeview(char *pkgname);

/* Clear all treeviews and textviews, except groups_treeview */
void _clear_treeviews();
