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

#include <api/na-core-utils.h>
#include <api/na-gconf-utils.h>

static GConfValue *read_value( GConfClient *gconf, const gchar *path, gboolean use_schema, GConfValueType type );
static gboolean    sync_gconf( GConfClient *gconf, gchar **message );

/**
 * na_gconf_utils_get_subdirs:
 * @gconf: a  #GConfClient instance.
 * @path: a full path to be readen.
 *
 * Returns: a list of full path subdirectories.
 *
 * The returned list should be #na_gconf_utils_free_subdirs() by the caller.
 */
GSList *
na_gconf_utils_get_subdirs( GConfClient *gconf, const gchar *path )
{
	static const gchar *thisfn = "na_gconf_utils_get_subdirs";
	GError *error = NULL;
	GSList *list_subdirs;

	list_subdirs = gconf_client_all_dirs( gconf, path, &error );

	if( error ){
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
		return(( GSList * ) NULL );
	}

	return( list_subdirs );
}

/**
 * na_gconf_utils_free_subdirs:
 * @subdirs: the subdirectory list as returned from #na_gconf_utils_get_subdirs().
 *
 * Release the list.
 */
void
na_gconf_utils_free_subdirs( GSList *subdirs )
{
	na_core_utils_slist_free( subdirs );
}

/**
 * na_gconf_utils_has_entry:
 * @gconf: a  #GConfClient instance.
 * @path: the full path of a key.
 * @entry: the entry to be tested.
 *
 * Returns: %TRUE if the given @entry exists for the given @path,
 * %FALSE else.
 */
gboolean
na_gconf_utils_has_entry( GConfClient *gconf, const gchar *path, const gchar *entry )
{
	static const gchar *thisfn = "na_gconf_utils_has_entry";
	gboolean have_entry = FALSE;
	GError *error = NULL;
	gchar *key;
	GConfValue *value;

	key = g_strdup_printf( "%s/%s", path, entry );

	value = gconf_client_get_without_default( gconf, key, &error );

	if( error ){
		g_warning( "%s: key=%s, error=%s", thisfn, key, error->message );
		g_error_free( error );
		if( value ){
			gconf_value_free( value );
			value = NULL;
		}
	}

	if( value ){
		have_entry = TRUE;
		gconf_value_free( value );
	}

	g_free( key );

	return( have_entry );
}

/**
 * na_gconf_utils_get_entries:
 * @gconf: a  #GConfClient instance.
 * @path: a full path to be readen.
 *
 * Loads all the key=value pairs of the specified key.
 *
 * Returns: a list of #GConfEntry.
 *
 * The returned list is not recursive : it contains only the immediate
 * children of @path. To free the returned list, call
 * na_gconf_utils_free_entries().
 */
GSList *
na_gconf_utils_get_entries( GConfClient *gconf, const gchar *path )
{
	static const gchar *thisfn = "na_gconf_utils_get_entries";
	GError *error = NULL;
	GSList *list_entries;

	list_entries = gconf_client_all_entries( gconf, path, &error );

	if( error ){
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
		return(( GSList * ) NULL );
	}

	return( list_entries );
}

/**
 * na_gconf_utils_get_bool_from_entries:
 * @entries: a list of #GConfEntry as returned by na_gconf_utils_get_entries().
 * @entry: the searched entry.
 * @value: a pointer to a gboolean to be set to the found value.
 *
 * Returns: %TRUE if the entry was found, %FALSE else.
 *
 * If the entry was not found, or was not of boolean type, @value is set
 * to %FALSE.
 */
gboolean
na_gconf_utils_get_bool_from_entries( GSList *entries, const gchar *entry, gboolean *value )
{
	GSList *ip;
	GConfEntry *gconf_entry;
	GConfValue *gconf_value;
	gchar *key;
	gboolean found;

	g_return_val_if_fail( value, FALSE );

	*value = FALSE;
	found = FALSE;

	for( ip = entries ; ip && !found ; ip = ip->next ){
		gconf_entry = ( GConfEntry * ) ip->data;
		key = g_path_get_basename( gconf_entry_get_key( gconf_entry ));

		if( !strcmp( key, entry )){
			gconf_value = gconf_entry_get_value( gconf_entry );

			if( gconf_value &&
				gconf_value->type == GCONF_VALUE_BOOL ){

					found = TRUE;
					*value = gconf_value_get_bool( gconf_value );
			}
		}
		g_free( key );
	}

	return( found );
}

