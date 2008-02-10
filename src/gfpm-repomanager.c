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
#include "gfpm-interface.h"
#include "gfpm-util.h"
#include <glib.h>

#define CONF_FILE "/etc/pacman-g2.conf"
#define REPO_PATH "/etc/pacman-g2/repos"

static gfpm_repolist_t 	*repolist = NULL;
static gchar			*curr_repo = NULL;

/* Repository manager widgets */
static GtkWidget *gfpm_repomgr_dlg;
static GtkWidget *gfpm_repomgr_treeview;
static GtkWidget *gfpm_repomgr_btnadd;
static GtkWidget *gfpm_repomgr_btndel;
static GtkWidget *gfpm_repomgr_btnmup;
static GtkWidget *gfpm_repomgr_btnmdn;
static GtkWidget *gfpm_repomgr_btnedit;

/* Server manager widgets */
static GtkWidget *gfpm_servmgr_dlg;
static GtkWidget *gfpm_servmgr_treeview;
static GtkWidget *gfpm_servmgr_btnadd;
static GtkWidget *gfpm_servmgr_btndel;
static GtkWidget *gfpm_servmgr_btnmup;
static GtkWidget *gfpm_servmgr_btnmdn;
static GtkWidget *gfpm_servmgr_btnedit;

static void gfpm_write_servers_to_file (const gchar *reponame);

/* signal callbacks */
static void cb_gfpm_repomgr_btnedit_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_servmgr_btndel_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_servmgr_btnadd_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_servmgr_btnup_clicked (GtkButton *button, gpointer data);

void
gfpm_repomanager_init (void)
{
	GtkListStore		*store = NULL;
	GtkCellRenderer		*renderer = NULL;
	GtkTreeViewColumn	*column = NULL;

	gfpm_repomgr_dlg = gfpm_get_widget ("gfpm_repomanager");
	gfpm_repomgr_treeview = gfpm_get_widget ("repoman_listview");
	gfpm_repomgr_btnadd = gfpm_get_widget ("repoman_add");
	gfpm_repomgr_btndel = gfpm_get_widget ("repoman_del");
	gfpm_repomgr_btnedit = gfpm_get_widget ("repoman_edit");
	gfpm_repomgr_btnmup = gfpm_get_widget ("repoman_mup");
	gfpm_repomgr_btnmdn = gfpm_get_widget ("repoman_mdn");
	gfpm_servmgr_dlg = gfpm_get_widget ("gfpm_repomanager_servermgr");
	gfpm_servmgr_treeview = gfpm_get_widget ("servman_listview");
	gfpm_servmgr_btnadd = gfpm_get_widget ("servman_add");
	gfpm_servmgr_btndel = gfpm_get_widget ("servman_del");
	gfpm_servmgr_btnedit = gfpm_get_widget ("servman_edit");
	gfpm_servmgr_btnmup = gfpm_get_widget ("servman_mup");
	gfpm_servmgr_btnmdn = gfpm_get_widget ("servman_mdn");

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
	
	/* setup server store */
	store = gtk_list_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes (_("S"),
														renderer,
														"pixbuf", 0,
														NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_servmgr_treeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Server"),
														renderer,
														"text", 1,
														NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_column_set_expand (column, TRUE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_servmgr_treeview), column);
	gtk_tree_view_set_model (GTK_TREE_VIEW(gfpm_servmgr_treeview), GTK_TREE_MODEL(store));
	
	/* connect important signals */
	/* repository manager signals */
	g_signal_connect (G_OBJECT(gfpm_repomgr_btnedit), "clicked", G_CALLBACK(cb_gfpm_repomgr_btnedit_clicked), NULL);
	
	/* server manager signals */
	g_signal_connect (G_OBJECT(gfpm_servmgr_btndel), "clicked", G_CALLBACK(cb_gfpm_servmgr_btndel_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_servmgr_btnadd), "clicked", G_CALLBACK(cb_gfpm_servmgr_btnadd_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_servmgr_btnmup), "clicked", G_CALLBACK(cb_gfpm_servmgr_btnup_clicked), NULL);

	return;
}

