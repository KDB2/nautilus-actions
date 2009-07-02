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

#ifndef __NACT_PROFILE_CONDITIONS_EDITOR_H__
#define __NACT_PROFILE_CONDITIONS_EDITOR_H__

/*
 * NactProfileConditionsEditor class definition.
 *
 * This class is derived from NactWindow.
 * It encapsulates the "EditActionDialogExt" widget dialog.
 */

#include "nact-window.h"

G_BEGIN_DECLS

#define NACT_PROFILE_CONDITIONS_EDITOR_TYPE					( nact_profile_conditions_editor_get_type())
#define NACT_PROFILE_CONDITIONS_EDITOR( object )			( G_TYPE_CHECK_INSTANCE_CAST( object, NACT_PROFILE_CONDITIONS_EDITOR_TYPE, NactProfileConditionsEditor ))
#define NACT_PROFILE_CONDITIONS_EDITOR_CLASS( klass )		( G_TYPE_CHECK_CLASS_CAST( klass, NACT_PROFILE_CONDITIONS_EDITOR_TYPE, NactProfileConditionsEditorClass ))
#define NACT_IS_PROFILE_CONDITIONS_EDITOR( object )			( G_TYPE_CHECK_INSTANCE_TYPE( object, NACT_PROFILE_CONDITIONS_EDITOR_TYPE ))
#define NACT_IS_PROFILE_CONDITIONS_EDITOR_CLASS( klass )	( G_TYPE_CHECK_CLASS_TYPE(( klass ), NACT_PROFILE_CONDITIONS_EDITOR_TYPE ))
#define NACT_PROFILE_CONDITIONS_EDITOR_GET_CLASS( object )	( G_TYPE_INSTANCE_GET_CLASS(( object ), NACT_PROFILE_CONDITIONS_EDITOR_TYPE, NactProfileConditionsEditorClass ))

typedef struct NactProfileConditionsEditorPrivate NactProfileConditionsEditorPrivate;

typedef struct {
	NactWindow                          parent;
	NactProfileConditionsEditorPrivate *private;
}
	NactProfileConditionsEditor;

typedef struct NactProfileConditionsEditorClassPrivate NactProfileConditionsEditorClassPrivate;

typedef struct {
	NactWindowClass                          parent;
	NactProfileConditionsEditorClassPrivate *private;
}
	NactProfileConditionsEditorClass;

GType    nact_profile_conditions_editor_get_type( void );

gboolean nact_profile_conditions_editor_run_editor( NactWindow *parent, gpointer user_data );

G_END_DECLS

#endif /* __NACT_PROFILE_CONDITIONS_EDITOR_H__ */