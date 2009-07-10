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
#include "gfpm-prefs.h"
#include "gfpm-db.h"

#define UI_FILE	"/share/gfpm/gfpm.ui"

GtkBuilder *gb = NULL;

int
main (int argc, char *argv[])
{
	gchar	*path;
	char	*tmp = NULL;
	int		opt;
	int		longopt_index;
	int		arg = 0;
	GError	*error = NULL;

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	/* parse command-line arguments */
	static struct option long_options[] = {
		{"help", 0, NULL, 'h'},
		{"version", 0, NULL, 'v'},
		{"add", 1, NULL, 'A'},
		{NULL, 0, NULL, 0}
	};
	
	while ((opt = getopt_long(argc, argv, "hvA:", long_options, &longopt_index)) > 0)
	{
		switch (opt)
		{
			case 'v':
				tmp = g_strdup_printf ("%s version %s (%s)\n",
							g_ascii_strdown(PACKAGE,strlen(PACKAGE)),
							VERSION,
							GFPM_RELEASE_NAME);
				fprintf (stdout, tmp);
				g_free (tmp);
				return 0;
			case 'A':
				if (optarg)
				{
					tmp = g_strdup (optarg);
					arg |= ARG_ADD;
				}
				break;
			case 'h':
			default:
				fprintf(stderr, "usage: %s [options]\n", basename(argv[0]));
				fprintf(stderr, "  -A, --add <file>		install a package from file\n");
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

	gb = gtk_builder_new ();
	if (!gtk_builder_add_from_file(gb, path, &error))
	{
		gchar *msg = g_strdup_printf ("%s\n\n%s",
				_("Failed to initialize interface due to the following error(s) in gfpm.ui:"),
				error->message);
		gfpm_error (_("Interface initialization Failed"), msg);
		g_free (msg);
		return 1;
	}
	g_free (path);
	gtk_builder_connect_signals (gb, NULL);

	if (pacman_initialize ("/") == -1)
	{
		gfpm_error (_("Error initializing libpacman"), _("Failed to initialize libpacman"));
		return 1;
	}
	/* initialize configuration subsystem */
	gfpm_config_init ();
	/* initialize everything else */
	gfpm_interface_init (arg, (void*)tmp);
	
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

