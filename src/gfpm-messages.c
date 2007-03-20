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
gfpm_error (const char *error_str, GfpmErrorType type)
{
	if (!strlen(error_str))
		return;
	if (type == GFPM_ERROR_STDOUT)
	{
		fprintf (stderr, "\n\033[1;31m%s ==>\033[0;1m %s\n", _("ERROR"), error_str);
	}
	else if (type == GFPM_ERROR_GUI)
	{
		GtkWidget *error_dlg;

		error_dlg = gtk_message_dialog_new (NULL, 							GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_MESSAGE_ERROR,
						GTK_BUTTONS_OK,
						"%s: %s",
						_("ERROR"),
						error_str);
		gtk_window_set_resizable (GTK_WINDOW(error_dlg), FALSE);
		g_signal_connect (error_dlg,	"response", G_CALLBACK (gtk_widget_destroy), error_dlg);
	
		gtk_widget_show_all (error_dlg);
	}
	return;
}

void
gfpm_message (const char *message_str)
{

}


