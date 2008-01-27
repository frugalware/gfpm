#ifndef __GFPM_LOGVIEWER_H__
#define __GFPM_LOGVIEWER_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>

void gfpm_logviewer_init (void);

void gfpm_logviewer_show (void);

#endif
