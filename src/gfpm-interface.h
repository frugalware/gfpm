#ifndef __GFPM_INTERFACE_H__
#define __GFPM_INTERFACE_H__

#include <glade/glade.h>
#include <gtk/gtk.h>
#include "gfpm-packagelist.h"

void gfpm_interface_init (void);

void gfpm_load_groups_treeview (char *);

void gfpm_load_pkgs_treeview (char *);

void gfpm_load_info_treeview (char *, gboolean);

void gfpm_load_files_textview (char *, gboolean);

#endif
