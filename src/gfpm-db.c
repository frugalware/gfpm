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
#define FW_CURRENT	"frugalware-current"
#define FW_LOCAL	"local"

#include <pacman.h>
#include "gfpm-db.h"
#include "gfpm.h"

PM_DB *sync_db = NULL;
PM_DB *local_db = NULL;

char *repo = NULL;

int
gfpm_db_init (void)
{
	if (NULL == (sync_db=pacman_db_register(FW_CURRENT)))
		return 1;
	if (NULL == (local_db=pacman_db_register(FW_LOCAL)))
		return 1;
	asprintf (&repo, "%s", FW_CURRENT);

	return 0;
}

void
gfpm_db_cleanup (void)
{
	pacman_db_unregister (sync_db);
	pacman_db_unregister (local_db);
	free (repo);

	return;
}
	
