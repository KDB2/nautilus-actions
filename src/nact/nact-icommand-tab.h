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

#ifndef __NACT_ICOMMAND_TAB_H__
#define __NACT_ICOMMAND_TAB_H__

/*
 * NactICommandTab interface definition.
 *
 * This interface implements the "Nautilus Menu Item" box.
 */

#include "nact-window.h"

G_BEGIN_DECLS

#define NACT_ICOMMAND_TAB_TYPE						( nact_icommand_tab_get_type())
#define NACT_ICOMMAND_TAB( object )					( G_TYPE_CHECK_INSTANCE_CAST( object, NACT_ICOMMAND_TAB_TYPE, NactICommandTab ))
#define NACT_IS_ICOMMAND_TAB( object )				( G_TYPE_CHECK_INSTANCE_TYPE( object, NACT_ICOMMAND_TAB_TYPE ))
#define NACT_ICOMMAND_TAB_GET_INTERFACE( instance )	( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NACT_ICOMMAND_TAB_TYPE, NactICommandTabInterface ))

typedef struct NactICommandTab NactICommandTab;

typedef struct NactICommandTabInterfacePrivate NactICommandTabInterfacePrivate;

typedef struct {
	GTypeInterface                   parent;
	NactICommandTabInterfacePrivate *private;

	/* api */
	GtkWidget *       ( *get_status_bar )    ( NactWindow *window );
	NAActionProfile * ( *get_edited_profile )( NactWindow *window );
	void              ( *field_modified )    ( NactWindow *window );
	void              ( *get_isfiledir )     ( NactWindow *window, gboolean *is_file, gboolean *is_dir );
	gboolean          ( *get_multiple )      ( NactWindow *window );
	GSList *          ( *get_schemes )       ( NactWindow *window );
}
	NactICommandTabInterface;

GType    nact_icommand_tab_get_type( void );

void     nact_icommand_tab_initial_load( NactWindow *window );
void     nact_icommand_tab_runtime_init( NactWindow *window );
void     nact_icommand_tab_all_widgets_showed( NactWindow *window );
void     nact_icommand_tab_dispose( NactWindow *window );

void     nact_icommand_tab_set_profile( NactWindow *window, const NAActionProfile *profile );
gboolean nact_icommand_tab_has_label( NactWindow *window );

G_END_DECLS

#endif /* __NACT_ICOMMAND_TAB_H__ */