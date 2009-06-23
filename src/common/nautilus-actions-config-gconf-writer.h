/*
 * Nautilus Actions
 * A Nautilus extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
 * Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009 Pierre Wieser and others (see AUTHORS)
 *
 * This Program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this Library; see the file COPYING.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   Frederic Ruaudel <grumz@grumz.net>
 *   Rodrigo Moya <rodrigo@gnome-db.org>
 *   Pierre Wieser <pwieser@trychlos.org>
 *   ... and many others (see AUTHORS)
 */

#ifndef __NAUTILUS_ACTIONS_CONFIG_GCONF_WRITER_H__
#define __NAUTILUS_ACTIONS_CONFIG_GCONF_WRITER_H__

#include <glib/glist.h>
#include <glib-object.h>
#include <gconf/gconf-client.h>
#include "nautilus-actions-config-gconf.h"

G_BEGIN_DECLS

#define NAUTILUS_ACTIONS_TYPE_CONFIG_GCONF_WRITER            (nautilus_actions_config_gconf_writer_get_type())
#define NAUTILUS_ACTIONS_CONFIG_GCONF_WRITER(obj)            (G_TYPE_CHECK_INSTANCE_CAST (obj, NAUTILUS_ACTIONS_TYPE_CONFIG_GCONF_WRITER, NautilusActionsConfigGconfWriter))
#define NAUTILUS_ACTIONS_CONFIG_GCONF_WRITER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (klass, NAUTILUS_ACTIONS_TYPE_CONFIG_GCONF_WRITER, NautilusActionsConfigGconfWriterClass))
#define NAUTILUS_ACTIONS_IS_CONFIG_GCONF_WRITER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE(obj, NAUTILUS_ACTIONS_TYPE_CONFIG_GCONF_WRITER))
#define NAUTILUS_ACTIONS_IS_CONFIG_GCONF_WRITER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), NAUTILUS_ACTIONS_TYPE_CONFIG_GCONF_WRITER))
#define NAUTILUS_ACTIONS_CONFIG_GCONF_WRITER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), NAUTILUS_ACTIONS_TYPE_CONFIG_GCONF_WRITER, NautilusActionsConfigGconfWriterClass))

typedef struct _NautilusActionsConfigGconfWriter NautilusActionsConfigGconfWriter;
typedef struct _NautilusActionsConfigGconfWriterClass NautilusActionsConfigGconfWriterClass;

struct _NautilusActionsConfigGconfWriter {
	NautilusActionsConfigGconf parent;

	/* Private data, don't access */
};

struct _NautilusActionsConfigGconfWriterClass {
	NautilusActionsConfigGconfClass parent_class;
};

GType                        nautilus_actions_config_gconf_writer_get_type (void);
NautilusActionsConfigGconfWriter       *nautilus_actions_config_gconf_writer_get (void);

G_END_DECLS

#endif /* __NAUTILUS_ACTIONS_CONFIG_GCONF_WRITER_H__ */