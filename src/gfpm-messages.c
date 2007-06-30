/*
 *  gfpm-messages.c for gfpm
 *
 *  This code is borrowed from gnetconfig.
 *  Copyright (C) 2006-2007 by Priyank Gosalia <priyankmg@gmail.com>
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

#include "gfpm-messages.h"

extern GtkWidget *gfpm_mw;

void
gfpm_error (const char *error_str)
{
	GtkWidget *error_dlg = NULL;

	if (!strlen(error_str))
		return;

	error_dlg = gtk_message_dialog_new (GTK_WINDOW(gfpm_mw),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s",
					error_str);
	gtk_window_set_resizable (GTK_WINDOW(error_dlg), FALSE);
	gtk_dialog_run (GTK_DIALOG(error_dlg));
	gtk_widget_destroy (error_dlg);

	return;
}

void
gfpm_message (const char *message_str)
{
	GtkWidget *message_dlg;

	message_dlg = gtk_message_dialog_new (GTK_WINDOW(gfpm_mw),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_INFO,
					GTK_BUTTONS_OK,
					"%s",
					message_str);
	gtk_window_set_resizable (GTK_WINDOW(message_dlg), FALSE);
	gtk_dialog_run (GTK_DIALOG(message_dlg));
	gtk_widget_destroy (message_dlg);

	return;
}

gint
gfpm_question (const char *message_str)
{
	GtkWidget 	*dialog;
	gint 		ret;

	dialog = gtk_message_dialog_new (GTK_WINDOW(gfpm_mw),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					"%s",
					message_str);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE), 
	ret = gtk_dialog_run (GTK_DIALOG(dialog));
	gtk_widget_destroy (dialog);

	return ret;
}

char *
gfpm_input (const char *title, const char *message, int *res)
{
	GtkWidget	*dialog;
	GtkWidget	*entry;
	GtkWidget	*label;
	char		*ret = NULL;

	ret = NULL;

	dialog = gtk_dialog_new_with_buttons (title,
						GTK_WINDOW(gfpm_mw),
						GTK_DIALOG_DESTROY_WITH_PARENT,
						GTK_STOCK_OK,
                                         	GTK_RESPONSE_ACCEPT,
                                         	GTK_STOCK_CANCEL,
                                         	GTK_RESPONSE_REJECT,
                                         	NULL);
	gtk_window_set_resizable (GTK_WINDOW(dialog), FALSE);
	label = gtk_label_new (message);
	entry = gtk_entry_new ();

	gtk_misc_set_padding (GTK_MISC(label), 5, 5);
	gtk_dialog_set_has_separator (GTK_DIALOG(dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER((GTK_DIALOG(dialog))->vbox), 10);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);
	gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), entry);
	gtk_widget_show_all (GTK_WIDGET(dialog));
	*res = gtk_dialog_run (GTK_DIALOG(dialog));
	ret = (char*)g_strdup(gtk_entry_get_text (GTK_ENTRY(entry)));
	gtk_widget_destroy (GTK_WIDGET(dialog));

	return ret;
}

