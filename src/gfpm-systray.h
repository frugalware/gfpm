#ifndef __GFPM_SYSTRAY_H__
#define __GFPM_SYSTRAY_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>

void gfpm_systray_init (void);
void gfpm_systray_free (void);
void gfpm_systray_set_visible (gboolean);

#endif
