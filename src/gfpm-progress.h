#ifndef __GFPM_PROGRESS_H__
#define __GFPM_PROGRESS_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <pacman.h>
#include <gtk/gtk.h>

typedef void* netbuf;

void gfpm_progress_init (void);

void gfpm_progress_event (unsigned char, void *, void *);

int gfpm_progress_update (netbuf *, int, void *);

void gfpm_progress_install (unsigned char, char *, int, int, int);

void gfpm_progress_show (gboolean, gint);

void gfpm_progress_set_main_text (const char *msg);

void gfpm_progress_set_sub_text (const char *msg);

#endif

