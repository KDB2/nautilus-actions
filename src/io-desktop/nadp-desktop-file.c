/*
 * Nautilus Actions
 * A Nautilus extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
 * Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009, 2010 Pierre Wieser and others (see AUTHORS)
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#include <gio/gio.h>

#include <api/na-core-utils.h>

#include "nadp-desktop-file.h"
#include "nadp-keys.h"

/* private class data
 */
struct NadpDesktopFileClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NadpDesktopFilePrivate {
	gboolean   dispose_has_run;
	gchar     *id;
	gchar     *path;
	GKeyFile  *key_file;
};

static GObjectClass *st_parent_class = NULL;

static GType            register_type( void );
static void             class_init( NadpDesktopFileClass *klass );
static void             instance_init( GTypeInstance *instance, gpointer klass );
static void             instance_dispose( GObject *object );
static void             instance_finalize( GObject *object );

static NadpDesktopFile *ndf_new( const gchar *path );
static gchar           *path2id( const gchar *path );
static gboolean         check_key_file( NadpDesktopFile *ndf );
#if 0
static gboolean         ndf_has_entry( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry );
static gboolean         ndf_read_bool( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean default_value );
static gchar           *ndf_read_profile_string( const NadpDesktopFile *ndf, const gchar *profile_id, const gchar *entry );
static GSList          *ndf_read_profile_string_list( const NadpDesktopFile *ndf, const gchar *profile_id, const gchar *entry );
static gchar           *group_name_to_profile_id( const gchar *group_name );
#endif

GType
nadp_desktop_file_get_type( void )
{
	static GType class_type = 0;

	if( !class_type ){
		class_type = register_type();
	}

	return( class_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "nadp_desktop_file_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NadpDesktopFileClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NadpDesktopFile ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( G_TYPE_OBJECT, "NadpDesktopFile", &info, 0 );

	return( type );
}

static void
class_init( NadpDesktopFileClass *klass )
{
	static const gchar *thisfn = "nadp_desktop_file_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NadpDesktopFileClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "nadp_desktop_file_instance_init";
	NadpDesktopFile *self;

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );
	g_return_if_fail( NADP_IS_DESKTOP_FILE( instance ));
	self = NADP_DESKTOP_FILE( instance );

	self->private = g_new0( NadpDesktopFilePrivate, 1 );

	self->private->dispose_has_run = FALSE;
	self->private->key_file = g_key_file_new();
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "nadp_desktop_file_instance_dispose";
	NadpDesktopFile *self;

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));
	g_return_if_fail( NADP_IS_DESKTOP_FILE( object ));
	self = NADP_DESKTOP_FILE( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( object );
		}
	}
}

