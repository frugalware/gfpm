/***************************************************************************
 *  gfpm-db.c
 *  Author: Priyank Gosalia <priyankmg@gmail.com>	
 *  Copyright 2006-2007 Frugalware Developer Team
 ****************************************************************************/

/*
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

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include "gfpm-db.h"

PM_DB *gfpmdb = NULL;
PM_DB *localdb = NULL;

char *repository = NULL;

int
gfpm_db_load (const char *repo)
{
	int ret = 0;	

	if (gfpmdb != NULL)
	{
		pacman_db_unregister (gfpmdb);
		gfpmdb = NULL;
	}
	if (NULL == (gfpmdb = pacman_db_register (repo)))
		ret = 1;
	else
	{
		if (repository != NULL)		
			free (repository);
		asprintf (&repository, "%s", repo);
	}

	return ret;
}

void
gfpm_db_init_localdb (void)
{
	int ret = 0;
	
	if (localdb != NULL)
		pacman_db_unregister (localdb);

	if (NULL == (localdb = pacman_db_register ("local")))
		ret = 1;

	return ret;
}

int
gfpm_db_is_local (void)
{
	if ((strcmp(repository, "local")==0) && (localdb != NULL))
		return 0;
	
	return -1;
}



