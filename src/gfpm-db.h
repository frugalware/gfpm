#ifndef __GFPM_DB_H__
#define __GFPM_DB_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <pacman.h>
#include <gtk/gtk.h>

#define FW_CURRENT	"frugalware-current"
#define FW_STABLE	"frugalware"
#define FW_LOCAL	"local"
#define CFG_FILE	"/etc/pacman-g2.conf"

void gfpm_db_register (const char *);
int gfpm_db_init (void);
void gfpm_db_reset_localdb (void);
void gfpm_db_cleanup (void);
int gfpm_db_populate_repolist (void);
GList *gfpm_db_get_repolist (void);

#endif

