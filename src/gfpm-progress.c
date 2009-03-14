/*
 *  gfpm-progress.c for gfpm
 *
 *  Copyright (C) 2006-2009 by Priyank Gosalia <priyankmg@gmail.com>
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
#include "gfpm.h"
#include "gfpm-progress.h"
#include "gfpm-interface.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

extern GtkWidget	*gfpm_mw;
extern gboolean		running;

GtkProgressBar		*progressbar = NULL;
GtkWidget		*progresswindow = NULL;
static GtkWidget	*main_label = NULL;
static GtkWidget	*sub_label = NULL;
static GtkWidget	*rate_label = NULL;
static GtkWidget	*rec_label = NULL;
static GtkWidget	*rate_box = NULL;
static GtkWidget	*progress_txtvw = NULL;
static GtkWidget	*details_scroll = NULL;
static GtkWidget	*button_close = NULL;
static GtkWidget	*autoclose_checkbtn = NULL;

GtkTextIter	t_iter;
GtkTextBuffer 	*buffer = NULL;

float	rate;
int	offset;
int	xferred1;
struct 	timeval	t0, t;
char 	reponame[PM_DLFNM_LEN+1];

/* package download status */
unsigned int remain;
unsigned int howmany;

gboolean cancelled = FALSE;

static void gfpm_progress_textview_reset (void);

/* callbacks */
static void cb_gfpm_close_button_clicked (GtkWidget *button, gpointer data);
static void cb_gfpm_details_button_toggled (GtkWidget *button, gpointer data);

void
gfpm_progress_init (void)
{
	pacman_parse_config ("/etc/pacman-g2.conf", NULL, "");
	pacman_set_option (PM_OPT_DLCB, (long)gfpm_progress_update);
	pacman_set_option (PM_OPT_DLOFFSET, (long)&offset);
	pacman_set_option (PM_OPT_DLRATE, (long)&rate);
	pacman_set_option (PM_OPT_DLFNM, (long)reponame);
	pacman_set_option (PM_OPT_DLHOWMANY, (long)&howmany);
	pacman_set_option (PM_OPT_DLREMAIN, (long)&remain);

	progressbar = GTK_PROGRESS_BAR(gfpm_get_widget ("progressbar1"));
	progresswindow = gfpm_get_widget ("progresswindow");
	main_label = gfpm_get_widget ("main_pr_label");
	sub_label = gfpm_get_widget ("sub_pr_label");
	rate_label = gfpm_get_widget ("rate_pr_label");
	rate_box = gfpm_get_widget ("rate_pr_box");
	rec_label = gfpm_get_widget ("rx_pr_label");
	progress_txtvw = gfpm_get_widget ("progress_txtvw");
	button_close = gfpm_get_widget ("close_progress");
	details_scroll = gfpm_get_widget ("details_scrollwindow");
	autoclose_checkbtn = gfpm_get_widget ("autoclose_checkbtn");
	g_signal_connect (G_OBJECT(gfpm_get_widget("show_details")),
					"toggled",
					G_CALLBACK(cb_gfpm_details_button_toggled),
					NULL);
	g_signal_connect (G_OBJECT(button_close),
					"clicked",
					G_CALLBACK(cb_gfpm_close_button_clicked),
					NULL);
	gtk_window_set_default_size (GTK_WINDOW(progresswindow), 350, 140);
	gtk_window_resize (GTK_WINDOW(progresswindow), 350, 140);
	gfpm_progress_textview_reset ();

	return;
}

static void
cb_gfpm_close_button_clicked (GtkWidget *button, gpointer data)
{
	if (!cancelled && running)
	{
		if (running)
		{
			pacman_trans_release ();
			cancelled = TRUE;
			gfpm_progress_show (FALSE);
		}
	}
	else
	{
		gfpm_progress_show (FALSE);
	}

	return;
}

static void
cb_gfpm_details_button_toggled (GtkWidget *button, gpointer data)
{
	static int width = 0;
	static int height = 0;

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	{
		gtk_widget_show (GTK_WIDGET(details_scroll));
		if (height>0 && width>0)
			gtk_window_resize (GTK_WINDOW(progresswindow), width, height);
		gtk_window_get_size (GTK_WINDOW(progresswindow), &width, &height);
		gtk_window_set_resizable (GTK_WINDOW(progresswindow), TRUE);
	}
	else
	{
		gint w;
		w = progresswindow->allocation.width;
		gtk_widget_hide (GTK_WIDGET(details_scroll));
		gtk_window_get_size (GTK_WINDOW(progresswindow), &width, &height);
		gtk_window_resize (GTK_WINDOW(progresswindow), 350, 1);
		gtk_window_set_resizable (GTK_WINDOW(progresswindow), FALSE);
	}
	return;
}

