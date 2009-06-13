/*
 *  nautilus-gfpm.c for GFpm
 *
 *  Copyright (c) 2009 by Priyank Gosalia <priyankmg@gmail.com>
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
#include <config.h>

#ifdef HAVE_NAUTILUS_EXT

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <pacman.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-file-info.h>
#include <libnautilus-extension/nautilus-property-page-provider.h>
#include <libnautilus-extension/nautilus-menu-provider.h>
#include "nautilus-gfpm.h"



static GObjectClass *parent_class;

static GtkWidget *
_create_package_info_treeview (void)
{
	GtkWidget		*treeview = NULL;
	GtkListStore	*store = NULL;
	GtkCellRenderer	*renderer = NULL;
	
	/* create the treeview */
	treeview = gtk_tree_view_new ();
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW(treeview), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW(treeview), FALSE);
	
	/* the list store (model) */
	store = gtk_list_store_new (2,
								G_TYPE_STRING, 	/* package info parameter */
								G_TYPE_STRING); /* value */
	
	/* cell renderers and columns */
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(treeview),
												-1,
												"Info",
												renderer,
												"markup",
												0,
												NULL);

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(treeview),
												-1,
												"Value",
												renderer,
												"text",
												1,
												NULL);
	g_object_set (renderer, "wrap-width", 250, NULL);
	g_object_set (renderer, "wrap-mode", PANGO_WRAP_WORD_CHAR, NULL);

	gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
	g_object_set (G_OBJECT(treeview), "hover-selection", TRUE, NULL);

	return treeview;
}

static GtkWidget *
_create_property_page (void)
{
	GtkWidget	*window = NULL;
	GtkWidget	*treeview = NULL;
	
	/* create the scrolled window */
	window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(window),
									GTK_POLICY_AUTOMATIC,
									GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(window),
										GTK_SHADOW_ETCHED_OUT);

	/* create the treeview */
	treeview = _create_package_info_treeview ();
	
	/* pack the treeview inside the scrolled window container */
	gtk_container_add (GTK_CONTAINER(window), treeview);

	/* set the border width */
	gtk_container_set_border_width (GTK_CONTAINER(window), 5);
	
	/* show our widget */
	gtk_widget_show_all (window);

	return window;
}

static GString *
_pmlist_to_gstring (PM_LIST *list)
{
	GString		*ret = NULL;
	
	if (list != NULL)
	{
		PM_LIST	*i = NULL;
		ret = g_string_new ("");
		for (i=list;i;i=pacman_list_next(i))
		{
			ret = g_string_append (ret, (char*)pacman_list_getdata(i));
			ret = g_string_append (ret, " ");
		}
	}

	return ret;
}

char *
gfpm_bold (const char *text)
{
	if (text == NULL)
		return NULL;
	return ((char*)g_markup_printf_escaped("<b>%s</b>",text));
}

char *
_get_file_ext (const char *file)
{
	char *ret = strrchr (file, '.');
	ret++;

	return ret;
}

