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

#include <api/na-iio-provider.h>

#include "na-factory-provider.h"

extern gboolean ifactory_provider_initialized;		/* defined in na-ifactory-provider.c */
extern gboolean ifactory_provider_finalized;

/**
 * na_factory_provider_read_data:
 * @reader: the instance which implements this #NAIFactoryProvider interface.
 * @reader_data: instance data.
 * @object: the #NAIFactoryobject being unserialized.
 * @def: a #NADataDef structure which identifies the data to be unserialized.
 * @messages: a pointer to a #GSList list of strings; the implementation
 *  may append messages to this list, but shouldn't reinitialize it.
 *
 * Reads the specified data and set it up into the @boxed.
 *
 * Returns: a new #NADataBoxed object which contains the data.
 */
NADataBoxed *
na_factory_provider_read_data( const NAIFactoryProvider *reader, void *reader_data,
								const NAIFactoryObject *object, const NADataDef *def,
								GSList **messages )
{
	NADataBoxed *boxed;

	g_return_val_if_fail( NA_IS_IFACTORY_PROVIDER( reader ), NULL );
	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NULL );

	boxed = NULL;

	if( ifactory_provider_initialized && !ifactory_provider_finalized ){

		if( NA_IFACTORY_PROVIDER_GET_INTERFACE( reader )->read_data ){

			boxed = NA_IFACTORY_PROVIDER_GET_INTERFACE( reader )->read_data( reader, reader_data, object, def, messages );
		}
	}

	return( boxed );
}

/**
 * na_factory_provider_write_data:
 * @writer: the instance which implements this #NAIFactoryProvider interface.
 * @writer_data: instance data.
 * @object: the #NAIFactoryobject being serialized.
 * @boxed: the #NADataBoxed object which is to be serialized.
 * @messages: a pointer to a #GSList list of strings; the implementation
 *  may append messages to this list, but shouldn't reinitialize it.
 *
 * Returns: a NAIIOProvider operation return code.
 */
guint
na_factory_provider_write_data( const NAIFactoryProvider *writer, void *writer_data,
								const NAIFactoryObject *object, const NADataBoxed *boxed,
								GSList **messages )
{
	guint code;

	g_return_val_if_fail( NA_IS_IFACTORY_PROVIDER( writer ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );
	g_return_val_if_fail( NA_IS_IFACTORY_OBJECT( object ), NA_IIO_PROVIDER_CODE_PROGRAM_ERROR );

	code = NA_IIO_PROVIDER_CODE_NOT_WILLING_TO_RUN;

	if( ifactory_provider_initialized && !ifactory_provider_finalized ){

		if( NA_IFACTORY_PROVIDER_GET_INTERFACE( writer )->write_data ){

			code = NA_IFACTORY_PROVIDER_GET_INTERFACE( writer )->write_data( writer, writer_data, object, boxed, messages );
		}
	}

	return( code );
}