static void
instance_finalize( GObject *object )
{
	NadpDesktopFile *self;

	g_assert( NADP_IS_DESKTOP_FILE( object ));
	self = NADP_DESKTOP_FILE( object );

	g_free( self->private->id );
	g_free( self->private->path );

	if( self->private->key_file ){
		g_key_file_free( self->private->key_file );
	}

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * nadp_desktop_file_new_from_path:
 * @path: the full pathname of a .desktop file.
 *
 * Retuns: a newly allocated #NadpDesktopFile object.
 *
 * Key file has been loaded, and first validity checks made.
 */
NadpDesktopFile *
nadp_desktop_file_new_from_path( const gchar *path )
{
	static const gchar *thisfn = "nadp_desktop_file_new_from_path";
	NadpDesktopFile *ndf;
	GError *error;

	ndf = NULL;
	g_debug( "%s: path=%s", thisfn, path );
	g_return_val_if_fail( path && g_utf8_strlen( path, -1 ) && g_path_is_absolute( path ), ndf );

	ndf = ndf_new( path );

	error = NULL;
	g_key_file_load_from_file( ndf->private->key_file, path, G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error );
	if( error ){
		g_warning( "%s: %s: %s", thisfn, path, error->message );
		g_error_free( error );
		g_object_unref( ndf );
		return( NULL );
	}

	if( !check_key_file( ndf )){
		g_object_unref( ndf );
		return( NULL );
	}

	return( ndf );
}

/**
 * nadp_desktop_file_new_for_write:
 * @path: the full pathname of a .desktop file.
 *
 * Retuns: a newly allocated #NadpDesktopFile object.
 */
NadpDesktopFile *
nadp_desktop_file_new_for_write( const gchar *path )
{
	static const gchar *thisfn = "nadp_desktop_file_new_for_write";
	NadpDesktopFile *ndf;

	ndf = NULL;
	g_debug( "%s: path=%s", thisfn, path );
	g_return_val_if_fail( path && g_utf8_strlen( path, -1 ) && g_path_is_absolute( path ), ndf );

	ndf = ndf_new( path );

	return( ndf );
}

/**
 * nadp_desktop_file_get_key_file_path:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the full pathname of the key file, as a newly allocated
 * string which should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_key_file_path( const NadpDesktopFile *ndf )
{
	gchar *path;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	path = NULL;

	if( !ndf->private->dispose_has_run ){

		path = g_strdup( ndf->private->path );
	}

	return( path );
}

/*
 * ndf_new:
 * @path: the full pathname of a .desktop file.
 *
 * Retuns: a newly allocated #NadpDesktopFile object.
 */
static NadpDesktopFile *
ndf_new( const gchar *path )
{
	NadpDesktopFile *ndf;

	ndf = g_object_new( NADP_DESKTOP_FILE_TYPE, NULL );

	ndf->private->id = path2id( path );
	ndf->private->path = g_strdup( path );

	return( ndf );
}

/*
 * path2id:
 * @path: a full pathname.
 *
 * Returns: the id of the file, as a newly allocated string which
 * should be g_free() by the caller.
 *
 * The id of the file is equal to the basename, minus the suffix.
 */
static gchar *
path2id( const gchar *path )
{
	gchar *bname;
	gchar *id;

	bname = g_path_get_basename( path );
	id = na_core_utils_str_remove_suffix( bname, NADP_DESKTOP_FILE_SUFFIX );
	g_free( bname );

	return( id );
}

static gboolean
check_key_file( NadpDesktopFile *ndf )
{
	static const gchar *thisfn = "nadp_desktop_file_check_key_file";
	gboolean ret;
	gchar *start_group;

	ret = TRUE;

	/* start group must be 'Desktop Entry' */
	start_group = g_key_file_get_start_group( ndf->private->key_file );
	if( strcmp( start_group, NADP_GROUP_DESKTOP )){

		g_warning( "%s: %s: invalid start group, found %s, waited for %s",
				thisfn, ndf->private->path, start_group, NADP_GROUP_DESKTOP );
		ret = FALSE;
	}

	g_free( start_group );

	return( ret );
}

/**
 * nadp_desktop_file_get_type:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the value for the Type entry as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_file_type( const NadpDesktopFile *ndf )
{
	static const gchar *thisfn = "nadp_desktop_file_get_type";
	gchar *type;
	GError *error;

	type = NULL;
	error = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		type = g_key_file_get_string( ndf->private->key_file, NADP_GROUP_DESKTOP, NADP_KEY_TYPE, &error );

		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );
			g_free( type );
			type = NULL;
		}
	}

	return( type );
}

/**
 * nadp_desktop_file_get_id:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: a newly allocated string which holds the id of the Desktop
 * File.
 */
gchar *
nadp_desktop_file_get_id( const NadpDesktopFile *ndf )
{
	gchar *value;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	value = NULL;

	if( !ndf->private->dispose_has_run ){

		value = g_strdup( ndf->private->id );
	}

	return( value );
}

/**
 * nadp_desktop_file_get_boolean:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 */
gboolean
nadp_desktop_file_get_boolean( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, gboolean default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_boolean";
	gboolean value;
	gboolean read_value;
	gboolean has_entry;
	GError *error;

	value = default_value;
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), FALSE );

	if( !ndf->private->dispose_has_run ){

		error = NULL;
		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_entry ){
			read_value = g_key_file_get_boolean( ndf->private->key_file, group, entry, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );

			} else {
				value = read_value;
				*key_found = TRUE;
			}
		}
	}

	return( value );
}

