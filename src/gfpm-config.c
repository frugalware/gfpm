/*
 *  gfpm-config.c for gfpm
 *
 *  Copyright (C) 2008 by Priyank Gosalia <priyankmg@gmail.com>
 *  Portions of this code are borrowed from gimmix
 *  gimmix is Copyright (C) 2006-2007 Priyank Gosalia <priyankmg@gmail.com>
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

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include "gfpm-config.h"

#define CONFIG_FILE		".gfpmrc"

static ConfigFile conf;

void
gfpm_config_init (void)
{
	char	*rcfile = NULL;

	cfg_init_config_file_struct (&conf);
	cfg_add_key (&conf, "show_compressed_size", "false");
	cfg_add_key (&conf, "show_uncompressed_size", "false");

	rcfile = cfg_get_path_to_config_file (CONFIG_FILE);
	if (cfg_read_config_file (&conf, rcfile) != 0)
	{
		goto cleanup;
	}
	else
	{
		if (!gfpm_config_get_value_bool("show_compressed_size"))
			cfg_add_key (&conf, "show_compressed_size", "false");
		if (!gfpm_config_get_value_bool("show_uncompressed_size"))
			cfg_add_key (&conf, "show_uncompressed_size", "false");
	}
	cleanup:
	gfpm_config_save ();
	g_free (rcfile);

	return;
}

char *
gfpm_config_get_value_string (const char *key)
{
	char *ret = NULL;

	ret = cfg_get_key_value (conf, (char*)key);

	return ret;
}

gboolean
gfpm_config_get_value_bool (const char *key)
{
	gboolean ret = FALSE;
	
	if (!strcmp(cfg_get_key_value(conf,(char*)key),"true"))
		ret = TRUE;
	
	return ret;
}

int
gfpm_config_get_value_int (const char *key)
{
	int ret = -1;

	ret = atoi (cfg_get_key_value (conf, (char*)key));

	return ret;
}

void
gfpm_config_set_value_string (const char *key, char *value)
{
	cfg_add_key (&conf, (char*)key, value);

	return;
}

void
gfpm_config_set_value_int (const char *key, int value)
{
	char *val = g_strdup_printf ("%d", value);
	cfg_add_key (&conf, (char*)key, val);
	g_free (val);
	
	return;
}

void
gfpm_config_set_value_bool (const char *key, gboolean value)
{
	if (value)
		cfg_add_key (&conf, (char*)key, "true");
	else
		cfg_add_key (&conf, (char*)key, "false");

	return;
}

void
gfpm_config_save (void)
{
	char *rcfile;
	
	rcfile = cfg_get_path_to_config_file (CONFIG_FILE);
	cfg_write_config_file (&conf, rcfile);
	chmod (rcfile, S_IRUSR|S_IWUSR);
	g_free (rcfile);
	
	return;
}

bool
gfpm_config_exists (void)
{
	char *config_file = NULL;
	bool status;
	
	config_file = cfg_get_path_to_config_file (CONFIG_FILE);
	if (g_file_test(config_file, G_FILE_TEST_EXISTS))
		status = true;
	else
		status = false;

	free (config_file);
	return status;
}

void
gfpm_config_free (void)
{
	cfg_free_config_file_struct (&conf);
	
	return;
}

