/***************************************************************************
 *  gfpm-progress.c
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
#include <glade/glade.h>
#include "gfpm-progress.h"

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

extern GladeXML *xml;

GtkProgressBar	*progressbar = NULL;
GtkWidget		*progresswindow = NULL;
GtkWidget		*main_label = NULL;
GtkWidget		*sub_label = NULL;

float rate;
int offset;

void
gfpm_progress_init (void)
{
	alpm_parse_config ("/etc/pacman.conf", NULL, "");
	alpm_set_option (PM_OPT_DLCB, (long)gfpm_progress_update);
	alpm_set_option (PM_OPT_DLOFFSET, (long)&offset);
	alpm_set_option (PM_OPT_DLRATE, (long)&rate);

	progressbar = GTK_PROGRESS_BAR(glade_xml_get_widget (xml, "progressbar1"));
	progresswindow = glade_xml_get_widget (xml, "progresswindow");
	main_label = glade_xml_get_widget (xml, "main_pr_label");
	sub_label = glade_xml_get_widget (xml, "sub_pr_label");

	return;
}

void
gfpm_progress_show (gboolean show)
{
	if (show == TRUE)
		gtk_widget_show (progresswindow);
	else
		gtk_widget_hide (progresswindow);

	return;
}

int
gfpm_progress_update (netbuf *ctl, int xferred, void *arg)
{
	int		size;
	int		per;
	char	text[6];

	ctl = NULL;
	size = *(int*)arg;
	per = ((float)(xferred+offset) / size) * 100;
	sprintf (text, "%d %%", per);
	gtk_progress_bar_set_text (progressbar, text);
	gtk_progress_bar_set_fraction (progressbar, (float)per/100);

	return 1;
}

void
gfpm_progress_set_main_text (const char *msg)
{
	if (msg != NULL)
		gtk_label_set_text (GTK_LABEL(main_label), msg);

	return;
}

void
gfpm_progress_set_sub_text (const char *msg)
{
	if (msg != NULL)
		gtk_label_set_text (GTK_LABEL(sub_label), msg);

	return;
}

