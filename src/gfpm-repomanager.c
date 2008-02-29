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

#define CONF_FILE	"/etc/pacman-g2.conf"
#define REPO_PATH	"/etc/pacman-g2/repos"

#define	MOVE_UP		1
#define MOVE_DN		0

extern GtkWidget *gfpm_mw;

static gfpm_repolist_t 	*repolist = NULL;
static gchar		*curr_repo = NULL;

/* Repository manager widgets */
static GtkWidget *gfpm_repomgr_dlg;
static GtkWidget *gfpm_repomgr_repo_input_dlg;
static GtkWidget *gfpm_repomgr_repo_input_dlg_entry1;
static GtkWidget *gfpm_repomgr_repo_input_dlg_entry2;
static GtkWidget *gfpm_repomgr_repo_input_dlg_entry3;
static GtkWidget *gfpm_repomgr_treeview;
static GtkWidget *gfpm_repomgr_btnadd;
static GtkWidget *gfpm_repomgr_btndel;
static GtkWidget *gfpm_repomgr_btnmup;
static GtkWidget *gfpm_repomgr_btnmdn;
static GtkWidget *gfpm_repomgr_btnedit;

/* Server manager widgets */
static GtkWidget *gfpm_servmgr_dlg;
static GtkWidget *gfpm_servmgr_server_input_dlg;
static GtkWidget *gfpm_servmgr_server_input_dlg_entry1;
static GtkWidget *gfpm_servmgr_server_input_dlg_entry2;
static GtkWidget *gfpm_servmgr_treeview;
static GtkWidget *gfpm_servmgr_btnadd;
static GtkWidget *gfpm_servmgr_btndel;
static GtkWidget *gfpm_servmgr_btnmup;
static GtkWidget *gfpm_servmgr_btnmdn;
static GtkWidget *gfpm_servmgr_btnedit;

static void gfpm_write_config_file (void);
static void convert_server_to_repofile (void);
static void gfpm_write_servers_to_file (const gchar *reponame);
static void gfpm_servmgr_edit_server (gfpm_server_entry_t *s);
static gfpm_repo_t * gfpm_repomgr_get_repo_input (void);
static gfpm_server_entry_t * gfpm_servmgr_get_server_input (void);

/* signal callbacks */
static void cb_gfpm_repomgr_btnadd_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_repomgr_btnedit_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_servmgr_btndel_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_servmgr_btnedit_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_servmgr_btnadd_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_servmgr_btnup_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_servmgr_btndown_clicked (GtkButton *button, gpointer data);
static void cb_gfpm_repo_enable_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data);

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
	gfpm_servmgr_server_input_dlg = gfpm_get_widget ("gfpm_servmgr_input_dlg");
	gfpm_servmgr_server_input_dlg_entry1 = gfpm_get_widget ("entry1");
	gfpm_servmgr_server_input_dlg_entry2 = gfpm_get_widget ("commentview");
	gfpm_repomgr_repo_input_dlg = gfpm_get_widget ("gfpm_repomgr_input_dlg");
	gfpm_repomgr_repo_input_dlg_entry1 = gfpm_get_widget ("reponame");
	gfpm_repomgr_repo_input_dlg_entry2 = gfpm_get_widget ("serverurl");
	gfpm_repomgr_repo_input_dlg_entry3 = gfpm_get_widget ("commentview2");

	/* setup repo store */
	store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_BOOLEAN, G_TYPE_STRING);
	
	renderer = gtk_cell_renderer_pixbuf_new ();
	column = gtk_tree_view_column_new_with_attributes (_("S"),
								renderer,
								"pixbuf", 0,
								NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_repomgr_treeview), column);
	
	renderer = gtk_cell_renderer_toggle_new ();
	g_object_set (G_OBJECT(renderer), "activatable", TRUE, NULL);
	g_signal_connect (renderer, "toggled", G_CALLBACK(cb_gfpm_repo_enable_toggled), store);
	column = gtk_tree_view_column_new_with_attributes (_("Enabled"),
							renderer,
							"active", 1,
							NULL);
	gtk_tree_view_column_set_resizable (column, FALSE);
	gtk_tree_view_append_column (GTK_TREE_VIEW(gfpm_repomgr_treeview), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Repository"),
								renderer,
								"text", 2,
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
	g_signal_connect (G_OBJECT(gfpm_repomgr_btnadd), "clicked", G_CALLBACK(cb_gfpm_repomgr_btnadd_clicked), NULL);
	
	/* server manager signals */
	g_signal_connect (G_OBJECT(gfpm_servmgr_btndel), "clicked", G_CALLBACK(cb_gfpm_servmgr_btndel_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_servmgr_btnadd), "clicked", G_CALLBACK(cb_gfpm_servmgr_btnadd_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_servmgr_btnedit), "clicked", G_CALLBACK(cb_gfpm_servmgr_btnedit_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_servmgr_btnmup), "clicked", G_CALLBACK(cb_gfpm_servmgr_btnup_clicked), NULL);
	g_signal_connect (G_OBJECT(gfpm_servmgr_btnmdn), "clicked", G_CALLBACK(cb_gfpm_servmgr_btndown_clicked), NULL);

	convert_server_to_repofile ();
	
	return;
}

