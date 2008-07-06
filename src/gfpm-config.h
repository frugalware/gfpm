#ifndef _GFPM_CONFIG_H_
#define _GFPM_CONFIG_H

#include <stdio.h>
#include <stdbool.h>
#include <glib.h>
#include "wejpconfig.h"

/* Returns true if file exists otherwise false */
bool gfpm_config_exists (void);

/* Parse config file and set the initial config values */
void gfpm_config_init (void);

/* Save current settings back to gfpmrc */
void gfpm_config_save (void);

/* Read a value from gfpmrc and return it as a string */
char * gfpm_config_get_value_string (const char *);

/* Read a value from gfpmrc and return it as a gboolean */
gboolean gfpm_config_get_value_bool (const char *);

/* Read a value from gfpmrc and return it as an int */
int gfpm_config_get_value_int (const char *);

/* Set a value for a particular key in gfpmrc (int) */
void gfpm_config_set_value_int (const char *, int);

/* Set a value for a particular key in gfpmrc (string) */
void gfpm_config_set_value_string (const char *, char *);

/* Set a value for a particular key in gfpmrc (gboolean) */
void gfpm_config_set_value_bool (const char *, gboolean);

/* Free conf */
void gfpm_config_free (void);

#endif
