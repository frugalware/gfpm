#ifndef __GFPM_DB_H__
#define __GFPM_DB_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <alpm.h>

int gfpm_db_load (const char *repo);

void gfpm_db_init_localdb (void);

int gfpm_db_is_local (void);

#endif