/* This function replaces every occurence of
 * [foo]
 * Server = http://foo.com/
 * with
 * Include = /etc/pacman-g2/repos/foo
 * and creates a new file /etc/pacman-g2/repos/foo with
 * that server URL. This makes parsing and editing with
 * gfpm-repomanager much easier */
static void
convert_server_to_repofile (void)
{
	char line[PATH_MAX+1] = "";
	char *ptr = NULL;
	FILE *fp = fopen (CONF_FILE, "r");
	FILE *bkp = tmpfile ();

	if (fp == NULL)
	{
		return;
	}
	while (fgets(line, PATH_MAX, fp))
	{
		char reponame[256] = "";
		fwutil_trim (line);
		if (line[0] == '#')
		{
			fprintf (bkp, "%s\n", line);
			continue;
		}
		else
		if (line[0] == '[' && line[strlen(line)-1] == ']')
		{
			ptr = line;
			ptr++;
			strncpy (reponame, ptr, fwutil_min(255, strlen(ptr)-1));
			reponame[fwutil_min(255,strlen(ptr-1))] = '\0';
			if (!strlen(reponame))
				continue;
			if (!strcmp(reponame,"options"))
			{
				fprintf (bkp, "%s\n", line);
				continue;
			}
			else
			{
				char *path = g_strdup_printf ("%s/%s", REPO_PATH, reponame);
				FILE *newp = fopen (path, "w");
				char server[256] = "";

				if (newp == NULL) return;
				fprintf (newp, "# \n# %s repository\n# \n\n", reponame);
				fprintf (newp, "[%s]\n\n", reponame);
				do {
					fgets (line, PATH_MAX, fp);
				} while (line[0] != 'S');
				sscanf (line, "Server = %s", server);
				fprintf (newp, "Server = %s\n", server);
				fprintf (bkp, "Include = %s\n", path);
				g_free (path);
				fclose (newp);
			}
		}
		else
		{
			fprintf (bkp, "%s\n", line);
		}
	}
	fclose (fp);
	rewind (bkp);
	fp = fopen (CONF_FILE, "w");
	while (fgets(line, PATH_MAX, bkp))
		fprintf (fp, "%s", line);
	fclose (fp);
	fclose (bkp);

	return;
}

static void
gfpm_write_config_file (void)
{
	FILE		*fp = NULL;
	GList		*rlist = NULL;
	gfpm_repo_t	*repo = NULL;
	GList		*header = NULL;
	
	fp = fopen (CONF_FILE, "w");
	if (fp == NULL)
	{
		gfpm_error (_("Error saving configuration"), _("Gfpm could not open the pacman-g2.conf file for saving. Make sure no other program is using it"));
		return;
	}
	
	/* write the header first */
	header = repolist->header;
	while (header != NULL)
	{
		fprintf (fp, "%s\n", (char*)header->data);
		header = g_list_next (header);
	}
	/* now the body and footer */
	rlist = repolist->list;
	while (rlist != NULL)
	{
		GList *footer = NULL;	
		repo = rlist->data;
		/* write the repository entry */
		if (repo->enabled)
			fprintf (fp, "Include = %s/%s\n", REPO_PATH, repo->name);
		else
			fprintf (fp, "#Include = %s/%s\n", REPO_PATH, repo->name);
		
		gfpm_write_servers_to_file (repo->name);
		/* write the footer */
		footer = repo->footer;
		if (footer == NULL)
		fprintf (fp, "\n");
		while (footer != NULL)
		{
			fprintf (fp, "%s\n", (char*)footer->data);
			footer = g_list_next (footer);
		}
		
		rlist = g_list_next (rlist);
	}
	fclose (fp);
	
	return;
}

