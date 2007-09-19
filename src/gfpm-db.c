/*
 *  gfpm-db.c for gfpm
 *
 *  Copyright (C) 2006-2007 by Priyank Gosalia <priyankmg@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#define _GNU_SOURCE
#define CFG_FILE	"/etc/pacman.conf"

#include <pacman.h>
#include "gfpm-db.h"
#include "gfpm.h"

PM_DB *sync_db = NULL;
PM_DB *local_db = NULL;

char *repo = NULL;
static GList *dblist = NULL;

static void _db_callback (char *section, PM_DB *db);

int
gfpm_db_init (void)
{
	if (NULL == (local_db=pacman_db_register(FW_LOCAL)))
		return 1;

	return 0;
}

void
gfpm_db_register (const char *dbname)
{
	if (sync_db != NULL)
	{
		pacman_db_unregister (sync_db);
		sync_db = NULL;
		g_free (repo);
	}
	if (strcmp(dbname,"local"))
		sync_db = pacman_db_register ((char*)dbname);
	asprintf (&repo, dbname);

	return;
}

void
gfpm_db_reset_localdb (void)
{
	if (local_db)
	{
		pacman_db_unregister (local_db);
		local_db = pacman_db_register (FW_LOCAL);
	}

	return;
}

void
gfpm_db_cleanup (void)
{
	pacman_db_unregister (sync_db);
	if (local_db)
		pacman_db_unregister (local_db);
	free (repo);

	return;
}

static void
_db_callback (char *section, PM_DB *db)
{
	dblist = g_list_append (dblist, db);

	return;
}

int
gfpm_db_populate_repolist (void)
{
	/* get the list of usable repositories */
	if (pacman_parse_config (CFG_FILE, _db_callback, "") == -1)
	{
		printf ("error parsing config file");
		return 1;
	}

	return 0;
}

GList *
gfpm_db_get_repolist (void)
{
	return dblist;
}

