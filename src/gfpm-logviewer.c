/*
 *  gfpm-logviewer.c for gfpm
 *
 *  Copyright (C) 2008-2009 by Priyank Gosalia <priyankmg@gmail.com>
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



#include "gfpm-logviewer.h"
#include "gfpm-messages.h"
#include "gfpm-interface.h"
#include <libfwutil.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>

/* required for getdate() */
#include <time.h>

extern GtkBuilder *gb;

typedef struct _LogViewItem
{
	gchar	*label;
	GList	*children;
} LogViewItem;

/* location of datemsk file */
#define DMK_FILE "/share/gfpm/datemsk"

int getdate_err;
struct tm *getdate(const char *string);

/* Log viewer widgets */
static GtkWidget *gfpm_logviewer_dlg;
static GtkWidget *gfpm_logviewer_tvw;
static GtkWidget *gfpm_logviewer_txtvw;
static GtkWidget *gfpm_logviewer_sizelabel;
static GtkWidget *gfpm_logviewer_progressbar;

static void _gfpm_logviewer_populate (void);
static void _gfpm_logviewer_populate_txtvw (const char *text);

static void cb_gfpm_logviewer_tvw_row_activated (GtkTreeSelection *selection, gpointer data);

void
gfpm_logviewer_init (void)
{
	 gint 			col_offset;
	 GtkCellRenderer	*renderer = NULL;
	 GtkTreeViewColumn	*column = NULL;
	 GtkTreeSelection	*sel = NULL;

	 if (getenv("DATEMSK") == NULL)
	 {
	 	gchar *loc = g_strdup_printf ("%s/%s", PREFIX, DMK_FILE);
	 	setenv ("DATEMSK", loc, 0);
	 	g_free (loc);
	 }
	 gfpm_logviewer_dlg = gfpm_get_widget ("syslog_window");
	 gfpm_logviewer_tvw = gfpm_get_widget ("log_tvw");
	 gfpm_logviewer_txtvw = gfpm_get_widget ("log_txtvw");
	 gfpm_logviewer_sizelabel = gfpm_get_widget ("log_size_label");
	 gfpm_logviewer_progressbar = gfpm_get_widget ("log_progress");

	 renderer = gtk_cell_renderer_text_new ();
	 g_object_set (renderer, "xalign", 0.0, NULL);
	 col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (gfpm_logviewer_tvw),
							    -1, "Month / Year",
							    renderer, "text",
							    0,
							    NULL);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (gfpm_logviewer_tvw), col_offset - 1);
	gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);
	sel = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_logviewer_tvw));
	g_signal_connect (G_OBJECT(sel), "changed", G_CALLBACK(cb_gfpm_logviewer_tvw_row_activated), NULL);

	/* hide window on delete-event */
	g_signal_connect (G_OBJECT(gfpm_logviewer_dlg),
			"delete-event",
			G_CALLBACK(gtk_widget_hide),
			(gpointer)gfpm_logviewer_dlg);
	return;
}

void
gfpm_logviewer_show ()
{
	gtk_widget_show (gfpm_logviewer_dlg);
	_gfpm_logviewer_populate ();

	return;
}

