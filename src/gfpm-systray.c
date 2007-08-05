/*
 *  gfpm-systray.c for gfpm
 *
 *  Copyright (C) 2007 by Priyank Gosalia <priyankmg@gmail.com>
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
 
#define _GNU_SOURCE
#include <time.h>
#include <sys/time.h>
#include <glade/glade.h>
#include "gfpm-systray.h"

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

GtkStatusIcon *gfpm_statusicon;

void
gfpm_systray_init (void)
{
	gfpm_statusicon = gtk_status_icon_new_from_icon_name ("gfpm");
	gtk_status_icon_set_visible (gfpm_statusicon, FALSE);

	return;
}

void
gfpm_systray_set_visible (gboolean visible)
{
	gtk_status_icon_set_visible (gfpm_statusicon, FALSE);

	return;
}