static gfpm_server_entry_t *
gfpm_servmgr_get_server_input (void)
{
	gint			response;
	gchar			*url = NULL;
	GtkTextBuffer		*buffer = NULL;
	gfpm_server_entry_t	*ret = NULL;

	ret = (gfpm_server_entry_t *) malloc (sizeof(gfpm_server_entry_t));
	memset (ret, 0, sizeof(gfpm_server_entry_t));
	run: response = gtk_dialog_run (GTK_DIALOG(gfpm_servmgr_server_input_dlg));
	url = (char*) gtk_entry_get_text (GTK_ENTRY(gfpm_servmgr_server_input_dlg_entry1));
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(gfpm_servmgr_server_input_dlg_entry2));
	
	switch (response)
	{
		gchar 		*comments = NULL;
		GtkTextIter 	siter;
		GtkTextIter 	eiter;
		
		case 32: /* OK Button */
		{
			if (url == NULL || !strlen (url))
			{
				gfpm_error (_("Error"), _("The Repository URL field cannot be left blank"));
				gtk_widget_hide (gfpm_servmgr_server_input_dlg);
				goto run;
			}
			strncpy (ret->url, url, strlen(url));
			gtk_text_buffer_get_start_iter (buffer, &siter);
			gtk_text_buffer_get_end_iter (buffer, &eiter);
			comments = gtk_text_buffer_get_text (buffer, &siter, &eiter, FALSE);
			if (comments != NULL)
			{
				gchar *cslice = NULL;
				cslice = strtok (comments, "\n");
				if (cslice != NULL)
				{
					do {
						gchar *text = g_strdup_printf ("# %s", cslice);
						ret->comments = g_list_append (ret->comments, (gpointer) text);
					} while ((cslice=strtok(NULL,"\n"))!=NULL);
				}
			}
			gtk_entry_set_text (GTK_ENTRY(gfpm_servmgr_server_input_dlg_entry1), "");
			gtk_text_buffer_set_text (buffer, "", 0);
			gtk_text_view_set_buffer (GTK_TEXT_VIEW(gfpm_servmgr_server_input_dlg_entry2), buffer);
			gtk_widget_hide (gfpm_servmgr_server_input_dlg);
			break;
		}
		
		case 64: /* CANCEL Button */
			gtk_entry_set_text (GTK_ENTRY(gfpm_servmgr_server_input_dlg_entry1), "");
			gtk_text_buffer_set_text (buffer, "", 0);
			gtk_text_view_set_buffer (GTK_TEXT_VIEW(gfpm_servmgr_server_input_dlg_entry2), buffer);
			gtk_widget_hide (gfpm_servmgr_server_input_dlg);
			g_free (ret);
			return NULL;

			break;
	}	

	return ret;
}

