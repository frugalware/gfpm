/***************************************************************************
 *  gfpm-interface.c
 *  Author: Priyank Gosalia <priyankmg@gmail.com>	
 *  Copyright 2006-2007 Frugalware Developer Team
 ****************************************************************************/

/*
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
#include "gfpm.h"
#include "gfpm-messages.h"

void
gfpm_error (const char *error_str)
{
	if (!strlen(error_str))
		return;
	fprintf (stderr, "\n\033[1;31m%s ==>\033[0;1m %s\n", _("ERROR"), _(error_str));

	return;
}

void
gfpm_message (const char *message_str)
{

}


