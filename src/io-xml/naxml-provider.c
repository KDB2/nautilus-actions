/*
 * Nautilus-Actions
 * A Nautilus extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
 * Copyright (C) 2006-2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009-2014 Pierre Wieser and others (see AUTHORS)
 *
 * Nautilus-Actions is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * Nautilus-Actions is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nautilus-Actions; see the file COPYING. If not, see
 * <http://www.gnu.org/licenses/>.
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

#include <api/na-ifactory-provider.h>
#include <api/na-iexporter.h>
#include <api/na-iimporter.h>

#include "naxml-provider.h"
#include "naxml-formats.h"
#include "naxml-reader.h"
#include "naxml-writer.h"

/* private class data
 */
struct _NAXMLProviderClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct _NAXMLProviderPrivate {
	gboolean dispose_has_run;
};

static GType         st_module_type = 0;
static GObjectClass *st_parent_class = NULL;

static void   class_init( NAXMLProviderClass *klass );
static void   instance_init( GTypeInstance *instance, gpointer klass );
static void   instance_dispose( GObject *object );
static void   instance_finalize( GObject *object );

static void   iimporter_iface_init( NAIImporterInterface *iface );
static guint  iimporter_get_version( const NAIImporter *importer );

static void   iexporter_iface_init( NAIExporterInterface *iface );
static guint  iexporter_get_version( const NAIExporter *exporter );
static gchar *iexporter_get_name( const NAIExporter *exporter );
static void  *iexporter_get_formats( const NAIExporter *exporter );
static void   iexporter_free_formats( const NAIExporter *exporter, GList *format_list );

static void   ifactory_provider_iface_init( NAIFactoryProviderInterface *iface );
static guint  ifactory_provider_get_version( const NAIFactoryProvider *factory );

GType
naxml_provider_get_type( void )
{
	return( st_module_type );
}

void
naxml_provider_register_type( GTypeModule *module )
{
	static const gchar *thisfn = "naxml_provider_register_type";

	static GTypeInfo info = {
		sizeof( NAXMLProviderClass ),
		NULL,
		NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAXMLProvider ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo iimporter_iface_info = {
		( GInterfaceInitFunc ) iimporter_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo iexporter_iface_info = {
		( GInterfaceInitFunc ) iexporter_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo ifactory_provider_iface_info = {
		( GInterfaceInitFunc ) ifactory_provider_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	st_module_type = g_type_module_register_type( module, G_TYPE_OBJECT, "NAXMLProvider", &info, 0 );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IIMPORTER, &iimporter_iface_info );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IEXPORTER, &iexporter_iface_info );

	g_type_module_add_interface( module, st_module_type, NA_TYPE_IFACTORY_PROVIDER, &ifactory_provider_iface_info );
}

static void
class_init( NAXMLProviderClass *klass )
{
	static const gchar *thisfn = "naxml_provider_class_init";
	GObjectClass *object_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAXMLProviderClassPrivate, 1 );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "naxml_provider_instance_init";
	NAXMLProvider *self;

	g_return_if_fail( NA_IS_XML_PROVIDER( instance ));

	g_debug( "%s: instance=%p (%s), klass=%p",
			thisfn, ( void * ) instance, G_OBJECT_TYPE_NAME( instance ), ( void * ) klass );

	self = NAXML_PROVIDER( instance );

	self->private = g_new0( NAXMLProviderPrivate, 1 );

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "naxml_provider_instance_dispose";
	NAXMLProvider *self;

	g_return_if_fail( NA_IS_XML_PROVIDER( object ));

	self = NAXML_PROVIDER( object );

	if( !self->private->dispose_has_run ){

		g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

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
	static const gchar *thisfn = "naxml_provider_instance_finalize";
	NAXMLProvider *self;

	g_return_if_fail( NA_IS_XML_PROVIDER( object ));

	g_debug( "%s: object=%p (%s)", thisfn, ( void * ) object, G_OBJECT_TYPE_NAME( object ));

	self = NAXML_PROVIDER( object );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

static void
iimporter_iface_init( NAIImporterInterface *iface )
{
	static const gchar *thisfn = "naxml_provider_iimporter_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = iimporter_get_version;
	iface->import_from_uri = naxml_reader_import_from_uri;
}

static guint
iimporter_get_version( const NAIImporter *importer )
{
	return( 2 );
}

static void
iexporter_iface_init( NAIExporterInterface *iface )
{
	static const gchar *thisfn = "naxml_provider_iexporter_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = iexporter_get_version;
	iface->get_name = iexporter_get_name;
	iface->get_formats = iexporter_get_formats;
	iface->free_formats = iexporter_free_formats;
	iface->to_file = naxml_writer_export_to_file;
	iface->to_buffer = naxml_writer_export_to_buffer;
}

static guint
iexporter_get_version( const NAIExporter *exporter )
{
	return( 2 );
}

static gchar *
iexporter_get_name( const NAIExporter *exporter )
{
	return( g_strdup( "NAXML Exporter" ));
}

static void *
iexporter_get_formats( const NAIExporter *exporter )
{
	return(( void * ) naxml_formats_get_formats( exporter ));
}

static void
iexporter_free_formats( const NAIExporter *exporter, GList *format_list )
{
	naxml_formats_free_formats( format_list );
}

static void
ifactory_provider_iface_init( NAIFactoryProviderInterface *iface )
{
	static const gchar *thisfn = "naxml_provider_ifactory_provider_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );

	iface->get_version = ifactory_provider_get_version;
	iface->read_start = naxml_reader_read_start;
	iface->read_data = naxml_reader_read_data;
	iface->read_done = naxml_reader_read_done;
	iface->write_start = naxml_writer_write_start;
	iface->write_data = naxml_writer_write_data;
	iface->write_done = naxml_writer_write_done;
}

static guint
ifactory_provider_get_version( const NAIFactoryProvider *factory )
{
	return( 1 );
}
