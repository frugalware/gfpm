#ifndef __GFPM_UTIL_H__
#define __GFPM_UTIL_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <pacman.h>
#include "gfpm.h"

char * gfpm_bold (const char *);

GList * gfpm_pmlist_to_glist (PM_LIST *);

#endif