static void
_gfpm_logviewer_populate (void)
{
	FILE		*fp = NULL;
	char		line[PATH_MAX+1] = "";
	int		prev_day = -1;
	int		prev_month = -1;
	int		prev_year = -1;
	GtkTreeStore 	*store;
	GtkTreeIter	iter;
	GList		*master = NULL;
	LogViewItem 	*li = NULL;
	struct stat	fstat;
	char		*command = NULL;
	unsigned int	lines = 0;

	if ((fp=fopen(LOG_FILE,"r"))==NULL)
	{
		gfpm_error (_("Error"), _("Error opening log file."));
		return;
	}
	/* calculate the size of the log file in KB */
	if (!g_stat(LOG_FILE,&fstat))
	{
		gint size = 0;
		gchar *sizetxt = NULL;
		size = fstat.st_size / 1024;
		sizetxt = g_strdup_printf ("%d KB", size);
		gtk_label_set_text (GTK_LABEL(gfpm_logviewer_sizelabel), sizetxt);
		g_free (sizetxt);
	}

	/* display a nice progress bar for huge log files */
	/* for that we need the number of lines in the log file */
	command = g_strdup_printf ("wc -l %s | cut -d ' ' -f1", LOG_FILE);
	FILE *ft = popen (command, "r");
	if (ft)
	{
		char ln[PATH_MAX+1] = "";
		fgets (ln, PATH_MAX, ft);
		lines = atoi (ln);
	}
	g_free (command);
	g_print ("%s contains %d lines\n", LOG_FILE, lines);

	unsigned int 	iter_lines = 0;
	float		progress = 0;
	float		prev_progress = 0;
	while (fgets(line,PATH_MAX,fp))
	{
		char *ptr = NULL;

		fwutil_trim (line);
		if (!strlen(line))
			continue;

		/* update progress */
		progress = (float) iter_lines / lines;

		if ((progress-prev_progress)>=0.01)
		{
			//g_print ("progress: %0.2f  prev_progress: %0.2f \n", progress, prev_progress);
			gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(gfpm_logviewer_progressbar), progress);
			prev_progress = progress;
		}
		iter_lines++;

		if (line[0] == '[' && line[15] == ']')
		{
			struct tm *t;

			ptr = line;
			ptr++;
			ptr[14] = 0;
			t = getdate (ptr);
			if (t!=NULL)
			{
				if (prev_year != (t->tm_year+1900))
				{
					prev_year = (t->tm_year+1900);
					if (prev_month != (t->tm_mon+1))
					{
						char tday[64] = "";
						char *day = NULL;
						strftime (tday, 64, "%B %Y", t);
						/* convert to utf-8 */
						day = g_convert (tday, strlen(tday), "UTF-8", "", NULL, NULL, NULL);
						prev_month = t->tm_mon+1;
						li = (LogViewItem*) malloc(sizeof(LogViewItem));

						prev_month = t->tm_mon+1;
						li->label = g_strdup (day);
						li->children = NULL;
						master = g_list_append (master, (gpointer)li);
					}
				}
				if (prev_month != (t->tm_mon+1))
				{
					char tday[64] = "";
					char *day = NULL;
					strftime (tday, 64, "%B %Y", t);
					/* convert to utf-8 */
					day = g_convert (tday, strlen(tday), "UTF-8", "", NULL, NULL, NULL);
					li = (LogViewItem*) malloc(sizeof(LogViewItem));
					prev_month = t->tm_mon+1;
					li->label = g_strdup (day);
					li->children = NULL;
					master = g_list_append (master, (gpointer)li);
				}
				if (prev_day != t->tm_mday)
				{
					char	day[64] = "";
					strftime (day, 64, "%a %d %b %Y", t);
					li->children = g_list_append (li->children, (gpointer)(g_strdup(day)));
					prev_day = t->tm_mday;
				}
				while (gtk_events_pending())
					gtk_main_iteration ();
			}
			else
			{
				if (getdate_err == 1) {
                                        gfpm_error (_("Error"), _("Missing datemsk file. Make sure that the file exists on your system"));
                                        goto cleanup;
                                } else if (getdate_err == 2) {
                                        gfpm_error (_("Error"), _("The template file cannot be opened for reading."));
                                        goto cleanup;
                                } else if (getdate_err == 3) {
                                        gfpm_error (_("Error"), _("Failed to get file status information."));
                                        goto cleanup;
                                } else if (getdate_err == 4) {
                                        gfpm_error (_("Error"), _("The template file is not a regular file."));
                                        goto cleanup;
                                } else if (getdate_err == 5) {
                                        gfpm_error (_("Error"), _("An I/O error is encountered while reading the template file."));
                                        goto cleanup;
                                } else if (getdate_err == 6) {
                                        gfpm_error (_("Error"), _("Memory allocation failed (not enough memory available)."));
                                        goto cleanup;
                                } else if (getdate_err == 7) {
                                        gfpm_error (_("Error"), _("There is no line in the template that matches the input."));
                                        goto cleanup;
                                } else if (getdate_err == 8) {
                                        gfpm_error (_("Error"), _("Invalid input specification."));
                                        goto cleanup;
                                } else {
                                        gfpm_error (_("Unknown Error"), _("Getdate: unknown error."));
                                        goto cleanup;
				}

			}
		}
	}
	/* hide the progress bar */
	gtk_widget_hide (gfpm_logviewer_progressbar);

	/* add the master list */
	store = gtk_tree_store_new (1, G_TYPE_STRING);

	while (master != NULL)
	{
		LogViewItem *m = master->data;
		gtk_tree_store_append (store, &iter, NULL);
		gtk_tree_store_set (store, &iter, 0, m->label, -1);
		/* add children */
		GList *child = m->children;
		while (child != NULL)
		{
			GtkTreeIter 	child_iter;
			gtk_tree_store_append (store, &child_iter, &iter);
			gtk_tree_store_set (store, &child_iter, 0, child->data, -1);
			child = g_list_next (child);
		}
		master = g_list_next (master);
	}
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_logviewer_tvw), GTK_TREE_MODEL(store));

	cleanup: fclose (fp);
	return;
}

static void
_gfpm_logviewer_populate_txtvw (const char *text)
{
	struct tm 	*t = NULL;
	char 		date[10] = "";
	GtkTextBuffer	*buffer;
	GtkTextIter	iter;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(gfpm_logviewer_txtvw));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
	t = getdate (text);
	if (t != NULL)
	{
		FILE *fp = fopen (LOG_FILE, "r");
		char line[PATH_MAX+1] = "";
		if (!fp)
		{
			gfpm_error (_("Error"), _("Error opening log file"));
			return;
		}
		strftime (date, 10, "[%m/%d/%y", t);
		//printf ("Searching for: %s\n", date);
		while (fgets(line, PATH_MAX, fp))
		{
			gchar *pot = g_strstr_len (line, 10, date);
			if (pot != NULL)
				gtk_text_buffer_insert (buffer, &iter, pot, -1);
		}
		gtk_text_view_set_buffer (GTK_TEXT_VIEW(gfpm_logviewer_txtvw), buffer);
		fclose (fp);
	}

	return;
}

static void
cb_gfpm_logviewer_tvw_row_activated (GtkTreeSelection *selection, gpointer data)
{
	GtkTreeIter	iter;
	GtkTreeIter	piter;
	GtkTreeModel	*model;

	if (gtk_tree_selection_get_selected (selection, &model, &iter))
	{
		gchar *text = NULL;

		if (gtk_tree_model_iter_parent (model, &piter, &iter))
		{
			gtk_tree_model_get (model, &iter, 0, &text, -1);
			_gfpm_logviewer_populate_txtvw (text);
			g_free (text);
		}
	}

	return;
}