/**
 * na_gconf_utils_get_string_from_entries:
 * @entries: a list of #GConfEntry as returned by na_gconf_utils_get_entries().
 * @entry: the searched entry.
 * @value: a pointer to a gchar * to be set to the found value.
 *
 * Returns: %TRUE if the entry was found, %FALSE else.
 *
 * If the entry was not found, or was not of string type, @value is set
 * to %NULL.
 *
 * If @value is returned not NULL, it should be g_free() by the caller.
 */
gboolean
na_gconf_utils_get_string_from_entries( GSList *entries, const gchar *entry, gchar **value )
{
	GSList *ip;
	GConfEntry *gconf_entry;
	GConfValue *gconf_value;
	gchar *key;
	gboolean found;

	g_return_val_if_fail( value, FALSE );

	*value = NULL;
	found = FALSE;

	for( ip = entries ; ip && !found ; ip = ip->next ){
		gconf_entry = ( GConfEntry * ) ip->data;
		key = g_path_get_basename( gconf_entry_get_key( gconf_entry ));

		if( !strcmp( key, entry )){
			gconf_value = gconf_entry_get_value( gconf_entry );

			if( gconf_value &&
				gconf_value->type == GCONF_VALUE_STRING ){

					found = TRUE;
					*value = g_strdup( gconf_value_get_string( gconf_value ));
			}
		}
		g_free( key );
	}

	return( found );
}

/**
 * na_gconf_utils_get_string_list_from_entries:
 * @entries: a list of #GConfEntry as returned by na_gconf_utils_get_entries().
 * @entry: the searched entry.
 * @value: a pointer to a GSList * to be set to the found value.
 *
 * Returns: %TRUE if the entry was found, %FALSE else.
 *
 * If the entry was not found, or was not of string list type, @value
 * is set to %NULL.
 *
 * If @value is returned not NULL, it should be na_utils_free_string_list()
 * by the caller.
 */
gboolean
na_gconf_utils_get_string_list_from_entries( GSList *entries, const gchar *entry, GSList **value )
{
	GSList *ip, *iv;
	GConfEntry *gconf_entry;
	GConfValue *gconf_value;
	gchar *key;
	gboolean found;
	GSList *list_values;

	g_return_val_if_fail( value, FALSE );

	*value = NULL;
	found = FALSE;

	for( ip = entries ; ip && !found ; ip = ip->next ){
		gconf_entry = ( GConfEntry * ) ip->data;
		key = g_path_get_basename( gconf_entry_get_key( gconf_entry ));

		if( !strcmp( key, entry )){
			gconf_value = gconf_entry_get_value( gconf_entry );

			if( gconf_value &&
				gconf_value->type == GCONF_VALUE_LIST ){

					found = TRUE;
					list_values = gconf_value_get_list( gconf_value );
					for( iv = list_values ; iv ; iv = iv->next ){
						*value = g_slist_append( *value, g_strdup( gconf_value_get_string(( GConfValue * ) iv->data )));
					}
			}
		}
		g_free( key );
	}

	return( found );
}

/**
 * na_gconf_utils_free_entries:
 * @list: a list of #GConfEntry as returned by na_gconf_utils_get_entries().
 *
 * Releases the provided list.
 */
void
na_gconf_utils_free_entries( GSList *list )
{
	g_slist_foreach( list, ( GFunc ) gconf_entry_unref, NULL );
	g_slist_free( list );
}

/**
 * na_gconf_utils_read_string:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the key.
 * @use_schema: whether to use the default value from schema, or not.
 * @default_value: default value to be used if schema is not used or
 * doesn't exist.
 *
 * Returns: the required string value in a newly allocated string which
 * should be g_free() by the caller.
 */
