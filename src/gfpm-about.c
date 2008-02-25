/*
 *  gfpm-about.c for gfpm
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
static GtkWidget *about_dlg = NULL;
static GdkPixbuf *about_pixbuf = NULL;
static const gchar *authors[] = { \
				"Priyank M. Gosalia <priyankmg@gmail.com>",
				"Christian Hamar <krics@linuxforum.hu>" ,
				"Miklos Nemeth <desco@frugalware.org>",
				NULL
			};

static const gchar *artists[] = { \
				"Viktor Gondor <nadfoka@frugalware.org>",
				"Sekkyumu <charavel.olivier@gmail.com>",
				"Priyank Gosalia <priyankmg@gmail.com>",
				NULL
			};

static const gchar translators[] = \
				"Martin Burda <brdam@email.cz> (cs_CZ)\n"
				"Carl Andersen <carl@frugalware.dk> (da_DK)\n"
				"Manuel Peral <mcklaren@gmail.com> (es_ES)\n"
				"Michel Hermier <michel.hermier@gmail.com> (fr_FR)\n"
				"Miklos Vajna <vmiklos@frugalware.org> (hu_HU)\n"
				"Patric Werme <xenonpower@clovermail.net> (sv_SE)\n";

static void gfpm_about_dlg_create (void);
static void gfpm_about_dlg_hide (void);

static void
gfpm_about_dlg_create (void)
{
	gchar *ver = NULL;
	GList *list;

	if (!about_pixbuf)
		about_pixbuf = gfpm_get_icon ("gfpm", 128);
	ver = g_strdup_printf ("%s (%s)", VERSION, GFPM_RELEASE_NAME);
	about_dlg = gtk_about_dialog_new ();
	gtk_about_dialog_set_name (GTK_ABOUT_DIALOG(about_dlg), PACKAGE);
	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about_dlg), ver);
	gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG(about_dlg), _("(C) 2006-2008 Frugalware Developer Team (GPL)"));
	gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG(about_dlg), _("A graphical package manager for Frugalware Linux"));
	gtk_about_dialog_set_license (GTK_ABOUT_DIALOG(about_dlg), license);
	gtk_about_dialog_set_website (GTK_ABOUT_DIALOG(about_dlg), "http://www.frugalware.org/");
	gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG(about_dlg), "http://www.frugalware.org/");
	gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG(about_dlg), about_pixbuf);
	gtk_about_dialog_set_wrap_license (GTK_ABOUT_DIALOG(about_dlg), TRUE);
	gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG(about_dlg), authors);
	gtk_about_dialog_set_artists (GTK_ABOUT_DIALOG(about_dlg), artists);
	gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG(about_dlg), translators);
	g_signal_connect (G_OBJECT(about_dlg), "destroy", G_CALLBACK(gtk_widget_destroyed), &about_dlg);

	list = gtk_container_get_children (GTK_CONTAINER((GTK_DIALOG(about_dlg))->action_area));
	list = list->next;
	list = list->next;
	g_signal_connect (G_OBJECT(list->data), "clicked", G_CALLBACK(gfpm_about_dlg_hide), NULL);
	g_free (ver);

	return;
}

static void
gfpm_about_dlg_hide (void)
{
	gtk_widget_hide (about_dlg);

	return;
}

void
gfpm_about (void)
{
	if (about_dlg == NULL)
		gfpm_about_dlg_create ();

	gtk_widget_show (about_dlg);

	return;
}

