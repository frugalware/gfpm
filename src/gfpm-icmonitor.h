#ifndef __GFPM_ICMONITOR_H__
#define __GFPM_ICMONITOR_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>

int gfpm_icmonitor_init (void);

void gfpm_icmonitor_start_monitor (void);

void gfpm_icmonitor_stop_monitor (void);

void gfpm_icmonitor_reset_ic (void);

gboolean gfpm_icmonitor_is_ic_changed (void);

gboolean gfpm_icmonitor_is_running (void);

#endif /* __GFPM_ICMONITOR_H__ */
