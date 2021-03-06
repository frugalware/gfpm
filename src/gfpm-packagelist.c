/*
 *  gfpm-packagelist.c for gfpm
 *
 *  Copyright (C) 2006-2008 by Priyank Gosalia <priyankmg@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gfpm-packagelist.h"

GfpmList *install_list = NULL;
GfpmList *remove_list = NULL;

void
gfpm_package_list_add (GfpmListType type, const gchar *item)
{
	GfpmList *new = NULL;
	GfpmList *temp = NULL;

	new = (GfpmList *) malloc(sizeof(GfpmList));
	new->next = NULL;
	new->data = g_strdup_printf ("%s", item);

	if (type == GFPM_INSTALL_LIST)
	{
		if (install_list == NULL)
		{
			install_list = new;
			return;
		}
		else
		{
			temp = install_list;
		}
	}
	else if (type == GFPM_REMOVE_LIST)
	{
		if (remove_list == NULL)
		{
			remove_list = new;
			return;
		}
		else
		{
			temp = remove_list;
		}
	}

	do {
		if (!strcmp(temp->data, item))
		{
			g_free (new);
			return;
		}
		if (temp->next != NULL)
			temp = temp->next;
		else
			break;
	} while (temp != NULL);

	temp->next = new;

	return;
}

void
gfpm_package_list_del (GfpmListType type, const gchar *item)
{
	GfpmList *prev = NULL;
	GfpmList *temp = NULL;

	if ((type == GFPM_INSTALL_LIST && install_list == NULL) || (type == GFPM_REMOVE_LIST && remove_list == NULL))
		return;

	if (type == GFPM_INSTALL_LIST)
		temp = install_list;
	else
		temp = remove_list;

	for (; temp!=NULL; temp=temp->next)
	{
		if (strcmp (item, temp->data) == 0)
			break;
		prev = temp;
	}

	if (temp == NULL)
		return;

	if (prev != NULL)
		prev->next = temp->next;
	else
	{
		if (type == GFPM_INSTALL_LIST)
			install_list = temp->next;
		else
			remove_list = temp->next;
	}

	g_free (temp->data);
	g_free (temp);

	return;
}

void
gfpm_package_list_free (GfpmListType type)
{
	GfpmList *list = NULL;
	GfpmList *tmp;

	if (type == GFPM_INSTALL_LIST)
		list = install_list;
	else
		list = remove_list;

	while (list != NULL)
	{
		tmp = list->next;
		g_free (list->data);
		g_free (list);
		list = tmp;
	}
	if (type == GFPM_INSTALL_LIST)
		install_list = NULL;
	else
		remove_list = NULL;

	return;
}

/* checks whether a list is empty
 * returns 0 if empty
 * returns 1 if not empty
 * returns -1 on error
 */
int
gfpm_package_list_is_empty (GfpmListType type)
{
	if ((type != GFPM_INSTALL_LIST) && (type != GFPM_REMOVE_LIST))
		return -1;
	switch (type)
	{
		case GFPM_INSTALL_LIST:
			{
				if (install_list == NULL)
					return 0;
				else if (g_list_length((GList *)install_list)==0)
					return 0;
				else
					return 1;
				break;
			}
		case GFPM_REMOVE_LIST:
			{
				if (remove_list == NULL)
					return 0;
				else if (g_list_length((GList *)remove_list)==0)
					return 0;
				else
					return 1;
				break;
			}
		default: return -1;
	}
}

void
gfpm_package_list_print (GfpmListType type)
{
	GfpmList *list;

	if (type == GFPM_INSTALL_LIST)
		list = install_list;
	else
		list = remove_list;

	g_print ("*****************************\n");
	while (list != NULL)
	{
		g_print ("-> %s\n", list->data);
		list = list->next;
	}
	g_print ("*****************************\n");

	return;
}

