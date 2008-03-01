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

GtkWidget * gfpm_get_widget (const char *);
void gfpm_update_status (const char *);
void gfpm_load_groups_tvw (const char *);
void gfpm_load_pkgs_tvw (const char *);
void gfpm_load_info_tvw (const char *);
void gfpm_load_files_txtvw (const char *, gboolean);
void gfpm_load_changelog_txtvw (const char *, gboolean);
void gfpm_interface_init (void);
void gfpm_interface_setup_repo_combos (void);
void cb_gfpm_apply_btn_clicked (GtkButton *, gpointer);

#endif
