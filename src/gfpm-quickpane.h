#ifndef __GFPM_QUICKPANE_H__
#define __GFPM_QUICKPANE_H__

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <pacman.h>

void gfpm_quickpane_init (void);

void gfpm_quickpane_readme_btn_show (void);

void gfpm_quickpane_show (gboolean, gboolean, gboolean);

void gfpm_quickpane_readme_dlg_populate (const char *);

void gfpm_quickpane_readme_dlg_show (void);

#endif

