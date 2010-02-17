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

#include <api/na-gconf-utils.h>

#include "nact-application.h"
#include "nact-iprefs.h"

/* private interface data
 */
struct NactIPrefsInterfacePrivate {
	GConfClient *client;
};

#define DEFAULT_IMPORT_MODE_INT				IPREFS_IMPORT_NO_IMPORT
#define DEFAULT_IMPORT_MODE_STR				"NoImport"

static GConfEnumStringPair import_mode_table[] = {
	{ IPREFS_IMPORT_NO_IMPORT,				DEFAULT_IMPORT_MODE_STR },
	{ IPREFS_IMPORT_RENUMBER,				"Renumber" },
	{ IPREFS_IMPORT_OVERRIDE,				"Override" },
	{ IPREFS_IMPORT_ASK,					"Ask" },
	{ 0, NULL }
};

static gboolean st_initialized = FALSE;
static gboolean st_finalized = FALSE;

static GType       register_type( void );
static void        interface_base_init( NactIPrefsInterface *klass );
static void        interface_base_finalize( NactIPrefsInterface *klass );

static GConfValue *get_value( GConfClient *client, const gchar *path, const gchar *entry );
static void        set_value( GConfClient *client, const gchar *path, const gchar *entry, GConfValue *value );

GType
nact_iprefs_get_type( void )
{
	static GType iface_type = 0;

	if( !iface_type ){
		iface_type = register_type();
	}

	return( iface_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "nact_iprefs_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NactIPrefsInterface ),
		( GBaseInitFunc ) interface_base_init,
		( GBaseFinalizeFunc ) interface_base_finalize,
		NULL,
		NULL,
		NULL,
		0,
		0,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( NA_IPREFS_TYPE, "NactIPrefs", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NactIPrefsInterface *klass )
{
	static const gchar *thisfn = "nact_iprefs_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NactIPrefsInterfacePrivate, 1 );

		klass->private->client = gconf_client_get_default();

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NactIPrefsInterface *klass )
{
	static const gchar *thisfn = "nact_iprefs_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_free( klass->private );
	}
}

/**
 * nact_iprefs_get_export_format:
 * @window: this #BaseWindow-derived window.
 * @name: name of the export format key to be readen
 *
 * Returns: the export format currently set as a #GQuark.
 *
 * Defaults to exporting as a GConfEntry (see. #nact-iprefs.h)
 *
 * Note: please take care of keeping the default value synchronized with
 * those defined in schemas.
 */
GQuark
nact_iprefs_get_export_format( const BaseWindow *window, const gchar *name )
{
	GQuark export_format;
	NactApplication *application;
	NAUpdater *updater;
	gchar *format_str;

	export_format = g_quark_from_static_string( IPREFS_EXPORT_FORMAT_DEFAULT );

	g_return_val_if_fail( BASE_IS_WINDOW( window ), export_format );

	if( st_initialized && !st_finalized ){

		application = NACT_APPLICATION( base_window_get_application( window ));
		updater = nact_application_get_updater( application );

		format_str = na_iprefs_read_string(
				NA_IPREFS( updater ),
				name,
				IPREFS_EXPORT_FORMAT_DEFAULT );

		export_format = g_quark_from_string( format_str );

		g_free( format_str );
	}

	return( export_format );
}

/**
 * nact_iprefs_set_export_format:
 * @window: this #BaseWindow-derived window.
 * @format: the new value to be written.
 *
 * Writes the preferred export format' to the GConf preference system.
 */
void
nact_iprefs_set_export_format( const BaseWindow *window, const gchar *name, GQuark format )
{
	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( st_initialized && !st_finalized ){

		nact_iprefs_write_string(
				window,
				name,
				g_quark_to_string( format ));
	}
}

/**
 * nact_iprefs_get_import_mode:
 * @instance: this #NAIPrefs interface instance.
 * @name: name of the import key to be readen
 *
 * Returns: the import mode currently set.
 *
 * Note: this function returns a suitable default value even if the key
 * is not found in GConf preferences or no schema has been installed.
 *
 * Note: please take care of keeping the default value synchronized with
 * those defined in schemas.
 */
gint
nact_iprefs_get_import_mode( const BaseWindow *window, const gchar *name )
{
	gint import_mode = DEFAULT_IMPORT_MODE_INT;
	gint import_int;
	gchar *import_str;
	NactApplication *application;
	NAUpdater *updater;

	g_return_val_if_fail( BASE_IS_WINDOW( window ), DEFAULT_IMPORT_MODE_INT );

	if( st_initialized && !st_finalized ){

		application = NACT_APPLICATION( base_window_get_application( window ));
		updater = nact_application_get_updater( application );

		import_str = na_iprefs_read_string(
				NA_IPREFS( updater ),
				name,
				DEFAULT_IMPORT_MODE_STR );

		if( gconf_string_to_enum( import_mode_table, import_str, &import_int )){
			import_mode = import_int;
		}

		g_free( import_str );
	}

	return( import_mode );
}

/**
 * nact_iprefs_set_import_mode:
 * @instance: this #NAIPrefs interface instance.
 * @mode: the new value to be written.
 *
 * Writes the current status of 'import mode' to the GConf
 * preference system.
 */
void
nact_iprefs_set_import_mode( const BaseWindow *window, const gchar *name, gint mode )
{
	const gchar *import_str;

	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( st_initialized && !st_finalized ){

		import_str = gconf_enum_to_string( import_mode_table, mode );

		nact_iprefs_write_string(
				window,
				name,
				import_str ? import_str : DEFAULT_IMPORT_MODE_STR );
	}
}

/**
 * nact_iprefs_migrate_key:
 * @window: a #BaseWindow window.
 * @old_key: the old preference entry.
 * @new_key: the new preference entry.
 *
 * Migrates the content of an entry from an obsoleted key to a new one.
 * Removes the old key, along with the schema associated to it,
 * considering that the version which asks for this migration has
 * installed a schema corresponding to the new key.
 */
void
nact_iprefs_migrate_key( const BaseWindow *window, const gchar *old_key, const gchar *new_key )
{
	static const gchar *thisfn = "nact_iprefs_migrate_key";
	GConfClient *gconf_client;
	GConfValue *value;

	g_debug( "%s: window=%p, old_key=%s, new_key=%s", thisfn, ( void * ) window, old_key, new_key );
	g_return_if_fail( BASE_IS_WINDOW( window ));

	if( st_initialized && !st_finalized ){

		gconf_client = NACT_IPREFS_GET_INTERFACE( window )->private->client;

		value = get_value( gconf_client, IPREFS_GCONF_PREFS_PATH, new_key );
		if( !value ){
			value = get_value( gconf_client, IPREFS_GCONF_PREFS_PATH, old_key );
			if( value ){
				set_value( gconf_client, IPREFS_GCONF_PREFS_PATH, new_key, value );
				gconf_value_free( value );
			}
		}

		/* do not remove entries which may still be used by an older N-A version
		 */
	}
}

/**
 * nact_iprefs_write_bool:
 * @window: this #BaseWindow-derived window.
 * @name: the preference entry.
 * @value: the value to be written.
 *
 * Writes the given boolean value.
 */
void
nact_iprefs_write_bool( const BaseWindow *window, const gchar *name, gboolean value )
{
	gchar *path;

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( NACT_IS_IPREFS( window ));

	if( st_initialized && !st_finalized ){

		path = gconf_concat_dir_and_key( IPREFS_GCONF_PREFS_PATH, name );
		na_gconf_utils_write_bool( NACT_IPREFS_GET_INTERFACE( window )->private->client, path, value, NULL );
		g_free( path );
	}
}

/**
 * nact_iprefs_write_string:
 * @window: this #BaseWindow-derived window.
 * @name: the preference key.
 * @value: the value to be written.
 *
 * Writes the value as the given GConf preference.
 */
void
nact_iprefs_write_string( const BaseWindow *window, const gchar *name, const gchar *value )
{
	gchar *path;

	g_return_if_fail( BASE_IS_WINDOW( window ));
	g_return_if_fail( NACT_IS_IPREFS( window ));

	if( st_initialized && !st_finalized ){

		path = gconf_concat_dir_and_key( IPREFS_GCONF_PREFS_PATH, name );

		na_gconf_utils_write_string( NACT_IPREFS_GET_INTERFACE( window )->private->client, path, value, NULL );

		g_free( path );
	}
}

static GConfValue *
get_value( GConfClient *client, const gchar *path, const gchar *entry )
{
	static const gchar *thisfn = "na_iprefs_get_value";
	GError *error = NULL;
	gchar *fullpath;
	GConfValue *value;

	fullpath = gconf_concat_dir_and_key( path, entry );

	value = gconf_client_get_without_default( client, fullpath, &error );

	if( error ){
		g_warning( "%s: key=%s, %s", thisfn, fullpath, error->message );
		g_error_free( error );
		if( value ){
			gconf_value_free( value );
			value = NULL;
		}
	}

	g_free( fullpath );

	return( value );
}

static void
set_value( GConfClient *client, const gchar *path, const gchar *entry, GConfValue *value )
{
	static const gchar *thisfn = "na_iprefs_set_value";
	GError *error = NULL;
	gchar *fullpath;

	g_return_if_fail( value );

	fullpath = gconf_concat_dir_and_key( path, entry );

	gconf_client_set( client, fullpath, value, &error );

	if( error ){
		g_warning( "%s: key=%s, %s", thisfn, fullpath, error->message );
		g_error_free( error );
	}

	g_free( fullpath );
}