static void
gfpm_progress_textview_reset (void)
{
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(progress_txtvw));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &t_iter, 0);

	return;
}

gboolean
gfpm_progress_is_autoclose_checkbtn_set (void)
{
	return (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoclose_checkbtn)));
}

void
gfpm_progress_show (gboolean show)
{
	if (show == TRUE)
	{
		gtk_widget_hide (details_scroll);
		gtk_widget_show (progresswindow);
	}
	else
	{
		gtk_widget_hide (progresswindow);
		/* reset dialog attributes before showing */
		gfpm_progress_textview_reset ();
		gtk_label_set_text (GTK_LABEL(sub_label), "");
		gtk_label_set_text (GTK_LABEL(main_label), "");
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
	char		sub_text[PATH_MAX+1];
	struct timeval	t1;
	float 		tdiff;
	gchar		*rx_str = NULL;

	if (cancelled)
	{
		cancelled = FALSE;
		return 0;
	}

	while (gtk_events_pending())
		gtk_main_iteration ();
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
		sprintf (rate_text, "%6.1fK/s", (rate>0)?rate:0);
	}
	rx_str = g_strdup_printf ("%dK / %dK", (xferred+offset)/1024, size/1024);
	gtk_label_set_text (GTK_LABEL(rec_label), rx_str);

	gtk_progress_bar_set_text (progressbar, text);
	gtk_label_set_text (GTK_LABEL(rate_label), rate_text);
	gtk_progress_bar_set_fraction (progressbar, (float)per/100);

	if (remain && howmany)
	{
		snprintf (sub_text, PATH_MAX, "(%d/%d) %s", remain, howmany, g_strstrip(reponame));
	}
	else
	{
		snprintf (sub_text, PATH_MAX, g_strstrip(reponame));
	}
	gfpm_progress_set_sub_text (sub_text);

	while (gtk_events_pending())
		gtk_main_iteration ();

	return 1;
}

void
gfpm_progress_install (unsigned char event, char *pkgname, int percent, int count, int remaining)
{
	char *main_text = NULL;
	char *sub_text = NULL;

	if (!pkgname)
		return;
	if (percent < 0 || percent > 100)
		return;

	while (gtk_events_pending())
		gtk_main_iteration ();

	switch (event)
	{
		case PM_TRANS_PROGRESS_ADD_START:
			if (count > 1)
				main_text = g_strdup (_("Installing packages..."));
			else
				main_text = g_strdup (_("Installing package..."));
			break;
		case PM_TRANS_PROGRESS_UPGRADE_START:
			if (count > 1)
				main_text = g_strdup (_("Upgrading packages..."));
			else
				main_text = g_strdup (_("Upgrading package..."));
			break;
		case PM_TRANS_PROGRESS_REMOVE_START:
			if (count > 1)
				main_text = g_strdup (_("Removing packages..."));
			else
				main_text = g_strdup (_("Removing package..."));
			break;
		case PM_TRANS_PROGRESS_CONFLICTS_START:
			if (count > 1)
				main_text = g_strdup (_("Checking packages for file conflicts..."));
			else
				main_text = g_strdup (_("Checking package for file conflicts..."));
			break;
		default:
			return;
	}
	gfpm_progress_set_main_text (main_text, 0);
	if (count > 1)
	{
		/* TODO: Figure out a way to display "checking for file conflicts" status */
		if (pkgname && strlen(pkgname))
		{
			sub_text = g_strdup_printf ("(%d/%d) %s", remaining, count, pkgname);
			gfpm_progress_set_sub_text (sub_text);
			g_free (sub_text);
		}
	}
	/* set percentage */
	char text[6];
	snprintf (text, sizeof(text), "%d %%", percent);
	gtk_progress_bar_set_text (progressbar, text);
	gtk_progress_bar_set_fraction (progressbar, (float)percent/100);
	g_free (main_text);
	while (gtk_events_pending())
		gtk_main_iteration ();

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
	switch (event)
	{
		case PM_TRANS_EVT_CHECKDEPS_START:
			substr = g_strdup(_("Checking dependencies"));
			m = 1;
			break;
		case PM_TRANS_EVT_FILECONFLICTS_START:
			substr = g_strdup (_("Checking for file conflicts"));
			m = 1;
			break;
		case PM_TRANS_EVT_RESOLVEDEPS_START:
			substr = g_strdup (_("Resolving dependencies"));
			break;
		case PM_TRANS_EVT_INTERCONFLICTS_START:
			substr = g_strdup (_("Looking for inter-conflicts"));
			m = 1;
			break;
		case PM_TRANS_EVT_CHECKDEPS_DONE:
		case PM_TRANS_EVT_RESOLVEDEPS_DONE:
		case PM_TRANS_EVT_INTERCONFLICTS_DONE:
			substr = g_strdup (_("Done"));
			break;
		case PM_TRANS_EVT_ADD_START:
			substr = g_strdup_printf (_("Installing %s-%s"),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_NAME),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_VERSION));
			m = 1;
			break;
		case PM_TRANS_EVT_ADD_DONE:
			gfpm_progress_set_main_text (_("Package installation finished"), 0);
			substr = g_strdup_printf (_("Installed %s-%s"),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_NAME),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_VERSION));
			break;
		case PM_TRANS_EVT_UPGRADE_START:
			substr = g_strdup_printf (_("Upgrading %s"),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
			m = 1;
			break;
		case PM_TRANS_EVT_UPGRADE_DONE:
			gfpm_progress_set_main_text (_("Package upgrade finished"), 0);
			substr = g_strdup_printf (_("Upgraded %s from %s to %s"),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_NAME),
					(char*)pacman_pkg_getinfo(data2, PM_PKG_VERSION),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_VERSION));
			break;
		case PM_TRANS_EVT_REMOVE_START:
			substr = g_strdup_printf (_("removing %s"),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
			break;
		case PM_TRANS_EVT_REMOVE_DONE:
			gfpm_progress_set_main_text (_("Package removal finished"), 0);
			substr = g_strdup_printf (_("Removed %s"),
					(char*)pacman_pkg_getinfo(data1, PM_PKG_NAME));
			gtk_widget_set_sensitive (button_close, TRUE);
			break;
		case PM_TRANS_EVT_INTEGRITY_START:
			substr = g_strdup (_("Checking package integrity"));
			break;
		case PM_TRANS_EVT_INTEGRITY_DONE:
			substr = g_strdup (_("Done"));
			gtk_widget_set_sensitive (button_close, TRUE);
			break;
		case PM_TRANS_EVT_SCRIPTLET_INFO:
			substr = g_strdup ((char*)data1);
			break;
		case PM_TRANS_EVT_SCRIPTLET_START:
			substr = g_strdup ((char*)data1);
			break;
		case PM_TRANS_EVT_SCRIPTLET_DONE:
			substr = g_strdup (_("Done"));
			gtk_widget_set_sensitive (button_close, TRUE);
			break;
		case PM_TRANS_EVT_RETRIEVE_START:
			substr = g_strdup_printf (_("Retrieving packages from %s"), (char*)data1);
			m = 1;
			gtk_widget_show (rate_box);
			gtk_widget_show (rec_label);
			break;
		default:
			return;
	}
	if (m == 1)
	{
		gfpm_progress_set_main_text (substr, 1);
		m = 0;
	}
	else
	{
		gfpm_progress_set_sub_text (substr);
	}
	g_free (substr);
