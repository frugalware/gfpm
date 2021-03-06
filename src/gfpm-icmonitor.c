/*
 *  gfpm-icmonitor.c for gfpm
 *  Gfpm Icon cache monitor
 *
 *  Copyright (C) 2006-2009 by Priyank Gosalia <priyankmg@gmail.com>
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

#ifdef HAVE_ICMONITOR

#include "gfpm-icmonitor.h"
#include <inotifytools/inotifytools.h>
#include <inotifytools/inotify.h>

#define MON_DIR	"/usr/share/icons/hicolor"

static gboolean changed;
static gboolean inited;
static gboolean stop = FALSE;
struct inotify_event *event;

//static gboolean gfpm_icmonitor (void);

int
gfpm_icmonitor_init (void)
{
	if (!inotifytools_initialize())
	{
		inited = FALSE;
		return -1;
	}

	if (!inotifytools_watch_recursively(MON_DIR, IN_ALL_EVENTS))
	{
		inited = FALSE;
		return -1;
	}

	inited = TRUE;

	return 0;
}

/* execute the monitor function every 0.5 second to monitor changes
   in the MON_DIR 
*/
static gboolean
gfpm_icmonitor_monitor (void)
{
	if (stop == TRUE)
	{
		return FALSE;
	}
	if (changed)
	{
		return TRUE;
	}
	event = inotifytools_next_event (0);
	while (event)
	{
		switch (event->mask)
		{
			case IN_ALL_EVENTS:
			case IN_ACCESS:
			case IN_CREATE:
			case IN_DELETE:
			case IN_MODIFY:
			{
				changed = TRUE;
				stop = TRUE;
				return FALSE;
				break;
			}
			default:
			{
				break;
			}
		}
		event = inotifytools_next_event (0);
	}

	return TRUE;
}

void
gfpm_icmonitor_start_monitor (void)
{	
	if (inited)
	{
		if (changed)
			return;	
		stop = FALSE;
		/* start the timer */
		g_timeout_add (100, (GSourceFunc)gfpm_icmonitor_monitor, NULL);
	}

	return;
}

void
gfpm_icmonitor_stop_monitor (void)
{	
	stop = TRUE;
}

void
gfpm_icmonitor_reset_ic (void)
{
	changed = FALSE;
}

gboolean
gfpm_icmonitor_is_ic_changed (void)
{
	if (inited)
		return changed;

	return FALSE;
}

gboolean
gfpm_icmonitor_is_running (void)
{
	if (stop == TRUE)
		return FALSE;
	
	return TRUE;
}

#endif

