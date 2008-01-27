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
#include <glib.h>

typedef struct _LogViewItem
{
	gchar		*label;
	GList		*children;
} LogViewItem;

#define LOG_FILE "/var/log/pacman-g2.log"

extern GladeXML *xml;

int getdate_err;

/* Log viewer widgets */
static GtkWidget *gfpm_logviewer_dlg;
static GtkWidget *gfpm_logviewer_tvw;
static GtkWidget *gfpm_logviewer_txtvw;

static void _gfpm_logviewer_populate (void);

void
gfpm_logviewer_init (void)
{
	 gint col_offset;
	 GtkCellRenderer *renderer;
	 GtkTreeViewColumn *column;
	 
	 gfpm_logviewer_dlg = glade_xml_get_widget (xml, "syslog_window");
	 gfpm_logviewer_tvw = glade_xml_get_widget (xml, "log_tvw");
	 gfpm_logviewer_txtvw = glade_xml_get_widget (xml, "log_txtvw");
	 
	 renderer = gtk_cell_renderer_text_new ();
	 g_object_set (renderer, "xalign", 0.0, NULL);
	 col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (gfpm_logviewer_tvw),
							    -1, "Month / Year",
							    renderer, "text",
							    0,
							    NULL);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW (gfpm_logviewer_tvw), col_offset - 1);
	gtk_tree_view_column_set_clickable (GTK_TREE_VIEW_COLUMN (column), TRUE);

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
	FILE			*fp = NULL;
	char			line[PATH_MAX+1] = "";
	int				prev_day = -1;
	int				prev_month = -1;
	int				prev_year = -1;
	GtkTreeStore 	*store;
	GtkTreeIter		iter;
	GList			*master = NULL;
	GList			*child = NULL;
	LogViewItem 	*li = NULL;

	if ((fp=fopen(LOG_FILE,"r"))==NULL)
	{
		gfpm_error (_("Error"), _("Error opening log file."));
		return;
	}
	while (fgets(line,PATH_MAX,fp))
	{
		char *ptr = NULL;

		fwutil_trim (line);
		if (!strlen(line))
			continue;
		if (line[0] == '[' && line[15] == ']')
		{
			int i;
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
				printf ("ERROR: getdate() failed with error code: %d\n", getdate_err);
			}
		}
	}
	fclose (fp);

	/* add the master list */
	store = gtk_tree_store_new (1, G_TYPE_STRING);
	
	while (master != NULL)
	{
		LogViewItem *m = master->data;
		//printf ("SECTION : %s\n", m->label);
		gtk_tree_store_append (store, &iter, NULL);
		gtk_tree_store_set (store, &iter, 0, m->label, -1);
		/* add children */
		GList *child = m->children;
		while (child != NULL)
		{
			GtkTreeIter 	child_iter;
			//printf ("\tSUB: %s\n", child->data);
			gtk_tree_store_append (store, &child_iter, &iter);
			gtk_tree_store_set (store, &child_iter, 0, child->data, -1);
			child = g_list_next (child);
		}
		master = g_list_next (master);
	}
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_logviewer_tvw), GTK_TREE_MODEL(store));
	
	return;
}