/*	
	if (gtk_events_pending())
	{
		while (gtk_events_pending ())
			gtk_main_iteration ();
	}
	else
	{
		g_print ("boogaing\n");
		usleep (5000);
	}
*/
	return;
}


void
gfpm_progress_set_main_text (const char *msg, int txt)
{
	gchar *str = NULL;
	gchar *nstr = NULL;
	static char *prev_msg = NULL;

	if (msg != NULL)
	{
		str = g_strdup (msg);
		g_strstrip (str);
		gchar *markup = g_markup_printf_escaped ("<span size=\"large\" weight=\"bold\">%s</span>", str);
		nstr = g_strdup_printf ("%s\n", str);
		g_free (str);
		gtk_label_set_markup (GTK_LABEL(main_label), markup);
		g_free (markup);
	
		if (txt)
		{
			if (prev_msg != NULL)
			{
				if (!strcmp(prev_msg, msg))
					return;
			}
			gtk_text_buffer_insert (buffer, &t_iter, nstr, strlen(nstr));
			g_free (prev_msg);
			prev_msg = g_strdup_printf (msg);
			g_free (nstr);
		}
	}

	return;
}


void
gfpm_progress_set_sub_text (const char *msg)
{
	gchar *str = NULL;
	gchar *nstr = NULL;
	static char *sub_prev_msg = NULL;

	if (msg != NULL)
	{
		if (sub_prev_msg != NULL)
		{
			if (!strcmp(sub_prev_msg, msg))
				return;
		}

		str = g_strdup (msg);
		g_strstrip (str);
		gtk_label_set_text (GTK_LABEL(sub_label), str);
		nstr = g_strdup_printf ("%s\n", str);
		g_free (str);
		gtk_text_buffer_insert (buffer, &t_iter, nstr, strlen(nstr));
		g_free (nstr);
		g_free (sub_prev_msg);
		sub_prev_msg = g_strdup_printf (msg);
	}
	
	return;
}