/**
 * nadp_desktop_file_get_locale_string:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 *
 * Note that g_key_file_has_key doesn't deal correctly with localized
 * strings which have a key[modifier] (it recognizes them as the key
 *  "key[modifier]", not "key")
 */
gchar *
nadp_desktop_file_get_locale_string( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_locale_string";
	gchar *value;
	gchar *read_value;
	GError *error;

	value = g_strdup( default_value );
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		error = NULL;

		read_value = g_key_file_get_locale_string( ndf->private->key_file, group, entry, NULL, &error );
		if( !read_value || error ){
			if( error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );
				g_free( read_value );
			}
		} else {
			g_free( value );
			value = read_value;
			*key_found = TRUE;
		}
	}

	return( value );
}

/**
 * nadp_desktop_file_get_string:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 */
gchar *
nadp_desktop_file_get_string( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_string";
	gchar *value;
	gchar *read_value;
	gboolean has_entry;
	GError *error;

	value = g_strdup( default_value );
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		error = NULL;
		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_entry ){
			read_value = g_key_file_get_string( ndf->private->key_file, group, entry, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );
				g_free( read_value );

			} else {
				g_free( value );
				value = read_value;
				*key_found = TRUE;
			}
		}
	}

	return( value );
}

/**
 * nadp_desktop_file_get_string_list:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 */
GSList *
nadp_desktop_file_get_string_list( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, const gchar *default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_string_list";
	GSList *value;
	gchar **read_array;
	gboolean has_entry;
	GError *error;

	value = g_slist_append( NULL, g_strdup( default_value ));
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		error = NULL;
		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_entry ){
			read_array = g_key_file_get_string_list( ndf->private->key_file, group, entry, NULL, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );

			} else {
				na_core_utils_slist_free( value );
				value = na_core_utils_slist_from_array(( const gchar ** ) read_array );
				*key_found = TRUE;
			}

			g_strfreev( read_array );
		}
	}

	return( value );
}

/**
 * nadp_desktop_file_get_uint:
 * @ndf: this #NadpDesktopFile instance.
 * @group: the searched group.
 * @entry: the searched entry.
 * @key_found: set to %TRUE if the key has been found, to %FALSE else.
 * @default_value: value to be set if key has not been found.
 *
 * Returns: the readen value, or the default value if the entry has not
 * been found in the given group.
 */
guint
nadp_desktop_file_get_uint( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean *key_found, guint default_value )
{
	static const gchar *thisfn = "nadp_desktop_file_get_uint";
	guint value;
	gboolean has_entry;
	GError *error;

	value = default_value;
	*key_found = FALSE;

	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), 0 );

	if( !ndf->private->dispose_has_run ){

		error = NULL;
		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, &error );
		if( error ){
			g_warning( "%s: %s", thisfn, error->message );
			g_error_free( error );

		} else if( has_entry ){
			value = ( guint ) g_key_file_get_integer( ndf->private->key_file, group, entry, &error );
			if( error ){
				g_warning( "%s: %s", thisfn, error->message );
				g_error_free( error );

			} else {
				*key_found = TRUE;
			}
		}
	}

	return( value );
}