gchar *
na_gconf_utils_read_string( GConfClient *gconf, const gchar *path, gboolean use_schema, const gchar *default_value )
{
	GConfValue *value = NULL;
	gchar *result;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), NULL );

	result = g_strdup( default_value );

	value = read_value( gconf, path, use_schema, GCONF_VALUE_STRING );

	if( value ){
		g_free( result );
		result = g_strdup( gconf_value_get_string( value ));
		gconf_value_free( value );
	}

	return( result );
}

static GConfValue *
read_value( GConfClient *gconf, const gchar *path, gboolean use_schema, GConfValueType type )
{
	static const gchar *thisfn = "na_gconf_utils_read_value";
	GError *error = NULL;
	GConfValue *value = NULL;

	if( use_schema ){
		value = gconf_client_get( gconf, path, &error );
	} else {
		value = gconf_client_get_without_default( gconf, path, &error );
	}

	if( error ){
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
		if( value ){
			gconf_value_free( value );
			value = NULL;
		}
	}

	if( value ){
		if( value->type != type ){
			g_warning( "%s: path=%s, found type '%u' while waiting for type '%u'", thisfn, path, value->type, type );
			gconf_value_free( value );
			value = NULL;
		}
	}

	return( value );
}

/**
 * na_gconf_utils_read_bool:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the key.
 * @use_schema: whether to use the default value from schema, or not.
 * @default_value: default value to be used if schema is not used or
 * doesn't exist.
 *
 * Returns: the required boolean value.
 */
gboolean
na_gconf_utils_read_bool( GConfClient *gconf, const gchar *path, gboolean use_schema, gboolean default_value )
{
	GConfValue *value;
	gboolean ret;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), FALSE );

	ret = default_value;

	value = read_value( gconf, path, use_schema, GCONF_VALUE_BOOL );
	if( value ){
		ret = gconf_value_get_bool( value );
		gconf_value_free( value );
	}

	return( ret );
}

/**
 * na_gconf_utils_read_int:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the key.
 * @use_schema: whether to use the default value from schema, or not.
 * @default_value: default value to be used if schema is not used or
 * doesn't exist.
 *
 * Returns: the required integer value.
 */
gint
na_gconf_utils_read_int( GConfClient *gconf, const gchar *path, gboolean use_schema, gint default_value )
{
	GConfValue *value = NULL;
	gint ret;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), FALSE );

	ret = default_value;

	value = read_value( gconf, path, use_schema, GCONF_VALUE_INT );

	if( value ){
		ret = gconf_value_get_int( value );
		gconf_value_free( value );
	}

	return( ret );
}

/**
 * na_gconf_utils_read_string_list:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the key to be read.
 *
 * Returns: a list of strings,
 * or %NULL if the entry was not found or was not of string list type.
 *
 * The returned list must be released with na_utils_free_string_list().
 */
GSList *
na_gconf_utils_read_string_list( GConfClient *gconf, const gchar *path )
{
	static const gchar *thisfn = "na_gconf_utils_read_string_list";
	GError *error = NULL;
	GSList *list_strings;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), NULL );

	list_strings = gconf_client_get_list( gconf, path, GCONF_VALUE_STRING, &error );

	if( error ){
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
		return( NULL );
	}

	return( list_strings );
}

/**
 * na_gconf_utils_write_bool:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the key.
 * @value: the value to be written.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Writes a boolean at the given @path.
 *
 * Returns: %TRUE if the writing has been successfull, %FALSE else.
 *
 * If returned not NULL, the @message contains an error message.
 * It should be g_free() by the caller.
 */
gboolean
na_gconf_utils_write_bool( GConfClient *gconf, const gchar *path, gboolean value, gchar **message )
{
	static const gchar *thisfn = "na_gconf_utils_write_bool";
	gboolean ret = TRUE;
	GError *error = NULL;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), FALSE );

	if( !gconf_client_set_bool( gconf, path, value, &error )){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, value=%s, error=%s", thisfn, path, value ? "True":"False", error->message );
		g_error_free( error );
		ret = FALSE;
	}

	return( ret );
}

