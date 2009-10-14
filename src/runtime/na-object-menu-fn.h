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

#ifndef __NA_RUNTIME_OBJECT_MENU_FN_H__
#define __NA_RUNTIME_OBJECT_MENU_FN_H__

/**
 * SECTION: na_object_menu
 * @short_description: #NAObjectMenu class definition.
 * @include: runtime/na-object-menu.h
 *
 * This is a menu. It embeds other menus and/or actions.
 */

#include "na-object-menu-class.h"

G_BEGIN_DECLS

/* i18n: default label for a newly created menu */
#define NA_OBJECT_MENU_DEFAULT_LABEL	_( "New Nautilus menu" )

NAObjectMenu *na_object_menu_new( void );

G_END_DECLS

#endif /* __NA_RUNTIME_OBJECT_MENU_FN_H__ */