#if 0
/**
 * nadp_desktop_file_get_name:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the label of the action, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_name( const NadpDesktopFile *ndf )
{
	gchar *label;

	label = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), label );

	if( !ndf->private->dispose_has_run ){
		label = g_key_file_get_locale_string(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL );

		if( !label ){
			label = g_strdup( "" );
		}
	}

	return( label );
}

/**
 * nadp_desktop_file_get_tooltip:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the tooltip of the action, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_tooltip( const NadpDesktopFile *ndf )
{
	gchar *tooltip;

	tooltip = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), tooltip );

	if( !ndf->private->dispose_has_run ){
		tooltip = g_key_file_get_locale_string(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, NADP_KEY_TOOLTIP, NULL, NULL );

		if( !tooltip ){
			tooltip = g_strdup( "" );
		}
	}

	return( tooltip );
}

/**
 * nadp_desktop_file_get_icon:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the icon of the action, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_icon( const NadpDesktopFile *ndf )
{
	gchar *icon;

	icon = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), icon );

	if( !ndf->private->dispose_has_run ){
		icon = g_key_file_get_locale_string(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, NULL, NULL );

		if( !icon ){
			icon = g_strdup( "" );
		}
	}

	return( icon );
}

/**
 * nadp_desktop_file_get_enabled:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: %TRUE if the action is enabled, %FALSE else.
 *
 * Defaults to TRUE if the key is not specified.
 */
gboolean
nadp_desktop_file_get_enabled( const NadpDesktopFile *ndf )
{
	gboolean enabled;

	enabled = TRUE;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), enabled );

	if( !ndf->private->dispose_has_run ){

		enabled = !ndf_read_bool( ndf, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, FALSE );
	}

	return( enabled );
}

/**
 * nadp_desktop_file_get_items_list:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: The string list value associated to the ItemsList entry.
 * Returns: %NULL if the entry is not found or empty.
 */
GSList *
nadp_desktop_file_get_items_list( const NadpDesktopFile *ndf )
{
	GSList *list;
	gchar **string_list;
	gsize count;

	list = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		string_list = g_key_file_get_string_list(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, NADP_KEY_ITEMS_LIST, &count, NULL );

		if( string_list ){
			list = nadp_utils_to_slist(( const gchar ** ) string_list );
			g_strfreev( string_list );
		}
	}

	return( list );
}

/**
 * nadp_desktop_file_get_target_context:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: %TRUE if the action targets the context menu, %FALSE else.
 *
 * Defaults to TRUE if the key is not specified.
 */
gboolean
nadp_desktop_file_get_target_context( const NadpDesktopFile *ndf )
{
	gboolean target_context;

	target_context = TRUE;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), target_context );

	if( !ndf->private->dispose_has_run ){

		target_context = ndf_read_bool( ndf, G_KEY_FILE_DESKTOP_GROUP, NADP_KEY_TARGET_CONTEXT, TRUE );
	}

	return( target_context );
}

/**
 * nadp_desktop_file_get_target_toolbar:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: %TRUE if the action is target_toolbar, %FALSE else.
 *
 * Defaults to TRUE if the key is not specified.
 */
gboolean
nadp_desktop_file_get_target_toolbar( const NadpDesktopFile *ndf )
{
	gboolean target_toolbar;

	target_toolbar = TRUE;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), target_toolbar );

	if( !ndf->private->dispose_has_run ){

		target_toolbar = ndf_read_bool( ndf, G_KEY_FILE_DESKTOP_GROUP, NADP_KEY_TARGET_TOOLBAR, FALSE );
	}

	return( target_toolbar );
}

/**
 * nadp_desktop_file_get_toolbar_label:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: the label of the action in the toolbar, as a newly allocated
 * string which should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_toolbar_label( const NadpDesktopFile *ndf )
{
	gchar *label;

	label = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), label );

	if( !ndf->private->dispose_has_run ){

		label = g_key_file_get_locale_string(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, NADP_KEY_TOOLBAR_LABEL, NULL, NULL );
	}

	return( label );
}

/**
 * nadp_desktop_file_get_profiles_list:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: The string list value associated to the Profiles entry.
 * Returns: %NULL if the entry is not found or empty.
 */
GSList *
nadp_desktop_file_get_profiles_list( const NadpDesktopFile *ndf )
{
	GSList *list;
	gchar **string_list;
	gsize count;

	list = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		string_list = g_key_file_get_string_list(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, NADP_KEY_PROFILES, &count, NULL );

		if( string_list ){
			list = nadp_utils_to_slist(( const gchar ** ) string_list );
			g_strfreev( string_list );
		}
	}

	return( list );
}

