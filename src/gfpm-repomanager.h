#ifndef __GFPM_REPOMANAGER_H__
#define __GFPM_REPOMANAGER_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <gtk/gtk.h>
#include <libfwutil.h>
#include "gfpm.h"

#define REPONAME_MAX_SIZE 32

typedef struct __gfpm_repo_t {
	char	name[REPONAME_MAX_SIZE+1];
	GList	*servers;
} gfpm_repo_t;

typedef struct __gfpm_repolist_t {
	gint	n;
	GList	*list;
} gfpm_repolist_t;

void gfpm_repomanager_init (void);

void gfpm_repomanager_show (void);

#endif