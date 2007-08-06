/*
 *  gfpm-progress.c for gfpm
 *
 *  Copyright (C) 2006-2007 by Priyank Gosalia <priyankmg@gmail.com>
 *  Portions of this code Copyright (C) 2002-2006 Judd Vinet <jvinet@zeroflux.org>
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
#include <time.h>
#include <sys/time.h>
#include <glade/glade.h>
#include "gfpm.h"
#include "gfpm-progress.h"
#include "gfpm-systray.h"

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

extern GladeXML		*xml;
extern GtkWidget	*gfpm_mw;

GtkProgressBar		*progressbar = NULL;
GtkWidget		*progresswindow = NULL;
static GtkWidget	*main_label = NULL;
static GtkWidget	*sub_label = NULL;
static GtkWidget	*rate_label = NULL;
static GtkWidget	*rec_label = NULL;
static GtkWidget	*rate_box = NULL;

float		rate;
int		offset;
int		xferred1;
struct timeval	t0, t;
char 		reponame[PM_DLFNM_LEN+1];

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
	rate_label = glade_xml_get_widget (xml, "rate_pr_label");
	rate_box = glade_xml_get_widget (xml, "rate_pr_box");
	rec_label = glade_xml_get_widget (xml, "rx_pr_label");

	return;
}

void
gfpm_progress_show (gboolean show, gint sync)
{
	if (show == TRUE)
	{	
		gtk_widget_show (progresswindow);
		if (!sync)
		{
			gtk_widget_hide (gfpm_mw);
			gfpm_systray_set_visible (TRUE);
		}
	}
	else
	{	
		gtk_widget_hide (progresswindow);
		if (!sync)
		{
			gtk_widget_show (gfpm_mw);
			gfpm_systray_set_visible (FALSE);
		}
	}

	return;
}

int
gfpm_progress_update (netbuf *ctl, int xferred, void *arg)
{
	int		size;
	int		per;
	char		text[6];
	char		rate_text[10];
	struct timeval 	t1;
	float 		tdiff;
	gchar		*rx_str = NULL;

	ctl = NULL;
	size = *(int*)arg;
	per = ((float)(xferred+offset) / size) * 100;
	sprintf (text, "%d %%", per);
	gettimeofday (&t1, NULL);
	if (xferred+offset == size)
		t = t0;
	tdiff = t1.tv_sec-t.tv_sec + (float)(t1.tv_usec-t.tv_usec) / 1000000; 
	if (xferred+offset == size)
	{
		rate = xferred / (tdiff * 1024);
	}
	else if (tdiff > 1)
	{
		rate = (xferred - xferred1) / (tdiff * 1024);
		xferred1 = xferred;
		gettimeofday (&t, NULL);
	}
	if (rate > 1000)
	{
		sprintf (rate_text, "%6.0fK/s", rate);
	}
	else
	{
		sprintf (rate_text, "%6.1fK/s", rate);
	}
	rx_str = g_strdup_printf ("%dK / %dK", (xferred+offset)/1024, size/1024);
	gtk_label_set_text (GTK_LABEL(rec_label), rx_str);
		
	while (gtk_events_pending ())
		gtk_main_iteration ();
	gtk_progress_bar_set_text (progressbar, text);
	gtk_label_set_text (GTK_LABEL(rate_label), rate_text);
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
	if (percent < 0 || percent > 100)
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
		default: return;
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
	int m = 0;

	if (data1 == NULL)
		return;
	gtk_widget_hide (rate_box);
	gtk_widget_hide (rec_label);
	while (gtk_events_pending ())
		gtk_main_iteration ();
	switch (event)
	{
		case PM_TRANS_EVT_CHECKDEPS_START:	substr = g_strdup(_("Checking dependencies"));
							break;
		case PM_TRANS_EVT_FILECONFLICTS_START:	substr = g_strdup (_("Checking for file conflicts"));
							break;
		case PM_TRANS_EVT_RESOLVEDEPS_START:	substr = g_strdup (_("Resolving dependencies"));
							break;
		case PM_TRANS_EVT_INTERCONFLICTS_START: substr = g_strdup (_("Looking for inter-conflicts"));
							break;
		case PM_TRANS_EVT_CHECKDEPS_DONE:
		case PM_TRANS_EVT_RESOLVEDEPS_DONE:
		case PM_TRANS_EVT_INTERCONFLICTS_DONE:	substr = g_strdup (_("Done"));
							break;
		case PM_TRANS_EVT_ADD_START:		substr = g_strdup_printf (_("installing %s"), (char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
							break;
		case PM_TRANS_EVT_ADD_DONE:		substr = g_strdup_printf (_("installed %s"), (char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
							break;
		case PM_TRANS_EVT_UPGRADE_START:	substr = g_strdup_printf (_("upgrading %s"), (char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
							break;
		case PM_TRANS_EVT_UPGRADE_DONE:		substr = g_strdup_printf (_("upgraded %s from %s to %s"),
										(char*)pacman_pkg_getinfo(data1, PM_PKG_NAME),
										(char*)pacman_pkg_getinfo(data2, PM_PKG_VERSION),
										(char*)pacman_pkg_getinfo(data1, PM_PKG_VERSION));
							break;
		case PM_TRANS_EVT_REMOVE_START:		substr = g_strdup (_("Removing package"));
							break;
		case PM_TRANS_EVT_REMOVE_DONE:		substr = g_strdup (_("Done"));
							break;
		case PM_TRANS_EVT_INTEGRITY_START:	substr = g_strdup (_("Checking package integrity"));
							break;
		case PM_TRANS_EVT_INTEGRITY_DONE:	substr = g_strdup (_("Done"));
							break;
		case PM_TRANS_EVT_SCRIPTLET_INFO:	substr = g_strdup ((char*)data1);
							break;
		case PM_TRANS_EVT_SCRIPTLET_START:	substr = g_strdup ((char*)data1);
							break;
		case PM_TRANS_EVT_SCRIPTLET_DONE:	substr = g_strdup (_("Done"));
							break;
		case PM_TRANS_EVT_RETRIEVE_START:	substr = g_strdup_printf (_("Retrieving packages from %s"), (char*)data1);
							m = 1;
							gtk_widget_show (rate_box);
							gtk_widget_show (rec_label);
							break;
		default:				return;
	}
	if (m == 1)
	{	
		gfpm_progress_set_main_text (substr);
		m = 0;
	}
	else
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

