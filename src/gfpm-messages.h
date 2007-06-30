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

void gfpm_error (const char *);

void gfpm_message (const char *);

gint gfpm_question (const char *);

char * gfpm_input (const char *, const char *, int *);

#endif
