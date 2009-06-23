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

#ifndef __NA_OBJECT_H__
#define __NA_OBJECT_H__

/*
 * NAObject class definition.
 *
 * This is the base class for NactAction and NactActionProfile.
 *
 * It takes care of common things such as ids, i/o, etc.
 * It uses the NactIIOProviderInterface for all storage subsystems
 * management.
 */

#include <glib-object.h>

G_BEGIN_DECLS

#define NA_OBJECT_TYPE					( na_object_get_type())
#define NA_OBJECT( object )				( G_TYPE_CHECK_INSTANCE_CAST( object, NA_OBJECT_TYPE, NAObject ))
#define NA_OBJECT_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NA_OBJECT_TYPE, NAObjectClass ))
#define NA_IS_OBJECT( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NA_OBJECT_TYPE ))
#define NA_IS_OBJECT_CLASS( klass )		( G_TYPE_CHECK_CLASS_TYPE(( klass ), NA_OBJECT_TYPE ))
#define NA_OBJECT_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NA_OBJECT_TYPE, NAObjectClass ))

typedef struct NAObjectPrivate NAObjectPrivate;

typedef struct {
	GObject          parent;
	NAObjectPrivate *private;
}
	NAObject;

typedef struct NAObjectClassPrivate NAObjectClassPrivate;

typedef struct {
	GObjectClass          parent;
	NAObjectClassPrivate *private;

	/* virtual public functions */
	void    ( *dump )     ( const NAObject *object );
	gchar * ( *get_id )   ( const NAObject *object );
	gchar * ( *get_label )( const NAObject *object );
}
	NAObjectClass;

GType    na_object_get_type( void );

void     na_object_dump( const NAObject *object );
gchar   *na_object_get_id( const NAObject *object );
gchar   *na_object_get_label( const NAObject *object );

G_END_DECLS

#endif /* __NA_OBJECT_H__ */