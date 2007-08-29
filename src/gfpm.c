/*
 *  gfpm.c for gfpm
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
#include <locale.h>
#include <gtk/gtk.h>
#include <glade/glade.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gfpm.h"
#include "gfpm-interface.h"
#include "gfpm-messages.h"
#include "gfpm-db.h"

#define GLADE_FILE	"/share/gfpm/gfpm.glade"

GladeXML *xml = NULL;

int
main (int argc, char *argv[])
{
	gchar *path;

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);

	path = g_strdup_printf ("%s%s", PREFIX, GLADE_FILE);
	xml = glade_xml_new (path, NULL, NULL);
	g_free (path);

	if (!xml)
	{
		gfpm_error (_("Failed to initialize interface."));
		return 1;
	}

	glade_xml_signal_autoconnect (xml);

	if (pacman_initialize ("/") == -1)
	{
		gfpm_error (_("Failed to initialize libpacman"));
		return 1;
	}

	gfpm_interface_init ();
	gtk_main ();
	gfpm_db_cleanup ();
	pacman_release ();

	return 0;
}

