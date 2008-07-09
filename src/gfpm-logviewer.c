/*
 *  gfpm-logviewer.c for gfpm
 *
 *  Copyright (C) 2008 by Priyank Gosalia <priyankmg@gmail.com>
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
#include <time.h>

typedef struct _LogViewItem
{
	gchar	*label;
	GList	*children;
} LogViewItem;

/* location of datemsk file */
#define DMK_FILE "/share/gfpm/datemsk"

int getdate_err;

/* Log viewer widgets */
static GtkWidget *gfpm_logviewer_dlg;
static GtkWidget *gfpm_logviewer_tvw;
static GtkWidget *gfpm_logviewer_txtvw;
static GtkWidget *gfpm_logviewer_sizelabel;

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
	while (fgets(line,PATH_MAX,fp))
	{
		char *ptr = NULL;

		fwutil_trim (line);
		if (!strlen(line))
			continue;
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
						char day[64] = "";
						strftime (day, 64, "%B %Y", t);
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
					char day[64] = "";
					strftime (day, 64, "%B %Y", t);
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
				if (getdate_err == 1)
					gfpm_error (_("Error"), _("Missing datemsk file. Make sure that the file exists on your system"));
				else
					gfpm_error (_("Unknown Error"), _("Unknown Error."));
				goto cleanup;
			}
		}
	}

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

