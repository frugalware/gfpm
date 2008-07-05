/*
 *  gfpm-prefs.c for gfpm
 *
 *  Copyright (C) 2008 by Priyank Gosalia <priyankmg@gmail.com>
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
 
#include "gfpm-prefs.h"
#include "gfpm-messages.h"
#include "gfpm-interface.h"
#include "gfpm-util.h"
#include "gfpm-db.h"


static GtkWidget *gfpm_prefs_log_check;
static GtkWidget *gfpm_prefs_log_location;

static GtkWidget *gfpm_prefs_holdpkg_tvw;
static GtkWidget *gfpm_prefs_ignorepkg_tvw;

static GList *gfpm_prefs_holdpkg_list = NULL;
static GList *gfpm_prefs_ignorepkg_list = NULL;


static void gfpm_prefs_populate_holdpkg (void);

void
gfpm_prefs_init (void)
{
	gfpm_prefs_holdpkg_tvw = gfpm_get_widget ("gfpm_prefs_holdpkg_tvw");
	gfpm_prefs_ignorepkg_tvw = gfpm_get_widget ("gfpm_prefs_ignorepkg_tvw");
	
	gfpm_prefs_populate_holdpkg ();
	while (gfpm_prefs_holdpkg_list != NULL)
	{
		g_print ("%s\n", (char*)gfpm_prefs_holdpkg_list->data);
		gfpm_prefs_holdpkg_list = gfpm_prefs_holdpkg_list->next;
	}

	return;
}

static void
gfpm_prefs_populate_holdpkg (void)
{
	PM_LIST	*list = NULL;
	long	data;
	
	/* free the old list */
	if (gfpm_prefs_holdpkg_list != NULL)
		g_list_free (gfpm_prefs_holdpkg_list);
	
	/* populate the new list */
	pacman_parse_config (CFG_FILE, NULL, "");
	if (!pacman_get_option(PM_OPT_HOLDPKG, &data))
	{
		list = (PM_LIST*)data;
		while (list != NULL)
		{
			gfpm_prefs_holdpkg_list = g_list_append (gfpm_prefs_holdpkg_list,
								g_strdup(pacman_list_getdata(list)));
			list = pacman_list_next (list);
		}
	}
	
	return;
}

static void
gfpm_prefs_populate_ignorepkg (void)
{
	PM_LIST	*list = NULL;
	long	data;
	
	/* free the old list */
	if (gfpm_prefs_ignorepkg_list != NULL)
		g_list_free (gfpm_prefs_ignorepkg_list);
	
	/* populate the new list */
	pacman_parse_config (CFG_FILE, NULL, "");
	if (!pacman_get_option(PM_OPT_IGNOREPKG, &data))
	{
		list = (PM_LIST*)data;
		while (list != NULL)
		{
			gfpm_prefs_ignorepkg_list = g_list_append (gfpm_prefs_ignorepkg_list,
									g_strdup(pacman_list_getdata(list)));
			list = pacman_list_next (list);
		}
	}

	return;
}

