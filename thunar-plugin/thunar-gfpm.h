#ifndef THUNAR_GFPM_H
#define THUNAR_GFPM_H

#include <glib-object.h>

G_BEGIN_DECLS

#define THUNAR_TYPE_GFPM  (thunar_gfpm_get_type ())
#define THUNAR_GFPM(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), THUNAR_TYPE_GFPM, NautilusGfpm))
#define THUNAR_IS_GFPM(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), THUNAR_TYPE_GFPM))

typedef struct _ThunarGfpm      ThunarGfpm;
typedef struct _ThunarGfpmClass ThunarGfpmClass;

struct _ThunarGfpm {
	GObject __parent;
};

struct _ThunarGfpmClass {
	GObjectClass __parent;
};

GType thunar_gfpm_get_type      (void);
void  thunar_gfpm_register_type (ThunarxProviderPlugin *module);

G_END_DECLS

#endif


