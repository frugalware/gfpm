/*
 *  gfpm-util.c for gfpm
 *
 *  Copyright (C) 2007 by Priyank Gosalia <priyankmg@gmail.com>
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

#include "gfpm-util.h"

char *
gfpm_bold (const char *text)
{
	if (text == NULL)
		return NULL;
	return ((char*)g_markup_printf_escaped("<b>%s</b>",text));
}

GList *
gfpm_pmlist_to_glist (PM_LIST *list)
{
	GList	*ret = NULL;
	PM_LIST *i = NULL;

	if (list == NULL)
		return ret;
	for (i=pacman_list_first(list);i;i=pacman_list_next(i))
		ret = g_list_append (ret, (char*)pacman_list_getdata(i));

	return ret;
}

gint
gfpm_vercmp (const char *str1, const char *str2)
{
	int		l1 = 0, l2 = 0;
	char		st1[l1];
	char		st2[l2];
	register int	i;
	int		j = 0;
	int		v1, v2;
	
	if (str1 == NULL || str2 == NULL)
		return -1;
	l1 = strlen(str1);
	l2 = strlen(str2);
	for (i=0;i<l1;i++)
	{
		if (isdigit(str1[i]))
		{
			st1[j] = str1[i];
			j++;
		}
	}
	st1[j] = 0;
	for (i=0,j=0;i<l2;i++)
	{
		if (isdigit(str1[i]))
		{
			st2[j] = str2[i];
			j++;
		}
	}
	st2[j] = 0;

	v1 = atoi (st1);
	v2 = atoi (st2);
	if (v1 == v2)
		return 0;
	if (v1 < v2)
		return -1;
	if (v1 > v2)
		return 1;
	
	return -1;
}
	