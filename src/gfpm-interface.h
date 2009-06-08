#ifndef __GFPM_INTERFACE_H__
#define __GFPM_INTERFACE_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "gfpm.h"

/* a wrapper for glade_xml_get_widget(xml,"foo") */
GtkWidget * gfpm_get_widget (const char *);

/* Initialize gfpm interface. Call this function before any other interface functions */
void gfpm_interface_init (ARGS arg, void* argdata);

/* update status */
void gfpm_update_status (const char *);

/* setup repository combobox */
void gfpm_interface_setup_repo_combos (void);

/* setup gfpm's package treeview */
void gfpm_setup_pkgs_tvw (void);

/* populate gfpm's group treeview for a specified repo */
void gfpm_load_groups_tvw (const char *);

/* populate gfpm's package treeview for a specified group */
void gfpm_load_pkgs_tvw (const char *);

/* populate gfpm's info treeview for a specified package */
void gfpm_load_info_tvw (const char *, GtkTreeView *);

/* populate files textview for a specied package */
void gfpm_load_files_txtvw (const char *, gboolean);

/* populate changelog textview for a specied package */
void gfpm_load_changelog_txtvw (const char *, gboolean);

/* apply button callback. this is public because it's been shared by other components */
void cb_gfpm_apply_btn_clicked (GtkButton *, gpointer);

/* show/hide 'compressed size' column in package treeview */
void gfpm_pkgs_show_compressed_size (gboolean check);

/* show/hide 'uncompressed size' column in package treeview */
void gfpm_pkgs_show_uncompressed_size (gboolean check);

#endif
