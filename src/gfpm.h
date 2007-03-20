#ifndef __GFPM_H__
#define __GFPM_H__

#include <alpm.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>

#define _(string) gettext (string)

PM_DB *gfpmdb;

#endif