static gboolean
_populate_property_page (GtkWidget *page, const gchar *file)
{
	gboolean	ret = TRUE;

	if (pacman_initialize("/") == -1)
	{
		ret = FALSE;
	}
	else
	{
		PM_PKG	*pm_pkg = NULL;

		if (pacman_pkg_load((char*)file,&pm_pkg) == -1)
		{
			ret = FALSE;
		}
		else
		{
			GtkTreeModel	*model = NULL;
			GtkWidget		*tvw = NULL;
			GtkTreeIter		iter;
			gchar			*st = NULL;
			PM_LIST			*temp;
			GString			*str = NULL;
			
			tvw = (gtk_container_get_children(GTK_CONTAINER(page)))->data;
			model = gtk_tree_view_get_model (GTK_TREE_VIEW(tvw));
			gtk_list_store_clear (GTK_LIST_STORE(model));

			/* package name */
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			st = (char*)gfpm_bold (_("Name:"));
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								0, st,
								1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_NAME),
								-1);
			g_free (st);

			/* package version */
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			st = (char*)gfpm_bold (_("Version:"));
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								0, st,
								1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_VERSION),
								-1);
			g_free (st);

			/* package description */
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			st = (char*)gfpm_bold (_("Description:"));
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								0, st,
								1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_DESC),
								-1);
			g_free (st);

			/* package url */
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			st = (char*)gfpm_bold (_("URL:"));
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								0, st,
								1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_URL),
							-1);
			g_free (st);

			/* package license */
			temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_LICENSE);
			str = _pmlist_to_gstring (temp);	
			if (str && str->len > 1)
			{
				st = (char*)gfpm_bold (_("License:"));
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
							0, st,
							1, (char*)str->str,
							-1);
				g_free (st);
				g_string_free (str, TRUE);
			}

			/* package dependencies */
			temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_DEPENDS);
			str = _pmlist_to_gstring (temp);
			if (str && str->len > 1)
			{
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				st = (char*)gfpm_bold (_("Depends:"));
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
									0, st,
									1, (char*)str->str,
									-1);
				g_free (st);
				g_string_free (str, TRUE);
			}

			/* package groups */
			temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_GROUPS);
			str = _pmlist_to_gstring (temp);
			if (str && str->len > 1)
			{
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				st = (char*)gfpm_bold (_("Group(s):"));
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
									0, st,
									1, (char*)str->str,
									-1);
				g_free (st);
				g_string_free (str, TRUE);
			}

			/* package provides */
			temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_PROVIDES);
			str = _pmlist_to_gstring (temp);
			if (str && str->len)
			{
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				st = (char*)gfpm_bold (_("Provides:"));
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
									0, st,
									1, (char*)str->str,
									-1);
				g_free (st);
				g_string_free (str, TRUE);
			}

			/* package conflicts */
			temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_CONFLICTS);
			str = _pmlist_to_gstring (temp);
			if (str && str->len)
			{
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				st = (char*)gfpm_bold (_("Conflicts:"));
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
									0, st,
									1, (char*)str->str,
									-1);
				g_free (st);
				g_string_free (str, TRUE);
			}

			/* package replaces */
			temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_REPLACES);
			str = _pmlist_to_gstring (temp);
			if (str && str->len)
			{
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				st = (char*)gfpm_bold (_("Replaces:"));
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
									0, st,
									1, (char*)str->str,
							-1);
				g_free (st);
				g_string_free (str, TRUE);
			}

			/* package size */
			char *tmp = NULL;
			float size = 0;
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			size = (float)((long)pacman_pkg_getinfo (pm_pkg, PM_PKG_SIZE)/1024)/1024;
			asprintf (&tmp, "%0.2f MB", size);
			st = (char*)gfpm_bold (_("Size:"));
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								0, st,
								1, (char*)tmp,
								-1);
			g_free (st);
			g_free (tmp);

			/* package packager */
			gtk_list_store_append (GTK_LIST_STORE(model), &iter);
			st = (char*)gfpm_bold (_("Packager:"));
			gtk_list_store_set (GTK_LIST_STORE(model), &iter,
								0, st,
								1, (char*)pacman_pkg_getinfo (pm_pkg, PM_PKG_PACKAGER),
								-1);
			g_free (st);

			/* package required by */
			temp = pacman_pkg_getinfo (pm_pkg, PM_PKG_REQUIREDBY);
			str = _pmlist_to_gstring (temp);
			if (str && str->len)
			{
				gtk_list_store_append (GTK_LIST_STORE(model), &iter);
				st = (char*)gfpm_bold (_("Required By:"));
				gtk_list_store_set (GTK_LIST_STORE(model), &iter,
									0, st,
									1, (char*)str->str,
									-1);
				g_free (st);
				g_string_free (str, TRUE);
			}

			/* release memory */
			pacman_pkg_free (pm_pkg);
		}

		/* release libpacman */
		pacman_release ();
	}

	return ret;
}

/* returns the absolute path of the file */
/* returned string must be freed */
static gchar *
_nautilus_file_info_get_file_path (NautilusFileInfo *fi)
{
	gchar	*ret = NULL;
	GFile	*file = NULL;

	if (fi)
	{
		file = nautilus_file_info_get_location (fi);
		if (file)
		{
			ret = g_file_get_path (file);
		}
	}
	return ret;
}

static gboolean
_validate (GList *files)
{
	NautilusFileInfo	*info = NULL;
	char				*filename = NULL;
	int					filescheme = 0;
	char				*scheme = NULL;
	gboolean			ret = TRUE;

	/* skip if no files or multiple files are selected */
	if (!files || files->next!=NULL)
		return FALSE;

	/* skip if it's a directory */
	info = (NautilusFileInfo*) files->data;
	if (!info || nautilus_file_info_is_directory(info))
		return FALSE;

	/* check the uri scheme of the file */
	scheme = nautilus_file_info_get_uri_scheme (info);
	if (scheme)
	{
		filescheme = strncmp (scheme, "file", 4);
		g_free (scheme);
		/* skip if it's not a local file */
		if (filescheme!=0)
			return FALSE;
	}

	/* now check the mime-type */
	/* fpm archives are nothing but bzip comressed archives with .fpm as extension */
	if (!nautilus_file_info_is_mime_type(info,"application/x-bzip"))
		return FALSE;
	
	/* ok, last one, check the extension */
	filename = _nautilus_file_info_get_file_path (info);
	if (filename)
	{
		if (strncmp(_get_file_ext(filename),"fpm",3))
		{
			ret = FALSE;
		}
		g_free (filename);
	}
	else
	{
		ret = FALSE;
	}

	return ret;
}