/**
 * nadp_desktop_file_get_profile_group_list:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: The list of profile names extracted from the list of groups.
 * A profile group is identified by its name beggining with X-Action-Profile.
 */
GSList *
nadp_desktop_file_get_profile_group_list( const NadpDesktopFile *ndf )
{
	GSList *groups, *strlist, *is;
	gchar **string_list;
	gchar *profile_id;

	groups = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		string_list = g_key_file_get_groups( ndf->private->key_file, NULL );
		if( string_list ){

			strlist = nadp_utils_to_slist(( const gchar ** ) string_list );
			g_strfreev( string_list );

			for( is = strlist ; is ; is = is->next ){

				profile_id = group_name_to_profile_id(( const gchar * ) is->data );
				if( profile_id && strlen( profile_id )){

					groups = g_slist_prepend( groups, g_strdup( profile_id ));
				}

				g_free( profile_id );
			}

			groups = g_slist_reverse( groups );
			nadp_utils_gslist_free( strlist );
		}
	}

	return( groups );
}

/**
 * nadp_desktop_file_get_profile_name:
 * @ndf: the #NadpDesktopFile instance.
 * @profile_id: the id of the profile to be read.
 *
 * Returns: The name of the profile, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_profile_name( const NadpDesktopFile *ndf, const gchar *profile_id )
{
	gchar *name;

	name = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		name = ndf_read_profile_string( ndf, profile_id, G_KEY_FILE_DESKTOP_KEY_NAME );
		if( !name ){
			name = g_strdup( "" );
		}
	}

	return( name );
}

/**
 * nadp_desktop_file_get_profile_exec:
 * @ndf: the #NadpDesktopFile instance.
 * @profile_id: the id of the profile to be read.
 *
 * Returns: The command to be executed, as a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
nadp_desktop_file_get_profile_exec( const NadpDesktopFile *ndf, const gchar *profile_id )
{
	gchar *command;

	command = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		command = ndf_read_profile_string( ndf, profile_id, G_KEY_FILE_DESKTOP_KEY_EXEC );
		if( !command ){

			command = g_strdup( "" );
		}
	}

	return( command );
}

/**
 * nadp_desktop_file_get_basenames:
 * @ndf: the #NadpDesktopFile instance.
 * @profile_id: the id of the profile to be read.
 *
 * Returns: The list of basenames.
 */
GSList *
nadp_desktop_file_get_basenames( const NadpDesktopFile *ndf, const gchar *profile_id )
{
	GSList *strlist;

	strlist = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		strlist = ndf_read_profile_string_list( ndf, profile_id, NADP_KEY_BASENAMES );
		if( !strlist ){
			strlist = g_slist_append( NULL, g_strdup( "*" ));
		}
	}

	return( strlist );
}

/**
 * nadp_desktop_file_get_matchcase:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Returns: %TRUE if the basenames are case sensitive, %FALSE else.
 *
 * Defaults to TRUE if the key is not specified.
 */
gboolean
nadp_desktop_file_get_matchcase( const NadpDesktopFile *ndf, const gchar *profile_id )
{
	gboolean matchcase;

	matchcase = TRUE;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), matchcase );

	if( !ndf->private->dispose_has_run ){

		matchcase = ndf_read_bool( ndf, G_KEY_FILE_DESKTOP_GROUP, NADP_KEY_TARGET_TOOLBAR, TRUE );
	}

	return( matchcase );
}

/**
 * nadp_desktop_file_get_mimetypes:
 * @ndf: the #NadpDesktopFile instance.
 * @profile_id: the id of the profile to be read.
 *
 * Returns: The list of mimetypes.
 */
GSList *
nadp_desktop_file_get_mimetypes( const NadpDesktopFile *ndf, const gchar *profile_id )
{
	GSList *strlist;

	strlist = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		strlist = ndf_read_profile_string_list( ndf, profile_id, NADP_KEY_MIMETYPES );
		if( !strlist ){
			strlist = g_slist_append( NULL, g_strdup( "*" ));
		}
	}

	return( strlist );
}

