/*
 *  gfpm-util.c for gfpm
 *
 *  Copyright (C) 2007-2008 by Priyank Gosalia <priyankmg@gmail.com>
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

	if (list != NULL)
	{
		for (i=pacman_list_first(list);i;i=pacman_list_next(i))
			ret = g_list_append (ret, (char*)pacman_list_getdata(i));
	}

	return ret;
}

GdkPixbuf *
gfpm_get_icon (const char *icon, int size)
{
	GtkIconTheme 	*icon_theme = NULL;
	GdkPixbuf	*ret = NULL;
	GError		*error = NULL;

	icon_theme = gtk_icon_theme_get_default ();
	ret = gtk_icon_theme_load_icon (icon_theme, icon, size, 0, &error);
	
	return ret;
}

gint
gfpm_check_if_package_updatable (const gchar *package)
{
	gint ret = 0;
	PM_PKG *pm_glpkg = NULL;
	PM_PKG *pm_gspkg = NULL;
	extern PM_DB *local_db;
	extern PM_DB *sync_db;
	
	pm_glpkg = pacman_db_readpkg (local_db, package);
	pm_gspkg = pacman_db_readpkg (sync_db, package);
	if (pm_glpkg && pm_gspkg)
	{
		char *v1 = (char*)pacman_pkg_getinfo (pm_gspkg, PM_PKG_VERSION);
		char *v2 = (char*)pacman_pkg_getinfo (pm_glpkg, PM_PKG_VERSION);
		if (pacman_pkg_vercmp(v1,v2)==1)
		{
			ret = 1;
		}
	}
	pacman_pkg_free (pm_glpkg);
	pacman_pkg_free (pm_gspkg);

	return ret;
}

void
gfpm_update_iconcache (void)
{
	if (!g_file_test("/usr/bin/gtk-update-icon-cache", G_FILE_TEST_EXISTS))
		return;

	g_print ("updating hicoloricon cache\n");
	while (gtk_events_pending()) gtk_main_iteration ();
	system ("gtk-update-icon-cache -f -t /usr/share/icons/hicolor > /dev/null 2>&1");
	while (gtk_events_pending()) gtk_main_iteration ();

	return;
}

gchar *
gfpm_convert_to_utf8 (const char *str)
{
	gchar	*ret = NULL;
	GError	*error = NULL;
	if (str)
	{
		ret = g_convert (str, strlen(str), "UTF-8", "", NULL, NULL, &error);
		if (ret == NULL)
		{
			g_print ("Error converting string to utf-8: %s\n", error->message);
		}
	}
	
	return ret;
}


