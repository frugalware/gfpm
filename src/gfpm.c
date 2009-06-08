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
#include <getopt.h>

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
	gchar	*path;
	int		opt;
	int		longopt_index;

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* parse command-line arguments */
	static struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"version", 0, NULL, 'v'},
		{NULL, 0, NULL, 0}
	};
	
	while ((opt = getopt_long(argc, argv, "hv", long_options, &longopt_index)) > 0)
	{
		switch (opt)
		{
			char *vstr = NULL;
			case 'v':
				vstr = g_strdup_printf ("%s version %s (%s)\n",
							g_ascii_strdown(PACKAGE,strlen(PACKAGE)),
							VERSION,
							GFPM_RELEASE_NAME);
				fprintf (stdout, vstr);
				g_free (vstr);
				return 0;
			case 'h':
			default:
				fprintf(stderr, "usage: %s [options]\n", basename(argv[0]));
				fprintf(stderr, "  -h, --help			display this help\n");
				fprintf(stderr, "  -v, --version			version information\n");
				return 1;
		}
	}
	
	/* invite trouble */
	g_thread_init (NULL);
	
	/* initialize internal gdk threads mutex */
	gdk_threads_init ();
	
	gtk_init (&argc, &argv);
	gdk_threads_enter ();
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
	gtk_main ();
	
	/* phew */
	gdk_threads_leave ();
	
	gfpm_db_cleanup ();
	gfpm_config_free ();
	gfpm_prefs_cleanup ();
	pacman_release ();

	return 0;
}

