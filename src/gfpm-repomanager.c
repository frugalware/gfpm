/*
 *  gfpm-repomanager.c for gfpm
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

#include "gfpm-repomanager.h"
#include "gfpm-messages.h"
#include <glib.h>

#define CONF_FILE "/etc/pacman-g2.conf"

extern GladeXML *xml;

static gfpm_repolist_t *repolist = NULL;

static GtkWidget *gfpm_repomgr_dlg;
static GtkWidget *gfpm_repomgr_treeview;
static GtkWidget *gfpm_repomgr_btnadd;
static GtkWidget *gfpm_repomgr_btndel;
static GtkWidget *gfpm_repomgr_btnmup;
static GtkWidget *gfpm_repomgr_btnmdn;
static GtkWidget *gfpm_repomgr_btnedit;

void
gfpm_repomanager_init (void)
{
	GtkListStore		*store = NULL;
	GtkCellRenderer		*renderer = NULL;
	GtkTreeViewColumn	*column = NULL;

	gfpm_repomgr_dlg = glade_xml_get_widget (xml, "gfpm_repomanager");
	gfpm_repomgr_treeview = glade_xml_get_widget (xml, "repoman_listview");
	gfpm_repomgr_btnadd = glade_xml_get_widget (xml, "repoman_add");
	gfpm_repomgr_btndel = glade_xml_get_widget (xml, "repoman_del");
	gfpm_repomgr_btnedit = glade_xml_get_widget (xml, "repoman_edit");
	gfpm_repomgr_btnmup = glade_xml_get_widget (xml, "repoman_mup");
	gfpm_repomgr_btnmdn = glade_xml_get_widget (xml, "repoman_mdn");

	/* setup repo store */
	store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes (_("S"),
														renderer,
														"pixbuf", 0,
														NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_repomgr_treeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Repository"),
														renderer,
														"text", 1,
														NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_repomgr_treeview), column);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_repomgr_treeview), GTK_TREE_MODEL(store));

	return;
}

static GList *
gfpm_repomgr_get_servers_from_repofile (const char *conf_file)
{
	FILE	*fp = NULL;
	GList	*ret = NULL;
	char	line[PATH_MAX+1];
	char	str[PATH_MAX+1];

	fp = fopen (conf_file, "r");
	if (fp == NULL)
	{
		g_error ("No configuration file found");
		return NULL;
	}

	while (fgets(line, PATH_MAX, fp))
	{
		fwutil_trim(line);
		if (sscanf(line, "Server = %s", str))
		{
			ret = g_list_append (ret, (gpointer)g_strdup(fwutil_trim(str)));
		}
	}
	fclose (fp);

	return ret;
}

static void
gfpm_repomgr_populate_repolist (void)
{
	char *ptr = NULL;
	FILE *fp = NULL;
	char line[PATH_MAX+1];
	char str[256];
	gint n = 0;

	fp = fopen (CONF_FILE, "r");
	if (fp == NULL)
	{
		g_error ("No configuration file found");
		return NULL;
	}
	if (repolist!=NULL)
		g_free (repolist);

	repolist = (gfpm_repolist_t*)malloc(sizeof(gfpm_repolist_t));
	if (repolist == NULL)
	{
		g_error ("Error allocating memory\n");
		return NULL;
	}
	memset (repolist, 0, sizeof(gfpm_repolist_t));
	while (fgets(line, PATH_MAX, fp))
	{
		char reponame[256] = "";
		fwutil_trim (line);
		if (!strlen(line) || line[0] == '#')
			continue;
		if (line[0] == '[' && line[strlen(line)-1] == ']')
		{
			// could be a repo entry
			ptr = line;
			ptr++;
			strncpy (reponame, ptr, fwutil_min(255, strlen(ptr)-1));
			reponame[fwutil_min(255, strlen(ptr-1))] = '\0';
			if (!strlen(reponame))
			{
				g_error ("Bad repository name. Skipping.");
				continue;
			}
			if (!strcmp(reponame, "options"))
				continue;
			else
			{
				// create a new repo record
				n++;
				gfpm_repo_t *repo_r = (gfpm_repo_t*)malloc(sizeof(gfpm_repo_t));
				if (repo_r == NULL)
				{
					g_error ("Error allocation memory. Exiting");
					return;
				}
				memset (repo_r, 0, sizeof(gfpm_repo_t));
				strncpy (repo_r->name, reponame, REPONAME_MAX_SIZE);
				// get the server url
				fgets (line, PATH_MAX, fp);
				sscanf (line, "Server = %s", str);
				repo_r->servers = g_list_append (repo_r->servers, (gpointer)g_strdup(str));
				// and then append it to our repo list
				repolist->list = g_list_append (repolist->list, (gpointer)repo_r);
			}
		}
		else if (sscanf(line, "Include = %s", str))
		{
			FILE *tmp = NULL;
			char ln[PATH_MAX+1];
			char rn[PATH_MAX+1];
			gfpm_repo_t *repo_r = (gfpm_repo_t*)malloc(sizeof(gfpm_repo_t));
			
			if ((tmp = fopen (str, "r")) == NULL)
			{
				g_error ("Error opening repository servers file.");
				break;
			}
			while (fgets(ln, PATH_MAX, tmp))
			{
				fwutil_trim (ln);
				if (!strlen(ln) || ln[0] == '#')
					continue;
				if (ln[0] == '[' && ln[strlen(ln)-1] == ']')
				{
					// could be a repo entry
					ptr = ln;
					ptr++;
					strncpy (rn, ptr, fwutil_min(255, strlen(ptr)-1));
					rn[fwutil_min(255, strlen(ptr-1))] = '\0';
					if (strlen(rn))
					{
						strncpy (repo_r->name, rn, REPONAME_MAX_SIZE);
					}
					else
					{
						g_error ("error parsing");
					}
				}
			}
			fclose (tmp);
			n++;
			if (repo_r == NULL)
				return;
			memset (repo_r, 0, sizeof(gfpm_repo_t));
			strncpy (repo_r->name, rn, REPONAME_MAX_SIZE);
			// populate the repo list here
			repo_r->servers = gfpm_repomgr_get_servers_from_repofile (str);
			
			// and then append it to our repo list
			repolist->list = g_list_append (repolist->list, (gpointer)repo_r);
		}
	}
	repolist->n = n;

	return;
}

static void
gfpm_repomgr_populate_repotvw (void)
{
	GtkListStore *store = NULL;
	GtkTreeIter iter;
	GList *ret = NULL;

	gfpm_repomgr_populate_repolist ();
	
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gfpm_repomgr_treeview)));
	ret = repolist->list;
	while (ret != NULL)
	{
		gfpm_repo_t	*repo = NULL;
		GList		*ser = NULL;

		repo = ret->data;
		gtk_list_store_append (GTK_LIST_STORE(store), &iter);
		gtk_list_store_set (store, &iter, 1, (char*)repo->name, -1);
		ret = g_list_next (ret);
	}
}

void
gfpm_repomanager_show ()
{
	gfpm_repomgr_populate_repotvw ();
	gtk_widget_show (GTK_WIDGET(gfpm_repomgr_dlg));

	return;
}

