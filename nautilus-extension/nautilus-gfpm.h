#ifndef NAUTILUS_GFPM_H
#define NAUTILUS_GFPM_H

#include <glib-object.h>

G_BEGIN_DECLS

#define NAUTILUS_TYPE_GFPM  (nautilus_gfpm_get_type ())
#define NAUTILUS_GFPM(o)    (G_TYPE_CHECK_INSTANCE_CAST ((o), NAUTILUS_TYPE_GFPM, NautilusGfpm))
#define NAUTILUS_IS_GFPM(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), NAUTILUS_TYPE_GFPM))

typedef struct _NautilusGfpm      NautilusGfpm;
typedef struct _NautilusGfpmClass NautilusGfpmClass;

struct _NautilusGfpm {
	GObject __parent;
};

struct _NautilusGfpmClass {
	GObjectClass __parent;
};

GType nautilus_gfpm_get_type      (void);
void  nautilus_gfpm_register_type (GTypeModule *module);

G_END_DECLS

#endif


