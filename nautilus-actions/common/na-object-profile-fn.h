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

#ifndef __NA_COMMON_OBJECT_PROFILE_FN_H__
#define __NA_COMMON_OBJECT_PROFILE_FN_H__

/**
 * SECTION: na_object_profile
 * @short_description: #NAObjectProfile public function declarations extension.
 * @include: common/na-object-profile-fn.h
 *
 * Define here the public functions of the #NAObjectProfile class which
 * are not shared by the Nautilus Actions plugin.
 */

#include <private/na-object-profile-class.h>

G_BEGIN_DECLS

void    na_object_profile_replace_folder_uri( NAObjectProfile *profile, const gchar *old, const gchar *new );

void    na_object_profile_set_scheme( NAObjectProfile *profile, const gchar *scheme, gboolean selected );

G_END_DECLS

#endif /* __NA_COMMON_OBJECT_PROFILE_FN_H__ */
