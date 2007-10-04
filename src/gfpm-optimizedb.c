/*
 *  gfpm-optimizedb.c for gfpm
 *
 *  Copyright (C) 2006-2007 by Priyank Gosalia <priyankmg@gmail.com>
 *  Based on pacman-optimize written by Judd Vinet
 *  Copyright (C) 2002-2006 Judd Vinet <jvinet@zeroflux.org>
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

#include "gfpm-optimizedb.h"
#include "gfpm-messages.h"

#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>

#define DBLOC "/var/lib/pacman"
#define TMPLOC "/var/lib/pacman.new"
#define OLDSUM "/tmp/dbsums.old"
#define NEWSUM "/tmp/dbsums.new"
#define LOCKFILE "/tmp/pacman-g2.lck"

extern GtkBuilder *xml;

static GtkWidget *gfpm_optimize_db_dlg;
static GtkWidget *gfpm_optimize_db_progressbar;
static GtkWidget *gfpm_optimize_db_startbtn;
static GtkWidget *gfpm_optimize_db_closebtn;
static GtkWidget *gfpm_optimize_db_label;
static void gfpm_optimize_db_set_progress_status (const char *text);
static void gfpm_optimize_db_optimize_btn_clicked (GtkWidget *widget, gpointer data);
static void gfpm_optimize_db_dlg_show (void);
static void gfpm_optimize_db_dlg_hide (void);
static void gfpm_optimize_db (void);

void
gfpm_optimize_db_dlg_init (void)
{
	gfpm_optimize_db_dlg = GTK_WIDGET (gtk_builder_get_object (xml, "optimize_db_dlg"));
	gfpm_optimize_db_progressbar = GTK_WIDGET (gtk_builder_get_object (xml, "optimize_db_progress"));
	gfpm_optimize_db_startbtn = GTK_WIDGET (gtk_builder_get_object (xml, "optimize_db_start"));
	gfpm_optimize_db_closebtn = GTK_WIDGET (gtk_builder_get_object (xml, "optimize_db_close"));
	gfpm_optimize_db_label = GTK_WIDGET (gtk_builder_get_object (xml, "optimize_db_label"));

	g_signal_connect (G_OBJECT(gfpm_optimize_db_startbtn),
			"clicked",
			G_CALLBACK(gfpm_optimize_db_optimize_btn_clicked),
			NULL);
	g_signal_connect (G_OBJECT(gfpm_optimize_db_closebtn),
			"clicked",
			G_CALLBACK(gfpm_optimize_db_dlg_hide),
			NULL);
	g_signal_connect (gtk_builder_get_object(xml,"optimize"),
			"activate",
			G_CALLBACK(gfpm_optimize_db_dlg_show),
			NULL);

	return;
}

static void
gfpm_optimize_db_set_progress_status (const char *text)
{
	gtk_label_set_text (GTK_LABEL(gfpm_optimize_db_label), text);

	return;
}

static void
gfpm_optimize_db_optimize_btn_clicked (GtkWidget *widget, gpointer data)
{
	gfpm_optimize_db ();

	return;
}

static void
gfpm_optimize_db_dlg_show (void)
{
	gfpm_optimize_db_set_progress_status (_("Ready"));
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_optimize_db_progressbar), 0.0);
	gtk_widget_show (gfpm_optimize_db_dlg);

	return;
}

static void
gfpm_optimize_db_dlg_hide (void)
{
	gtk_widget_hide (gfpm_optimize_db_dlg);

	return;
}

void
gfpm_optimize_db (void)
{
	char *cmdline = NULL;
	char temp[40];

	/* disable the start button */
	gtk_widget_set_sensitive (gfpm_optimize_db_startbtn, FALSE);
	gtk_widget_set_sensitive (gfpm_optimize_db_closebtn, FALSE);

	/* check if another package manager is already running */
	if (g_file_test(LOCKFILE,G_FILE_TEST_EXISTS))
	{
		gfpm_error (_("Error"), _("Cannot optimize database. Another instance of pacman-g2 or gfpm is already running."));
		goto cleanup;
	}

	/* create a lock file */
	if (g_creat(LOCKFILE,S_IRWXU)==-1)
	{
		gfpm_error (_("Error"), _("Error creating lockfile. Failed to optimize package database"));
		goto cleanup;
	}

	/* generate checksums for the old database */
	gfpm_optimize_db_set_progress_status (_("generating checksums for the old database"));
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_optimize_db_progressbar), 0.1);
	while (gtk_events_pending()) gtk_main_iteration ();
	sleep (2);
	cmdline = g_strdup_printf ("find %s -type f | sort | xargs md5sum > %s", DBLOC, OLDSUM);
	system (cmdline);
	g_free (cmdline);

	/* copy the old database to a new directory */
	sprintf (temp, _("copying %s"), DBLOC);
	gfpm_optimize_db_set_progress_status (temp);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_optimize_db_progressbar), 0.25);
	while (gtk_events_pending()) gtk_main_iteration ();
	sleep (2);
	cmdline = g_strdup_printf ("cp -a %s %s", DBLOC, TMPLOC);
	system (cmdline);
	g_free (cmdline);
	
	/* generate checksums for the new database */
	gfpm_optimize_db_set_progress_status (_("generating checksums for the new database"));
	cmdline = g_strdup_printf ("mv %s %s.bak", DBLOC, DBLOC);
	system (cmdline);
	g_free (cmdline);
	cmdline = g_strdup_printf ("mv %s %s", TMPLOC, DBLOC);
	system (cmdline);
	g_free (cmdline);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_optimize_db_progressbar), 0.4);
	while (gtk_events_pending()) gtk_main_iteration ();
	cmdline = g_strdup_printf ("find %s -type f | sort | xargs md5sum > %s", DBLOC, NEWSUM);
	system (cmdline);
	g_free (cmdline);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_optimize_db_progressbar), 0.6);
	while (gtk_events_pending()) gtk_main_iteration ();
	sleep (2);

	/* check integrity */
	gfpm_optimize_db_set_progress_status (_("performing integrity check"));
	cmdline = g_strdup_printf ("diff %s %s > /dev/null 2>&1", OLDSUM, NEWSUM);
	if (system(cmdline)!=0)
	{
		gfpm_error (_("Optimization failed"), _("Integrity check failed"));
		gfpm_optimize_db_set_progress_status (_("Failed."));
		g_free (cmdline);
		cmdline = g_strdup_printf ("rm -rf %s;mv %s.bak %s", DBLOC, DBLOC, DBLOC);
		system (cmdline);
		g_free (cmdline);
		goto cleanup;
	}
	g_free (cmdline);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_optimize_db_progressbar), 0.85);
	while (gtk_events_pending()) gtk_main_iteration ();

	/* remove old database */
	gfpm_optimize_db_set_progress_status (_("removing old database"));
	cmdline = g_strdup_printf ("rm -rf %s.bak", DBLOC);
	system (cmdline);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_optimize_db_progressbar), 0.95);
	while (gtk_events_pending()) gtk_main_iteration ();
	sleep (2);

	/* remove temp files */
	gfpm_optimize_db_set_progress_status (_("removing temporary files"));
	g_remove (OLDSUM);
	g_remove (NEWSUM);
	g_remove (LOCKFILE);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_optimize_db_progressbar), 1.0);
	while (gtk_events_pending()) gtk_main_iteration ();
	sleep (2);
	gfpm_optimize_db_set_progress_status (_("Database optimized."));
	gfpm_message (_("Database optimized"), _("Your package database is now optimized."));

cleanup:
	gtk_widget_set_sensitive (gfpm_optimize_db_startbtn, TRUE);
	gtk_widget_set_sensitive (gfpm_optimize_db_closebtn, TRUE);

	return;
}

