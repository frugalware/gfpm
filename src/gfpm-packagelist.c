/***************************************************************************
 *  gfpm-packagelist.c
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

#include "gfpm-packagelist.h"

GList *
gfpm_install_package_list_insert (GList *list, gchar *item)
{
	GList *ret;

	/* Remove the item if it already exists in the list */	
	if (list!=NULL && NULL != g_list_find (list, (gpointer)item))
	{
		ret = g_list_remove (list, (gpointer)item);
		return ret;
	}

	ret = g_list_append (list, (gpointer)item);
	return ret;
}

GList *
gfpm_remove_package_list_insert (GList *list, gchar *item)
{
	GList *ret;

	/* Remove the item if it already exists in the list */	
	if (list!=NULL && NULL != g_list_find (list, (gpointer)item))
	{
		ret = g_list_remove (list, (gpointer)item);
		return ret;
	}

	ret = g_list_append (list, (gpointer)item);
	return ret;
}

gboolean
gfpm_package_list_find (GList *list, gchar *item)
{
	GList *lst = NULL;
	gboolean ret = TRUE;

	if (list != NULL)
		lst = g_list_find (list, (gpointer)item);
	else
		ret = FALSE;
	
	if (lst != NULL && ret != FALSE)
		ret = TRUE;
	else
		ret = FALSE;

	return ret;
}

GList *
gfpm_remove_package_list_remove (GList *list, gchar *item)
{
	GList *ret = NULL;	
	
	if (list != NULL)
		ret = g_list_remove (list, (gpointer)item);
	else
		ret = list;

	return ret;
}



