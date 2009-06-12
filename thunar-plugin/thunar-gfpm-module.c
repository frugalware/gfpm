/*
 *  thunar-gfpm-module.c for GFpm
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_THUNAR_PLUGIN

#include <exo/exo.h>
#include <thunarx/thunarx.h>
#include <glib/gi18n-lib.h>
#include "thunar-gfpm.h"

static GType type_list[1];

G_MODULE_EXPORT void
thunar_extension_initialize (ThunarxProviderPlugin *plugin)
{
	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	thunar_gfpm_register_type (plugin);
	type_list[0] = THUNAR_TYPE_GFPM;
}


void
thunar_extension_shutdown (void)
{
	return;
}


void 
thunar_extension_list_types (const GType **types, int *num_types)
{	
	*types = type_list;
	*num_types = G_N_ELEMENTS (type_list);

	return;
}

#endif /* end  HAVE_THUNAR_PLUGIN */
