#ifndef __GFPM_MESSAGES_H__
#define __GFPM_MESSAGES_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include "gfpm.h"

void gfpm_messages_init (void);
void gfpm_apply_dlg_show (GList *, GList *);
void gfpm_apply_dlg_show_inst_box (gboolean);
void gfpm_apply_dlg_show_rem_box (gboolean);

void gfpm_error (const char *);

void gfpm_message (const char *);

gint gfpm_question (const char *);

gint gfpm_plist_question (const char *, GList *);

void gfpm_plist_message (const char *, GtkMessageType, GList *);

char * gfpm_input (const char *, const char *, int *);

#endif