static GList *
gfpm_repomgr_get_servers_from_repofile (const char *conf_file)
{
	FILE		*fp = NULL;
	GList		*ret = NULL;
	char		line[PATH_MAX+1] = "";
	char		server[PATH_MAX+1] = "";
	gfpm_server_entry_t *entry = NULL;
	
	fp = fopen (conf_file, "r");
	if (fp == NULL)
	{
		g_error ("No configuration file found");
		return NULL;
	}
	entry = (gfpm_server_entry_t*) malloc (sizeof(gfpm_server_entry_t));
	memset (entry, 0, sizeof(gfpm_server_entry_t));
	while (fgets(line, PATH_MAX, fp))
	{
		if (line[0] == '#')
		{
			continue;
		}
		else if (line[0]=='[')
			break;
	}
	while (fgets(line, PATH_MAX, fp))
	{
		fwutil_trim (line);
		if (!strlen(line))
			continue;
		if (line[0] == '#')
		{
			entry->comments = g_list_append (entry->comments, g_strdup(line));
			//g_print ("COMMENT: %s\n", entry->comments);
			continue;
		}
		if (sscanf(line, "Server = %s", server))
		{
			sprintf (entry->url, "%s", fwutil_trim(server));
			ret = g_list_append (ret, entry);
			entry = (gfpm_server_entry_t*) malloc (sizeof(gfpm_server_entry_t));
			memset (entry, 0, sizeof(gfpm_server_entry_t));
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
	char str[256];
	char line[PATH_MAX+1];
	
	gint n = 0;

	fp = fopen (CONF_FILE, "r");
	if (fp == NULL)
	{
		g_error ("No configuration file found");
		return;
	}
	if (repolist!=NULL)
		g_free (repolist);

	repolist = (gfpm_repolist_t*)malloc(sizeof(gfpm_repolist_t));
	if (repolist == NULL)
	{
		g_error ("Error allocating memory\n");
		return;
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
				char svr[256];
				gfpm_repo_t *repo_r = (gfpm_repo_t*)malloc(sizeof(gfpm_repo_t));
				memset (repo_r, 0, sizeof(gfpm_repo_t));
				if (repo_r == NULL)
				{
					g_error ("Error allocating memory. Exiting");
					return;
				}
				memset (repo_r, 0, sizeof(gfpm_repo_t));
				strncpy (repo_r->name, reponame, REPONAME_MAX_SIZE);
				// get the server url
				fgets (line, PATH_MAX, fp);
				sscanf (line, "Server = %s", svr);
				repo_r->servers = g_list_append (repo_r->servers, (gpointer)g_strdup(svr));
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
			memset (repo_r, 0, sizeof(gfpm_repo_t));
			
			if ((tmp = fopen (str, "r")) == NULL)
			{
				g_error ("Error opening repository servers file.");
				break;
			}
			while (fgets(ln, PATH_MAX, tmp))
			{
				fwutil_trim (ln);
				if (!strlen(ln))
					continue;
				if (ln[0] == '#')
				{
					repo_r->header = g_list_append (repo_r->header, g_strdup(ln));
					continue;
				}
				else
				if (ln[0] == '[' && ln[strlen(ln)-1] == ']')
				{
					// could be a repo entry
					ptr = ln;
					ptr++;
					strncpy (rn, ptr, fwutil_min(255, strlen(ptr)-1));
					rn[fwutil_min(255, strlen(ptr+1))] = '\0';
					if (strlen(rn))
					{
						strncpy (repo_r->name, rn, REPONAME_MAX_SIZE);
					}
					else
					{
						g_error ("error parsing");
					}
					break;
				}
			}
			fclose (tmp);
			n++;
			if (repo_r == NULL)
				return;
			//memset (repo_r, 0, sizeof(gfpm_repo_t));
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
	GtkListStore 	*store = NULL;
	GtkTreeIter 	iter;
	GList 			*ret = NULL;
	GdkPixbuf		*pixbuf = NULL;

	gfpm_repomgr_populate_repolist ();
	pixbuf = gfpm_get_icon ("gfpm", 32);
	
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gfpm_repomgr_treeview)));
	gtk_list_store_clear (store);
	if (repolist != NULL)
		ret = repolist->list;
	else
	{
		gfpm_message (_("Warning"), "No usable package repositories configured");
		return;
	}
	while (ret != NULL)
	{
		gfpm_repo_t	*repo = NULL;

		repo = ret->data;
		gtk_list_store_append (GTK_LIST_STORE(store), &iter);
		gtk_list_store_set (store, &iter, 0, pixbuf, 1, (char*)repo->name, -1);
		ret = g_list_next (ret);
	}
	
	return;
}

