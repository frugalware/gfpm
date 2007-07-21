/***************************************************************************
 *  gfpm-progress.c
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

#define _GNU_SOURCE
#include <glade/glade.h>
#include "gfpm.h"
#include "gfpm-progress.h"

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

extern GladeXML *xml;

GtkProgressBar	*progressbar = NULL;
GtkWidget		*progresswindow = NULL;
GtkWidget		*main_label = NULL;
GtkWidget		*sub_label = NULL;

float rate;
int offset;
char reponame[PM_DLFNM_LEN+1];

void
gfpm_progress_init (void)
{
	pacman_parse_config ("/etc/pacman.conf", NULL, "");
	pacman_set_option (PM_OPT_DLCB, (long)gfpm_progress_update);
	pacman_set_option (PM_OPT_DLOFFSET, (long)&offset);
	pacman_set_option (PM_OPT_DLRATE, (long)&rate);
	pacman_set_option (PM_OPT_DLFNM, (long)reponame);

	progressbar = GTK_PROGRESS_BAR(glade_xml_get_widget (xml, "progressbar1"));
	progresswindow = glade_xml_get_widget (xml, "progresswindow");
	main_label = glade_xml_get_widget (xml, "main_pr_label");
	sub_label = glade_xml_get_widget (xml, "sub_pr_label");

	return;
}

void
gfpm_progress_show (gboolean show)
{
	if (show == TRUE)
		gtk_widget_show (progresswindow);
	else
		gtk_widget_hide (progresswindow);

	return;
}

int
gfpm_progress_update (netbuf *ctl, int xferred, void *arg)
{
	int		size;
	int		per;
	char	text[6];

	ctl = NULL;
	size = *(int*)arg;
	per = ((float)(xferred+offset) / size) * 100;
	sprintf (text, "%d %%", per);
	while (gtk_events_pending ())
		gtk_main_iteration ();
	gtk_progress_bar_set_text (progressbar, text);
	gtk_progress_bar_set_fraction (progressbar, (float)per/100);
	gfpm_progress_set_sub_text (reponame);

	return 1;
}

void
gfpm_progress_install (unsigned char event, char *pkgname, int percent, int howmany, int remain)
{
	char *main_text = NULL;
	char *sub_text = NULL;

	if (!pkgname)
		return;
	if (percent > 100)
		return;
	while (gtk_events_pending ())
		gtk_main_iteration ();
	switch (event)
	{
		case PM_TRANS_PROGRESS_ADD_START:
			if (howmany > 1)
				main_text = g_strdup (_("Installing packages"));
			else
				main_text = g_strdup (_("Installing package"));
			break;
		case PM_TRANS_PROGRESS_UPGRADE_START:
			if (howmany > 1)
				main_text = g_strdup (_("Upgrading packages"));
			else
				main_text = g_strdup (_("Upgrading package"));
			break;
		case PM_TRANS_PROGRESS_REMOVE_START:
			if (howmany > 1)
				main_text = g_strdup (_("Removing packages"));
			else
				main_text = g_strdup (_("Removing package"));
			break;
		case PM_TRANS_PROGRESS_CONFLICTS_START:
			if (howmany > 1)
				main_text = g_strdup (_("Checking packages for file conflicts"));
			else
				main_text = g_strdup (_("Checking package for file conflicts"));
			break;
	}
	gfpm_progress_set_main_text (main_text);
	sub_text = g_strdup_printf ("(%d/%d) %s", remain, howmany, pkgname);
	gfpm_progress_set_sub_text (sub_text);
	gtk_progress_bar_set_fraction (progressbar, (float)percent/100);
	g_free (main_text);
	g_free (sub_text);

	return;
}

void
gfpm_progress_event (unsigned char event, void *data1, void *data2)
{
	char *substr = NULL;
	while (gtk_events_pending ())
		gtk_main_iteration ();
	switch (event)
	{
		case PM_TRANS_EVT_CHECKDEPS_START: substr = g_strdup(_("Checking dependencies"));
										   break;
		case PM_TRANS_EVT_FILECONFLICTS_START: substr = g_strdup (_("Checking for file conflicts"));
											   break;
		case PM_TRANS_EVT_RESOLVEDEPS_START: substr = g_strdup (_("Resolving dependencies"));
											 break;
		case PM_TRANS_EVT_INTERCONFLICTS_START: substr = g_strdup (_("Looking for inter-conflicts"));
												break;
		case PM_TRANS_EVT_CHECKDEPS_DONE:
		case PM_TRANS_EVT_RESOLVEDEPS_DONE:
		case PM_TRANS_EVT_INTERCONFLICTS_DONE: substr = g_strdup (_("Done"));
												break;
		case PM_TRANS_EVT_ADD_START: substr = g_strdup_printf (_("installing %s"), (char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
									break;
		case PM_TRANS_EVT_ADD_DONE: substr = g_strdup_printf (_("installed %s"), (char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
									break;
		case PM_TRANS_EVT_UPGRADE_START: substr = g_strdup_printf (_("upgrading %s"), (char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
									break;
		case PM_TRANS_EVT_UPGRADE_DONE: substr = g_strdup_printf (_("upgraded %s from %s to %s"),
																(char*)pacman_pkg_getinfo(data1, PM_PKG_NAME),
																(char*)pacman_pkg_getinfo(data2, PM_PKG_VERSION),
																(char*)pacman_pkg_getinfo(data1, PM_PKG_VERSION));
									break;
		case PM_TRANS_EVT_REMOVE_START: substr = g_strdup (_("Removing package"));
										break;
		case PM_TRANS_EVT_REMOVE_DONE: substr = g_strdup (_("Done"));
										break;
		case PM_TRANS_EVT_INTEGRITY_START: substr = g_strdup (_("Checking package integrity"));
										   break;
		case PM_TRANS_EVT_INTEGRITY_DONE: substr = g_strdup (_("Done"));
										   break;
		case PM_TRANS_EVT_SCRIPTLET_START: substr = g_strdup ((char*)data1);
										   break;
		case PM_TRANS_EVT_SCRIPTLET_DONE: substr = g_strdup (_("Done"));
										   break;
		case PM_TRANS_EVT_RETRIEVE_START: substr = g_strdup_printf (_("Retrieving packages from %s"), (char*)data1);
										  break;
	}
	gfpm_progress_set_sub_text (substr);
	g_free (substr);

	return;
}

void
gfpm_progress_set_main_text (const char *msg)
{
	if (msg != NULL)
	{	
		gchar *markup = g_markup_printf_escaped ("<span size=\"large\" weight=\"bold\">%s</span>", msg);
		gtk_label_set_markup (GTK_LABEL(main_label), markup);
		g_free (markup);
	}

	return;
}

void
gfpm_progress_set_sub_text (const char *msg)
{
	if (msg != NULL)
		gtk_label_set_text (GTK_LABEL(sub_label), msg);

	return;
}