/**
 * na_gconf_utils_write_int:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the key.
 * @value: the value to be written.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Writes an integer at the given @path.
 *
 * Returns: %TRUE if the writing has been successfull, %FALSE else.
 *
 * If returned not NULL, the @message contains an error message.
 * It should be g_free() by the caller.
 */
gboolean
na_gconf_utils_write_int( GConfClient *gconf, const gchar *path, gint value, gchar **message )
{
	static const gchar *thisfn = "na_gconf_utils_write_int";
	gboolean ret = TRUE;
	GError *error = NULL;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), FALSE );

	if( !gconf_client_set_int( gconf, path, value, &error )){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, value=%d, error=%s", thisfn, path, value, error->message );
		g_error_free( error );
		ret = FALSE;
	}

	return( ret );
}

/**
 * na_gconf_utils_write_string:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the key.
 * @value: the value to be written.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Writes a string at the given @path.
 *
 * Returns: %TRUE if the writing has been successfull, %FALSE else.
 *
 * If returned not NULL, the @message contains an error message.
 * It should be g_free() by the caller.
 */
gboolean
na_gconf_utils_write_string( GConfClient *gconf, const gchar *path, const gchar *value, gchar **message )
{
	static const gchar *thisfn = "na_gconf_utils_write_string";
	gboolean ret = TRUE;
	GError *error = NULL;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), FALSE );

	if( !gconf_client_set_string( gconf, path, value, &error )){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, value=%s, error=%s", thisfn, path, value, error->message );
		g_error_free( error );
		ret = FALSE;
	}

	return( ret );
}

/**
 * na_gconf_utils_write_string_list:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the key.
 * @value: the list of values to be written.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Writes a list of strings at the given @path.
 *
 * Returns: %TRUE if the writing has been successfull, %FALSE else.
 *
 * If returned not NULL, the @message contains an error message.
 * It should be g_free() by the caller.
 */
gboolean
na_gconf_utils_write_string_list( GConfClient *gconf, const gchar *path, GSList *value, gchar **message )
{
	static const gchar *thisfn = "na_gconf_utils_write_string_list";
	gboolean ret = TRUE;
	GError *error = NULL;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), FALSE );

	if( !gconf_client_set_list( gconf, path, GCONF_VALUE_STRING, value, &error )){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, value=%p (count=%d), error=%s",
				thisfn, path, ( void * ) value, g_slist_length( value ), error->message );
		g_error_free( error );
		ret = FALSE;
	}

	if( ret ){
		ret = sync_gconf( gconf, message );
	}

	return( ret );
}

/**
 * na_gconf_utils_remove_entry:
 * @gconf: a #GConfClient instance.
 * @path: the full path to the entry.
 * @message: a pointer to a gchar * which will be allocated if needed.
 *
 * Removes an entry from user preferences.
 */
gboolean
na_gconf_utils_remove_entry( GConfClient *gconf, const gchar *path, gchar **message )
{
	static const gchar *thisfn = "na_gconf_utils_remove_entry";
	gboolean ret;
	GError *error = NULL;

	g_return_val_if_fail( GCONF_IS_CLIENT( gconf ), FALSE );

	ret = gconf_client_unset( gconf, path, &error );
	if( !ret ){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: path=%s, error=%s", thisfn, path, error->message );
		g_error_free( error );
	}

	if( ret ){
		ret = sync_gconf( gconf, message );
	}

	return( ret );
}

static gboolean
sync_gconf( GConfClient *gconf, gchar **message )
{
	static const gchar *thisfn = "na_gconf_utils_sync_gconf";
	gboolean ret = TRUE;
	GError *error = NULL;

	gconf_client_suggest_sync( gconf, &error );
	if( error ){
		if( message ){
			*message = g_strdup( error->message );
		}
		g_warning( "%s: error=%s", thisfn, error->message );
		g_error_free( error );
		ret = FALSE;
	}

	return( ret );
}