/**
 * nadp_desktop_file_get_schemes:
 * @ndf: the #NadpDesktopFile instance.
 * @profile_id: the id of the profile to be read.
 *
 * Returns: The list of schemes.
 */
GSList *
nadp_desktop_file_get_schemes( const NadpDesktopFile *ndf, const gchar *profile_id )
{
	GSList *strlist;

	strlist = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		strlist = ndf_read_profile_string_list( ndf, profile_id, NADP_KEY_SCHEMES );
		if( !strlist ){
			strlist = g_slist_append( NULL, g_strdup( "*" ));
		}
	}

	return( strlist );
}

/**
 * nadp_desktop_file_get_folders:
 * @ndf: the #NadpDesktopFile instance.
 * @profile_id: the id of the profile to be read.
 *
 * Returns: The list of folders.
 */
GSList *
nadp_desktop_file_get_folders( const NadpDesktopFile *ndf, const gchar *profile_id )
{
	GSList *strlist;

	strlist = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), NULL );

	if( !ndf->private->dispose_has_run ){

		strlist = ndf_read_profile_string_list( ndf, profile_id, NADP_KEY_FOLDERS );
		if( !strlist ){
			strlist = g_slist_append( NULL, g_strdup( "/" ));
		}
	}

	return( strlist );
}

#if 0
static gboolean
ndf_has_entry( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry )
{
	gboolean has_entry;

	has_entry = FALSE;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), FALSE );

	if( !ndf->private->dispose_has_run ){

		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, NULL );
	}

	return( has_entry );
}
#endif

static gboolean
ndf_read_bool( const NadpDesktopFile *ndf, const gchar *group, const gchar *entry, gboolean default_value )
{
	gboolean value;
	gboolean has_entry;

	value = default_value;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), value );

	if( !ndf->private->dispose_has_run ){

		has_entry = g_key_file_has_key( ndf->private->key_file, group, entry, NULL );
		if( has_entry ){
			value = g_key_file_get_boolean(
				ndf->private->key_file, group, entry, NULL );
		}
	}

	return( value );
}

static gchar *
ndf_read_profile_string( const NadpDesktopFile *ndf, const gchar *profile_id, const gchar *entry )
{
	gchar *string;
	gchar *group_name;

	group_name = g_strdup_printf( "%s %s", NADP_GROUP_PROFILE, profile_id );

	string = g_key_file_get_locale_string( ndf->private->key_file, group_name, entry, NULL, NULL );

	g_free( group_name );

	return( string );
}

static GSList *
ndf_read_profile_string_list( const NadpDesktopFile *ndf, const gchar *profile_id, const gchar *entry )
{
	GSList *strlist;
	gchar **string_list;
	gchar *group_name;

	strlist = NULL;
	group_name = g_strdup_printf( "%s %s", NADP_GROUP_PROFILE, profile_id );

	string_list = g_key_file_get_string_list( ndf->private->key_file, group_name, entry, NULL, NULL );
	if( string_list ){
		strlist = nadp_utils_to_slist(( const gchar ** ) string_list );
		g_strfreev( string_list );
	}

	g_free( group_name );

	return( strlist );
}

static gchar *
group_name_to_profile_id( const gchar *group_name )
{
	gchar *profile_id;

	profile_id = NULL;

	if( g_str_has_prefix( group_name, NADP_GROUP_PROFILE )){

		gchar **tokens, **iter;
		gchar *tmp;
		gchar *source = g_strdup( group_name );

		tmp = g_strstrip( source );
		if( tmp && g_utf8_strlen( tmp, -1 )){

			tokens = g_strsplit_set( tmp, " ", -1 );
			iter = tokens;
			++iter;
			profile_id = g_strdup( g_strstrip( *iter ));
			g_strfreev( tokens );
		}

		g_free( source );
	}

	return( profile_id );
}

