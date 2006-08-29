#include <gtk/gtk.h>

GtkWidget *group_treeview;
GtkWidget *pkgs_treeview;
GtkWidget *statusbar;
GtkWidget *infoview;
GtkWidget *filesview;

/* Simple treeview layout creation */
void gfpm_create_group_treeview(void);
void gfpm_create_pkgs_treeview(void);
/* Callback for group_treeview selection */
int _group_treeview_select();
/* Callback for pkgs_treeview selection */
int _pkgs_treeview_select();

/* Update pkgs_treeview when clicked on a group in group_treeview */
void _update_pkgs_treeview(char *gn);
/* Loads groups data into group_treeview */
void _load_groups_treeview(char *repo);
/* Load file list into textview */
void _load_files_textview(char *pkgname);
/* Load infos into textview */
void _load_info_textview(char *pkgname);