static gfpm_repo_t *
gfpm_repomgr_get_repo_input (void)
{
	gint			response;
	gchar			*name = NULL;
	gchar			*url = NULL;
	GtkTextBuffer		*buffer = NULL;
	gfpm_repo_t		*ret = NULL;
	gfpm_server_entry_t	*server = NULL;

	ret = (gfpm_repo_t *) malloc (sizeof(gfpm_repo_t));
	memset (ret, 0, sizeof(gfpm_repo_t));
	run: response = gtk_dialog_run (GTK_DIALOG(gfpm_repomgr_repo_input_dlg));
	name = (char*) gtk_entry_get_text (GTK_ENTRY(gfpm_repomgr_repo_input_dlg_entry1));
	url = (char*) gtk_entry_get_text (GTK_ENTRY(gfpm_repomgr_repo_input_dlg_entry2));
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(gfpm_repomgr_repo_input_dlg_entry3));
	
	switch (response)
	{
		gchar 		*comments = NULL;
		GList		*rlist = NULL;
		GtkTextIter 	siter;
		GtkTextIter 	eiter;
		
		case 32: /* OK Button */
		{
			if (name == NULL || !strlen (name))
			{
				gfpm_error (_("Error"), _("The Repository Name field cannot be left blank"));
				gtk_widget_hide (gfpm_repomgr_repo_input_dlg);
				goto run;
			}
			if (url == NULL || !strlen (url))
			{
				gfpm_error (_("Error"), _("The Repository URL field cannot be left blank"));
				gtk_widget_hide (gfpm_repomgr_repo_input_dlg);
				goto run;
			}
			/* check if repository name already exists */
			rlist = repolist->list;
			while (rlist != NULL)
			{
				gfpm_repo_t *r = NULL;
				r = rlist->data;
				if (!strcmp(r->name, name))
				{
					gfpm_error (_("Repository exists"), _("A repository with the specified name already exists. Please provide a different name for the repository."));
					gtk_widget_hide (gfpm_repomgr_repo_input_dlg);
					goto run;
				}
				rlist = g_list_next (rlist);
			}
			/* good, let's add it then */
			strncpy (ret->name, name, strlen(name));
			server = (gfpm_server_entry_t *) malloc (sizeof(gfpm_server_entry_t));
			memset (server, 0, sizeof(gfpm_server_entry_t));
			strncpy (server->url, url, strlen(url));
			ret->header = g_list_append (ret->header, g_strdup_printf ("#\n# %s repository\n#\n", name));
			ret->servers = g_list_append (ret->servers, (gpointer)server);
			gtk_text_buffer_get_start_iter (buffer, &siter);
			gtk_text_buffer_get_end_iter (buffer, &eiter);
			comments = gtk_text_buffer_get_text (buffer, &siter, &eiter, FALSE);
			if (comments != NULL)
			{
				gchar *cslice = NULL;
				cslice = strtok (comments, "\n");
				if (cslice != NULL)
				{
					do {
						gchar *text = g_strdup_printf ("# %s", cslice);
						server->comments = g_list_append (server->comments, (gpointer) text);
					} while ((cslice=strtok(NULL,"\n"))!=NULL);
				}
			}
			gtk_entry_set_text (GTK_ENTRY(gfpm_repomgr_repo_input_dlg_entry1), "");
			gtk_entry_set_text (GTK_ENTRY(gfpm_repomgr_repo_input_dlg_entry2), "");
			gtk_text_buffer_set_text (buffer, "", 0);
			gtk_text_view_set_buffer (GTK_TEXT_VIEW(gfpm_repomgr_repo_input_dlg_entry3), buffer);
			gtk_widget_hide (gfpm_repomgr_repo_input_dlg);
			break;
		}
		
		case 64: /* CANCEL Button */
			gtk_entry_set_text (GTK_ENTRY(gfpm_repomgr_repo_input_dlg_entry1), "");
			gtk_entry_set_text (GTK_ENTRY(gfpm_repomgr_repo_input_dlg_entry2), "");
			gtk_text_buffer_set_text (buffer, "", 0);
			gtk_text_view_set_buffer (GTK_TEXT_VIEW(gfpm_repomgr_repo_input_dlg_entry3), buffer);
			gtk_widget_hide (gfpm_repomgr_repo_input_dlg);
			g_free (ret);
			return NULL;

			break;
	}	

	return ret;
}

