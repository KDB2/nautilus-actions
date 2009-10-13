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

#ifndef __NA_COMMON_OBJECT_ID_FN_H__
#define __NA_COMMON_OBJECT_ID_FN_H__

/**
 * SECTION: na_object_id
 * @short_description: #NAObjectId public function declarations extension.
 * @include: common/na-object-id-fn.h
 *
 * Define here the public functions of the #NAObjectId class which are
 * not shared by the Nautilus Actions plugin.
 *
 * Note that most users of the class should rather use macros defined
 * in na-object-api.h
 */

#include <runtime/na-object-action-class.h>
#include <runtime/na-pivot.h>

G_BEGIN_DECLS

void        na_object_id_check_status_up( NAObjectId *object );
NAObjectId *na_object_id_get_topmost_parent( NAObjectId *object );
void        na_object_id_prepare_for_paste( NAObjectId *object, NAPivot *pivot, gboolean renumber, NAObjectAction *action );
void        na_object_id_set_copy_of_label( NAObjectId *object );

G_END_DECLS

#endif /* __NA_COMMON_OBJECT_ID_FN_H__ */