/**
 * nadp_desktop_file_set_label:
 * @ndf: the #NadpDesktopFile instance.
 * @label: the label to be set.
 *
 * Sets the label.
 */
void
nadp_desktop_file_set_label( NadpDesktopFile *ndf, const gchar *label )
{
	char **locales;

	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		locales = ( char ** ) g_get_language_names();
		g_key_file_set_locale_string(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, locales[0], label );
	}
}

/**
 * nadp_desktop_file_set_tooltip:
 * @ndf: the #NadpDesktopFile instance.
 * @tooltip: the tooltip to be set.
 *
 * Sets the tooltip.
 */
void
nadp_desktop_file_set_tooltip( NadpDesktopFile *ndf, const gchar *tooltip )
{
	char **locales;

	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		locales = ( char ** ) g_get_language_names();
		g_key_file_set_locale_string(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, NADP_KEY_TOOLTIP, locales[0], tooltip );
	}
}

/**
 * nadp_desktop_file_set_icon:
 * @ndf: the #NadpDesktopFile instance.
 * @icon: the icon name or path to be set.
 *
 * Sets the icon.
 */
void
nadp_desktop_file_set_icon( NadpDesktopFile *ndf, const gchar *icon )
{
	char **locales;

	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		locales = ( char ** ) g_get_language_names();
		g_key_file_set_locale_string(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, locales[0], icon );
	}
}

/**
 * nadp_desktop_file_set_enabled:
 * @ndf: the #NadpDesktopFile instance.
 * @enabled: whether the action is enabled.
 *
 * Sets the enabled status of the item.
 *
 * Note that the NoDisplay key has an inversed logic regards to enabled
 * status.
 */
void
nadp_desktop_file_set_enabled( NadpDesktopFile *ndf, gboolean enabled )
{
	g_return_if_fail( NADP_IS_DESKTOP_FILE( ndf ));

	if( !ndf->private->dispose_has_run ){

		g_key_file_set_boolean(
				ndf->private->key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, !enabled );
	}
}
#endif

/**
 * nadp_desktop_file_write:
 * @ndf: the #NadpDesktopFile instance.
 *
 * Writes the key file to the disk.
 *
 * Returns: %TRUE if write is ok, %FALSE else.
 */
gboolean
nadp_desktop_file_write( NadpDesktopFile *ndf )
{
	static const gchar *thisfn = "nadp_desktop_file_write";
	gboolean ret;
	gchar *data;
	GFile *file;
	GFileOutputStream *stream;
	GError *error;

	ret = FALSE;
	error = NULL;
	g_return_val_if_fail( NADP_IS_DESKTOP_FILE( ndf ), ret );

	if( !ndf->private->dispose_has_run ){

		data = g_key_file_to_data( ndf->private->key_file, NULL, NULL );
		file = g_file_new_for_path( ndf->private->path );

		stream = g_file_replace( file, NULL, FALSE, G_FILE_CREATE_NONE, NULL, &error );
		if( error ){
			g_warning( "%s: g_file_replace: %s", thisfn, error->message );
			g_error_free( error );
			if( stream ){
				g_object_unref( stream );
			}
			g_object_unref( file );
			g_free( data );
			return( FALSE );
		}

		g_output_stream_write( G_OUTPUT_STREAM( stream ), data, g_utf8_strlen( data, -1 ), NULL, &error );
		if( error ){
			g_warning( "%s: g_output_stream_write: %s", thisfn, error->message );
			g_error_free( error );
			g_object_unref( stream );
			g_object_unref( file );
			g_free( data );
			return( FALSE );
		}

		g_output_stream_close( G_OUTPUT_STREAM( stream ), NULL, &error );
		if( error ){
			g_warning( "%s: g_output_stream_close: %s", thisfn, error->message );
			g_error_free( error );
			g_object_unref( stream );
			g_object_unref( file );
			g_free( data );
			return( FALSE );
		}

		g_object_unref( stream );
		g_object_unref( file );
		g_free( data );
	}

	return( TRUE );
}