static GList *
gfpm_repomgr_get_servers_from_repofile (const char *conf_file)
{
	FILE			*fp = NULL;
	GList			*ret = NULL;
	char			line[PATH_MAX+1] = "";
	char			server[PATH_MAX+1] = "";
	gfpm_server_entry_t 	*entry = NULL;
	
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
gfpm_repomgr_populate_repo_info (const char *path, gfpm_repo_t *repo_r)
{
	FILE *tmp = NULL;
	char ln[PATH_MAX+1];
	char rn[PATH_MAX+1];
	
	if ((tmp = fopen (path, "r")) == NULL)
	{
		gchar *errorstr = g_strdup_printf ("%s for %s %s", _("Error opening repository servers file"), path, _("repository"));
		gfpm_error (_("Error opening repository file"), errorstr);
		g_free (errorstr);
		return;
	}
	while (fgets(ln, PATH_MAX, tmp))
	{
		fwutil_trim (ln);
		if (!strlen(ln))
			continue;
		if (ln[0] == '#')
		{
			repo_r->header = g_list_append (repo_r->header, (gpointer)g_strdup(ln));
			continue;
		}
		else
		if (ln[0] == '[' && ln[strlen(ln)-1] == ']')
		{
			// could be a repo entry
			char *ptr = ln;
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
	if (repo_r == NULL)
		return;
	strncpy (repo_r->name, rn, REPONAME_MAX_SIZE);
	// populate the server list
	repo_r->servers = gfpm_repomgr_get_servers_from_repofile (path);
	
	return;
}

static void
gfpm_repomgr_populate_repolist (void)
{
	FILE *fp = NULL;
	char str[256];
	char line[PATH_MAX+1];
	gboolean flag = FALSE;
	gfpm_repo_t *repo_r = NULL;
	
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
		fwutil_trim (line);
		if (!strlen(line))
			continue;
		else if (line[0] == '#' && line[1] != 'I')
		{
			if (flag == FALSE)
			{
				repolist->header = g_list_append (repolist->header, (gpointer) g_strdup(line));
			}
			else
			{
				repo_r->footer = g_list_append (repo_r->footer, (gpointer) g_strdup(line));
			}
			continue;
		}
		else if (sscanf(line, "Include = %s", str))
		{
			if (flag == FALSE)
				flag = TRUE;
			repo_r = (gfpm_repo_t*)malloc(sizeof(gfpm_repo_t));
			memset (repo_r, 0, sizeof(gfpm_repo_t));
			gfpm_repomgr_populate_repo_info (str, repo_r);
			repo_r->enabled = TRUE;
			// and then append it to our repo list
			repolist->list = g_list_append (repolist->list, (gpointer)repo_r);
			n++;
		}
		else if (sscanf(line, "#Include = %s", str))
		{
			if (flag == FALSE)
				flag = TRUE;
			repo_r = (gfpm_repo_t*)malloc(sizeof(gfpm_repo_t));
			memset (repo_r, 0, sizeof(gfpm_repo_t));
			gfpm_repomgr_populate_repo_info (str, repo_r);
			repo_r->enabled = FALSE;
			// and then append it to our repo list
			repolist->list = g_list_append (repolist->list, (gpointer)repo_r);
			n++;
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
	GList 		*ret = NULL;
	GdkPixbuf	*pixbuf = NULL;

	gfpm_repomgr_populate_repolist ();
	pixbuf = gfpm_get_icon ("gfpm", 24);
	
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
		gfpm_repo_t *repo = NULL;

		repo = ret->data;
		gtk_list_store_append (GTK_LIST_STORE(store), &iter);
		gtk_list_store_set (store, &iter, 0, pixbuf, 1, repo->enabled ? TRUE:FALSE, 2, (char*)repo->name, -1);
		ret = g_list_next (ret);
	}
	
	return;
}

static void
gfpm_repomgr_populate_servtvw (const char *repo)
{
	GtkListStore 	*store = NULL;
	GtkTreeIter 	iter;
	GList 		*rlist = NULL;
	GList		*slist = NULL;
	GdkPixbuf	*pixbuf = NULL;
	gfpm_repo_t	*repository = NULL;

	pixbuf = gfpm_get_icon ("gfpm", 24);
	
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
	gfpm_repo_t 	*r = NULL;

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
gfpm_servmgr_move_server (const char *server, const int move_direction)
{
	GList		*rlist = NULL;
	GList		*slist = NULL;
	gfpm_repo_t 	*r = NULL;

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
			if (move_direction == MOVE_UP)
			{
				gint pos = g_list_position (r->servers, slist);
				r->servers = g_list_delete_link (r->servers, slist);
				r->servers = g_list_insert (r->servers, s, pos-1);
			}
			else
			{
				gint pos = g_list_position (r->servers, slist);
				r->servers = g_list_delete_link (r->servers, slist);
				r->servers = g_list_insert (r->servers, s, pos+1);
			}
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
gfpm_servmgr_edit_server (gfpm_server_entry_t *s)
{
	gint 		response;
	GtkTextBuffer 	*buffer = NULL;
	GtkTextIter	iter;
	GList		*list = NULL;
	gchar		*line = NULL;
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(gfpm_servmgr_server_input_dlg_entry2));
	gtk_text_buffer_set_text (buffer, "", 0);
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 0);
	list = s->comments;
	while (list != NULL)
	{
		line = g_strdup(list->data);
		if (line[0]=='#') line[0] = ' ';
		fwutil_trim (line);
		gtk_text_buffer_insert (buffer, &iter, (char*)line, -1);
		gtk_text_buffer_insert (buffer, &iter, "\n", -1);
		g_free (line);
		list = g_list_next (list);
	}
	
	gtk_entry_set_text (GTK_ENTRY(gfpm_servmgr_server_input_dlg_entry1), s->url);
	gtk_window_set_title (GTK_WINDOW(gfpm_servmgr_server_input_dlg), _("Edit Server"));
	run:response = gtk_dialog_run (GTK_DIALOG(gfpm_servmgr_server_input_dlg));
	
	switch (response)
	{
		gchar 		*comments = NULL;
		gchar		*url = NULL;
		GtkTextIter 	siter;
		GtkTextIter 	eiter;
		
		case 32: /* OK Button */
		{
			url = (char*) gtk_entry_get_text (GTK_ENTRY(gfpm_servmgr_server_input_dlg_entry1));
			if (url == NULL || !strlen (url))
			{
				gfpm_error (_("Error"), _("The Repository URL field cannot be left blank"));
				gtk_widget_hide (gfpm_servmgr_server_input_dlg);
				goto run;
			}
			strncpy (s->url, url, strlen(url));
			gtk_text_buffer_get_start_iter (buffer, &siter);
			gtk_text_buffer_get_end_iter (buffer, &eiter);
			comments = gtk_text_buffer_get_text (buffer, &siter, &eiter, FALSE);
			if (comments != NULL)
			{
				gchar *cslice = NULL;
				g_list_free (s->comments);
				s->comments = NULL;
				cslice = strtok (comments, "\n");
				if (cslice != NULL)
				{
					do {
						gchar *text = g_strdup_printf ("# %s", cslice);
						s->comments = g_list_append (s->comments, (gpointer) text);
					} while ((cslice=strtok(NULL,"\n"))!=NULL);
				}
			}
			gtk_entry_set_text (GTK_ENTRY(gfpm_servmgr_server_input_dlg_entry1), "");
			gtk_text_buffer_set_text (buffer, "", 0);
			gtk_text_view_set_buffer (GTK_TEXT_VIEW(gfpm_servmgr_server_input_dlg_entry2), buffer);
			/* update repository file */
			gfpm_write_servers_to_file (curr_repo);
			/* repopulate the server list and display */
			gfpm_repomgr_populate_servtvw (curr_repo);
			gtk_widget_hide (gfpm_servmgr_server_input_dlg);
			break;
		}
		
		case 64: /* CANCEL Button */
			gtk_entry_set_text (GTK_ENTRY(gfpm_servmgr_server_input_dlg_entry1), "");
			gtk_text_buffer_set_text (buffer, "", 0);
			gtk_text_view_set_buffer (GTK_TEXT_VIEW(gfpm_servmgr_server_input_dlg_entry2), buffer);
			gtk_widget_hide (gfpm_servmgr_server_input_dlg);
			break;
	}
	/* set the original title back */
	gtk_window_set_title (GTK_WINDOW(gfpm_servmgr_server_input_dlg), _("New Server"));
	
	return;
}

static void
gfpm_write_servers_to_file (const gchar *reponame)
{
	FILE	*fp = NULL;
	gchar 	*path = NULL;
	
	path = g_strdup_printf ("%s/%s", REPO_PATH, reponame);
	curr_repo = g_strdup_printf (reponame);
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
			
			if (curr_repo!=NULL)
			{
				if (!strcmp(r->name,curr_repo))
				{
					slist = r->servers;
					header = r->header;
					break;
				}
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
cb_gfpm_repo_enable_toggled (GtkCellRendererToggle *toggle, gchar *path_str, gpointer data)
{
	GtkTreeModel	*model;
	GtkTreeIter	iter;
	GtkTreePath	*path;
	gchar		*sel = NULL;
	gboolean	check;
	GList		*repos = NULL;
	gfpm_repo_t	*repo_r = NULL;

	model = (GtkTreeModel *)data;
	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, 1, &check, 2, &sel, -1);

	/* manually toggle the toggle button */
	check ^= 1;
	gtk_list_store_set (GTK_LIST_STORE(model), &iter, 1, check, -1);
	
	repos = repolist->list;
	while (repos!=NULL)
	{
		repo_r = repos->data;
		if (!strcmp(repo_r->name, sel))
		{
			repo_r->enabled = (check==TRUE) ? TRUE:FALSE;
			break;
		}
		repos = g_list_next (repos);
	}

	/* write config file */
	gfpm_write_config_file ();

	g_free (sel);
	gtk_tree_path_free (path);

	return;
}

static void
cb_gfpm_repomgr_btnadd_clicked (GtkButton *button, gpointer data)
{
	GList		*rlist = NULL;
	gfpm_repo_t	*rp = NULL;
	
	rlist = repolist->list;
	rp = gfpm_repomgr_get_repo_input ();
	if (rp == NULL)
		return;
	/* increment the repo count */
	repolist->n++;
	/* and add the repo */
	rlist = g_list_append (rlist, (gpointer)rp);
	/* save the configuration file */
	gfpm_write_config_file ();
	/* re-populate repository list */
	gfpm_repomgr_populate_repotvw ();
	
	return;
}

static void
cb_gfpm_repomgr_btnedit_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter		iter;
	gchar			*repo = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_repomgr_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 2, &repo, -1);
		gfpm_servmanager_show (repo);
		gfpm_repomgr_set_current_repo (repo);
	}

	return;
}

static void
cb_gfpm_servmgr_btnadd_clicked (GtkButton *button, gpointer data)
{
	gfpm_server_entry_t 	*s = NULL;
	GList			*rlist = NULL;
	gfpm_repo_t		*rp = NULL;
	GList			*slist = NULL;
	
	s = gfpm_servmgr_get_server_input ();
	if (s == NULL)
		return;
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
		if (!strcmp(slist->data,s->url))
		{
			gfpm_error (_("Server already exists"), _("The server you're trying to add already exists"));
			g_free (s);
			return;
		}
		slist = g_list_next (slist);
	}
	rp->servers = g_list_append (rp->servers, s);
	
	/* update repository file */
	gfpm_write_servers_to_file (curr_repo);

	/* repopulate the server list and display */
	gfpm_repomgr_populate_servtvw (curr_repo);
	
	return;
}

