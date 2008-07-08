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

#define CONF_FILE		"/etc/pacman-g2.conf"
#define REPONAME_MAX_SIZE	32
#define SERVER_MAX_SIZE		255

typedef struct __gfpm_repo_t {
	gboolean 	enabled;
	gboolean	delete;
	GList		*header;
	char		name[REPONAME_MAX_SIZE+1];
	GList		*servers;
	GList		*footer;
} gfpm_repo_t;

typedef struct __gfpm_repolist_t {
	gint	n;
	GList	*header;
	GList	*list;
} gfpm_repolist_t;

typedef struct __gfpm_server_entry_t {
	GList *comments;
	gchar url[SERVER_MAX_SIZE+1];
} gfpm_server_entry_t;

void gfpm_repomanager_init (void);

void gfpm_repomanager_show (void);

#endif
