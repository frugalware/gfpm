/*
 *  nautilus-gfpm-module.c for GFpm
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
#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-column-provider.h>
#include <glib/gi18n-lib.h>
#include "nautilus-gfpm.h"

#ifdef HAVE_NAUTILUS_EXT

void
nautilus_module_initialize (GTypeModule *module)
{
	nautilus_gfpm_register_type (module);

	bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
}


void
nautilus_module_shutdown (void)
{
	return;
}


void 
nautilus_module_list_types (const GType **types, int *num_types)
{
	static GType type_list[1];

	type_list[0] = NAUTILUS_TYPE_GFPM;
	*types = type_list;
	*num_types = 1;

	return;
}

#endif /* end  HAVE_NAUTILUS_EXT */