static void
cb_gfpm_servmgr_btnedit_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter		iter;
	gchar			*server = NULL;
	GList			*rlist;
	gfpm_repo_t		*rp = NULL;
	gfpm_server_entry_t 	*sp = NULL;
	GList			*slist = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_servmgr_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 1, &server, -1);
		rlist = repolist->list;
		while (rlist != NULL)
		{
			rp = rlist->data;
			if (!strcmp(rp->name, curr_repo))
			{
				slist = rp->servers;
				break;
			}
			rlist = g_list_next (rlist);
		}
		while (slist != NULL)
		{
			sp = slist->data;
			if (!strcmp(sp->url,server))
				break;
			slist = g_list_next (slist);
		}
		gfpm_servmgr_edit_server (sp);
	}
	
	return;
}

static void
cb_gfpm_servmgr_btndel_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter		iter;
	gchar			*server = NULL;

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
	GtkTreeIter		iter;
	gchar			*server = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_servmgr_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 1, &server, -1);
		gfpm_servmgr_move_server (server, MOVE_UP);
	}
}

static void
cb_gfpm_servmgr_btndown_clicked (GtkButton *button, gpointer data)
{
	GtkTreeSelection	*selection = NULL;
	GtkTreeModel		*model;
	GtkTreeIter		iter;
	gchar			*server = NULL;

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gfpm_servmgr_treeview));
	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get (model, &iter, 1, &server, -1);
		gfpm_servmgr_move_server (server, MOVE_DN);
	}
}