static void
gfpm_repomgr_populate_servtvw (const char *repo)
{
	GtkListStore 	*store = NULL;
	GtkTreeIter 	iter;
	GList 			*rlist = NULL;
	GList			*slist = NULL;
	GdkPixbuf		*pixbuf = NULL;
	gfpm_repo_t		*repository = NULL;

	pixbuf = gfpm_get_icon ("gfpm", 32);
	
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(gfpm_servmgr_treeview)));
	gtk_list_store_clear (store);
	rlist = repolist->list;
	while (rlist != NULL)
	{
		gfpm_repo_t	*rp = NULL;

		rp = rlist->data;
		if (!strcmp(rp->name, repo))
		{
			repository = rp;
			slist = repository->servers;
			break;
		}
		rlist = g_list_next (rlist);
	}
	
	while (slist != NULL)
	{
		gfpm_server_entry_t *entry = slist->data;
		gtk_list_store_append (GTK_LIST_STORE(store), &iter);
		gtk_list_store_set (store, &iter, 0, pixbuf, 1, (char*)entry->url, -1);
		slist = g_list_next (slist);
	}
	
	return;
}

void
gfpm_repomanager_show ()
{
	gfpm_repomgr_populate_repotvw ();
	gtk_widget_show (GTK_WIDGET(gfpm_repomgr_dlg));

	return;
}

void
gfpm_servmanager_show (const char *repo)
{
	gfpm_repomgr_populate_servtvw (repo);
	gtk_widget_show (GTK_WIDGET(gfpm_servmgr_dlg));

	return;
}

static void
gfpm_repomgr_set_current_repo (const char *reponame)
{
	if (curr_repo)
		g_free (curr_repo);
	curr_repo = g_strdup_printf (reponame);
	
	return;
}

static void
gfpm_servmgr_delete_server (const char *server)
{
	GList		*rlist = NULL;
	GList		*slist = NULL;
	gfpm_repo_t *r = NULL;

	rlist = repolist->list;
	while (rlist != NULL)
	{
		r = rlist->data;
		if (!strcmp(r->name,curr_repo))
		{
			slist = r->servers;
			break;
		}
		rlist = g_list_next (rlist);
	}
	while (slist != NULL)
	{
		gfpm_server_entry_t *s = slist->data;
		if (!strcmp(s->url,server))
		{
			r->servers = g_list_delete_link (r->servers, slist);
			break;
		}
		slist = g_list_next (slist);
	}
	
	/* update repository file */
	gfpm_write_servers_to_file (curr_repo);

	/* repopulate the server list and display */
	gfpm_repomgr_populate_servtvw (curr_repo);

	return;
}

static void
gfpm_servmgr_move_up_server (const char *server)
{
	GList		*rlist = NULL;
	GList		*slist = NULL;
	GList		*prev = NULL;
	gfpm_repo_t *r = NULL;

	rlist = repolist->list;
	while (rlist != NULL)
	{
		r = rlist->data;
		if (!strcmp(r->name,curr_repo))
		{
			slist = r->servers;
			break;
		}
		rlist = g_list_next (rlist);
	}
	while (slist != NULL)
	{
		gfpm_server_entry_t *s = slist->data;
		if (!strcmp(s->url,server))
		{
			//r->servers = g_list_delete_link (r->servers, slist);
			GList *new = prev->next;
			new->prev = slist;
			slist->prev = prev;
			prev->next = slist;
			slist->next = new;
			new->next = NULL;
			break;
		}
		prev = slist->prev;
		slist = g_list_next (slist);
	}
	
	/* update repository file */
	gfpm_write_servers_to_file (curr_repo);

	/* repopulate the server list and display */
	gfpm_repomgr_populate_servtvw (curr_repo);

	return;
}