static GList *
nautilus_gfpm_property_page_get_pages (NautilusPropertyPageProvider *provider,
										GList *files)
{
	GList   				*pages = NULL;
	NautilusPropertyPage	*page = NULL;
	GtkWidget				*package_widget = NULL;
	char					*filename = NULL;

	/* perform a few checks */
	if (!_validate(files))
		return NULL;

	/* now create and populate our property page with package info */
	package_widget = _create_property_page ();
	filename = _nautilus_file_info_get_file_path ((NautilusFileInfo*)files->data);
	if (_populate_property_page(package_widget,filename))
	{
		gtk_widget_show_all (package_widget);
		page = nautilus_property_page_new ("NautilusGfpm::property_page",
										gtk_label_new(_("Package")),
										package_widget);
		pages = g_list_append (pages, page);
	}
	if (filename)
	{
		g_free (filename);
	}

	return pages;
}

static void
install_callback (NautilusMenuItem *item, gpointer data)
{
	NautilusFileInfo	*info = NULL;
	char				*file = NULL;
	GString				*cmd = NULL;

	info = (NautilusFileInfo*) data;
	file = _nautilus_file_info_get_file_path (info);
	if (file)
	{
		cmd = g_string_new ("sudo /usr/bin/gfpm -A");
		g_string_append_printf (cmd, " \"%s\"", file);
		g_print ("launching command: %s\n", cmd->str);
		g_spawn_command_line_async (cmd->str, NULL);
		g_free (file);
		g_string_free (cmd, FALSE);
	}
	
	return;
}

static GList *
nautilus_gfpm_menu_get_file_items (NautilusMenuProvider *provider,
								GtkWidget *window,
								GList *files)
{
	GList				*items = NULL;
	NautilusMenuItem	*item = NULL;

	if (!_validate(files))
		return NULL;

	item = nautilus_menu_item_new ("NautilusGfpm::menu_item",
									_("Install this package"),
									_("Install this package using GFpm"),
									"gfpm-instfromfile");
	g_signal_connect (item,
					"activate",
					G_CALLBACK(install_callback),
					(gpointer)(files->data));
	items = g_list_append (items, item);

	return items;	
}

static void 
nautilus_gfpm_menu_provider_iface_init (NautilusMenuProviderIface *iface)
{
	iface->get_file_items = nautilus_gfpm_menu_get_file_items;
}

static void 
nautilus_gfpm_property_page_provider_iface_init (NautilusPropertyPageProviderIface *iface)
{
	iface->get_pages = nautilus_gfpm_property_page_get_pages;
}

static void 
nautilus_gfpm_instance_init (NautilusGfpm *ng)
{
}

static void
nautilus_gfpm_class_init (NautilusGfpmClass *klass)
{
	parent_class = g_type_class_peek_parent (klass);
}

static GType gfpm_type = 0;

GType
nautilus_gfpm_get_type (void) 
{
	return gfpm_type;
}

void
nautilus_gfpm_register_type (GTypeModule *module)
{
	static const GTypeInfo info = {
		sizeof (NautilusGfpmClass),
		(GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) nautilus_gfpm_class_init,
		NULL, 
		NULL,
		sizeof (NautilusGfpm),
		0,
		(GInstanceInitFunc) nautilus_gfpm_instance_init,
	};

	/* property page provider interface */
	static const GInterfaceInfo property_page_provider_iface_info = {
		(GInterfaceInitFunc) nautilus_gfpm_property_page_provider_iface_init,
		NULL,
		NULL
	};

	/* menu provider interface */
	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) nautilus_gfpm_menu_provider_iface_init,
		NULL,
		NULL
	};

	gfpm_type = g_type_module_register_type (module,
					         G_TYPE_OBJECT,
					         "NautilusGfpm",
					         &info, 0);

	g_type_module_add_interface (module,
				     gfpm_type,
				     NAUTILUS_TYPE_PROPERTY_PAGE_PROVIDER,
				     &property_page_provider_iface_info);

	g_type_module_add_interface (module,
				     gfpm_type,
				     NAUTILUS_TYPE_MENU_PROVIDER,
				     &menu_provider_iface_info);
}

#endif /* end HAVE_NAUTILUS_EXT */
