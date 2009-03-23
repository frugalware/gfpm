/*
 *  gfpm.c for gfpm
 *
 *  Copyright (C) 2006-2009 by Priyank Gosalia <priyankmg@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gfpm.h"
#include "gfpm-config.h"
#include "gfpm-interface.h"
#include "gfpm-messages.h"
#include "gfpm-db.h"

#define UI_FILE	"/share/gfpm/gfpm.glade"

GladeXML *xml = NULL;

int
main (int argc, char *argv[])
{
	gchar *path;

	/* invite trouble */
	g_thread_init (NULL);
	
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gtk_init (&argc, &argv);

	path = g_strdup_printf ("%s%s", PREFIX, UI_FILE);

	if (!(xml=glade_xml_new(path, NULL, NULL)))
	{
		gfpm_error (_("Interface initialization Failed"), _("Failed to initialize interface"));
		return 1;
	}
	g_free (path);
	glade_xml_signal_autoconnect (xml);

	if (pacman_initialize ("/") == -1)
	{
		gfpm_error (_("Error initializing libpacman"), _("Failed to initialize libpacman"));
		return 1;
	}
	/* initialize configuration subsystem */
	gfpm_config_init ();
	/* initialize everything else */
	gfpm_interface_init ();
	
	/* the main loop */
	gdk_threads_enter ();
	gtk_main ();
	gdk_threads_leave ();
	
	gfpm_db_cleanup ();
	gfpm_config_free ();
	gfpm_prefs_cleanup ();
	pacman_release ();

	return 0;
}