static void
gfpm_write_servers_to_file (const gchar *reponame)
{
	FILE	*fp = NULL;
	gchar 	*path = NULL;
	
	path = g_strdup_printf ("%s/%s", REPO_PATH, curr_repo);
	if ((fp=fopen(path,"w"))!=NULL)
	{
		GList *rlist = NULL;
		GList *slist = NULL;
		GList *header = NULL;
		gfpm_repo_t *r = NULL;
		
		rlist = repolist->list;
		while (rlist != NULL)
		{
			r = rlist->data;
			
			if (!strcmp(r->name,curr_repo))
			{
				slist = r->servers;
				header = r->header;
				break;
			}
			rlist = g_list_next (rlist);
		}
		
		/* write the header */
		while (header != NULL)
		{
			fprintf (fp, "%s\n", (char*)header->data);
			header = g_list_next (header);
		}
		/* write the repository name */
		fprintf (fp, "\n[%s]\n\n", r->name);
		/* and finally, the servers */
		while (slist != NULL)
		{
			gfpm_server_entry_t *s = NULL;
			GList *comment = NULL;
			s = slist->data;
			comment = s->comments;
			while (comment != NULL)
			{
				fprintf (fp, "%s\n", (char*)comment->data);
				comment = g_list_next (comment);
			}
			fprintf (fp, "Server = %s\n\n", s->url);
			slist = g_list_next (slist);
		}
		fclose (fp);
	}
	else
	{
		gfpm_error (_("Error"), _("Error opening repository file"));
	}
	g_free (path);
	
	return;
}

/* CALLBACKS */
static void
cb_gfpm_repomgr_btnedit_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter			iter;
	gchar				*repo = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_repomgr_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 1, &repo, -1);
		gfpm_servmanager_show (repo);
		gfpm_repomgr_set_current_repo (repo);
	}

	return;
}

static void
cb_gfpm_servmgr_btnadd_clicked (GtkButton *button, gpointer data)
{
	gint				msgres = NULL;
	gchar				*path = NULL;
	gchar				*server = NULL;
	GList				*rlist = NULL;
	GList				*slist = NULL;
	gfpm_repo_t			*rp = NULL;
	gfpm_server_entry_t *s = NULL;
	
	server = gfpm_input (_("Add new server"), _("Enter the URL of the server: "), &msgres);
	if (msgres != GTK_RESPONSE_ACCEPT)
		return;
	if (!strlen(server))
		return;
	/* check if the server already exists */
	rlist = repolist->list;
	while (rlist != NULL)
	{
		rp = rlist->data;
		if (!strcmp(rp->name, curr_repo))
		{
			slist = rp->servers;
			break;
		}
	}
	while (slist != NULL)
	{
		if (!strcmp(slist->data,server))
		{
			gfpm_error (_("Server already exists"), _("The server you're trying to add already exists"));
			return;
		}
		slist = g_list_next (slist);
	}
	s = (gfpm_server_entry_t*) malloc (sizeof(gfpm_server_entry_t));
	memset (s, 0, sizeof(gfpm_server_entry_t));
	sprintf (s->url, "%s", server);
	rp->servers = g_list_append (rp->servers, s);

	gfpm_write_servers_to_file (curr_repo);
	gfpm_repomgr_populate_servtvw (curr_repo);

	g_free (path);
	
	return;
}

static void
cb_gfpm_servmgr_btndel_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter			iter;
	gchar				*server = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_servmgr_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 1, &server, -1);
		gfpm_servmgr_delete_server (server);
	}
}

static void
cb_gfpm_servmgr_btnup_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter			iter;
	gchar				*server = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_servmgr_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 1, &server, -1);
		gfpm_servmgr_move_up_server (server);
	}
}
