/*
 *  gfpm-about.c for gfpm
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include "gfpm.h"
#include "gfpm-util.h"
#include "gfpm-about.h"

static gchar	*license =
("This program is free software; you can redistribute it and/or "
"modify it under the terms of the GNU General Public Licence as "
"published by the Free Software Foundation; either version 2 of the "
"Licence, or (at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful, "
"but WITHOUT ANY WARRANTY; without even the implied warranty of "
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU "
"General Public Licence for more details.\n"
"\n"
"You should have received a copy of the GNU General Public Licence "
"along with this program; if not, write to the Free Software "
"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, "
"MA  02110-1301  USA");


extern GtkWidget *gfpm_mw;
static GdkPixbuf *about_pixbuf = NULL;
static gchar *authors[] = {	"Priyank M. Gosalia <priyankmg@gmail.com>",
				"Christian Hamar <krics@linuxforum.hu>" ,
				"Miklos Nemeth <desco@frugalware.org>",
				NULL
			};
static gchar *translators[] = { "Vajna Miklos <vmiklos@frugalware.org> (hu)",
				"Michel Hermier <michel.hermier@gmail.com> (fr)",
				NULL
			};

static gchar *artists[] = {	"Viktor Gondor <nadfoka@frugalware.org>",
				"Sekkyumu <charavel.olivier@gmail.com>",
				"Priyank Gosalia <priyankmg@gmail.com>",
				NULL
			};
void
gfpm_about (void)
{
	gchar *ver = g_strdup_printf ("%s (%s)", VERSION, GFPM_RELEASE_NAME);
	if (!about_pixbuf)
		about_pixbuf = gfpm_get_icon ("gfpm", 128);
	gtk_show_about_dialog (GTK_WINDOW(gfpm_mw),
				"name", PACKAGE,
				"version", ver,
				"copyright", _("(C) 2006-2007 Frugalware Developer Team (GPL)"),
				"comments", _("A graphical package manager for Frugalware Linux."),
				"license", license,
				"authors", authors,
				"artists", artists,
				"translator-credits", _(translators),
				"website", "http://www.frugalware.org/",
				"website-label", "http://www.frugalware.org/",
				"logo", about_pixbuf,
				"wrap-license", TRUE,
				NULL);
	g_free (ver);
	return;
}

