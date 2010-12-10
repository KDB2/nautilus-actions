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

#ifndef __NAUTILUS_ACTIONS_API_NA_IEXPORTER_H__
#define __NAUTILUS_ACTIONS_API_NA_IEXPORTER_H__

#include "na-object-item.h"

G_BEGIN_DECLS

#define NA_IEXPORTER_TYPE                       ( na_iexporter_get_type())
#define NA_IEXPORTER( instance )                ( G_TYPE_CHECK_INSTANCE_CAST( instance, NA_IEXPORTER_TYPE, NAIExporter ))
#define NA_IS_IEXPORTER( instance )             ( G_TYPE_CHECK_INSTANCE_TYPE( instance, NA_IEXPORTER_TYPE ))
#define NA_IEXPORTER_GET_INTERFACE( instance )  ( G_TYPE_INSTANCE_GET_INTERFACE(( instance ), NA_IEXPORTER_TYPE, NAIExporterInterface ))

typedef struct _NAIExporter                 NAIExporter;
typedef struct _NAIExporterFileParms        NAIExporterFileParms;
typedef struct _NAIExporterBufferParms      NAIExporterBufferParms;
typedef struct _NAIExporterInterfacePrivate NAIExporterInterfacePrivate;

/**
 * NAIExporterFormat:
 * @format:      format identifier (ascii).
 * @label:       short label to be displayed in dialog (UTF-8 localized)
 * @description: full description of the format (UTF-8 localized);
 *               mainly used in the export assistant.
 *
 * This structure describes a supported output format.
 * It must be provided by each #NAIExporter implementation
 * (see e.g. <filename>src/io-xml/naxml-formats.c</filename>).
 *
 * When listing available export formats, the instance returns a #GList
 * of these structures.
 */
typedef struct {
	gchar *format;
	gchar *label;
	gchar *description;
}
	NAIExporterFormat;

/**
 * NAIExporterInterface:
 * @get_version: returns the version of this interface the plugin implements.
 * @get_name:    returns the public plugin name.
 * @get_formats: returns the list of supported formats.
 * @to_file:     exports an item to a file.
 * @to_buffer:   exports an item to a buffer.
 *
 * This defines the interface that a #NAIExporter should implement.
 */
typedef struct {
	/*< private >*/
	GTypeInterface               parent;
	NAIExporterInterfacePrivate *private;

	/*< public >*/
	/**
	 * get_version:
	 * @instance: this #NAIExporter instance.
	 *
	 * Returns: the version of this interface supported by the I/O provider.
	 *
	 * Defaults to 1.
	 *
	 * Since: Nautilus-Actions v 2.30, NAIExporter interface v 1.
	 */
	guint                     ( *get_version )( const NAIExporter *instance );

	/**
	 * get_name:
	 * @instance: this #NAIExporter instance.
	 *
	 * Returns: the name to be displayed for this instance, as a
	 * newly allocated string which should be g_free() by the caller.
	 *
	 * Since: Nautilus-Actions v 2.30, NAIExporter interface v 1.
	 */
	gchar *                   ( *get_name )   ( const NAIExporter *instance );

	/**
	 * get_formats:
	 * @instance: this #NAIExporter instance.
	 *
	 * The returned list is owned by the @instance. It must not be
	 * released by the caller.
	 *
	 * To avoid any collision, the format id is allocated by the
	 * Nautilus-Actions maintainer team. If you wish develop a new
	 * export format, and so need a new format id, please contact the
	 * maintainers (see #nautilus-actions.doap).
	 *
	 * Returns: a list of #NAIExporterFormat structures which describe the
	 * formats supported by @instance.
	 *
	 * Defaults to %NULL (no format at all).
	 *
	 * Since: Nautilus-Actions v 2.30, NAIExporter interface v 1.
	 */
	const NAIExporterFormat * ( *get_formats )( const NAIExporter *instance );

	/**
	 * to_file:
	 * @instance: this #NAIExporter instance.
	 * @parms: a #NAIExporterFileParms structure.
	 *
	 * Exports the specified 'exported' to the target 'folder' in the required
	 * 'format'.
	 *
	 * Returns: the #NAIExporterExportStatus status of the operation.
	 *
	 * Since: Nautilus-Actions v 2.30, NAIExporter interface v 1.
	 */
	guint                     ( *to_file )    ( const NAIExporter *instance, NAIExporterFileParms *parms );

	/**
	 * to_buffer:
	 * @instance: this #NAIExporter instance.
	 * @parms: a #NAIExporterFileParms structure.
	 *
	 * Exports the specified 'exported' to a newly allocated 'buffer' in
	 * the required 'format'. The allocated 'buffer' should be g_free()
	 * by the caller.
	 *
	 * Returns: the #NAIExporterExportStatus status of the operation.
	 *
	 * Since: Nautilus-Actions v 2.30, NAIExporter interface v 1.
	 */
	guint                     ( *to_buffer )  ( const NAIExporter *instance, NAIExporterBufferParms *parms );
}
	NAIExporterInterface;

/**
 * NAIExporterExportStatus:
 * @NA_IEXPORTER_CODE_OK:              export OK.
 * @NA_IEXPORTER_CODE_INVALID_ITEM:    exported item was found invalid.
 * @NA_IEXPORTER_CODE_INVALID_TARGET:  selected target was found invalid.
 * @NA_IEXPORTER_CODE_INVALID_FORMAT:  asked format was found invalid.
 * @NA_IEXPORTER_CODE_UNABLE_TO_WRITE: unable to write the item.
 * @NA_IEXPORTER_CODE_ERROR:           other undetermined error.
 *
 * The reasons for which an item may not have been exported
 */
typedef enum {
	NA_IEXPORTER_CODE_OK = 0,
	NA_IEXPORTER_CODE_INVALID_ITEM,
	NA_IEXPORTER_CODE_INVALID_TARGET,
	NA_IEXPORTER_CODE_INVALID_FORMAT,
	NA_IEXPORTER_CODE_UNABLE_TO_WRITE,
	NA_IEXPORTER_CODE_ERROR,
}
	NAIExporterExportStatus;

/**
 * NAIExporterFileParms:
 * @version:  version of this structure (input, since v 1)
 * @exported: exported NAObjectItem-derived object (input, since v 1)
 * @folder:   URI of the target folder (input, since v 1)
 * @format:   export format as a GQuark (input, since v 1)
 * @basename: basename of the exported file (output, since v 1)
 * @messages: a #GSList list of localized strings;
 *            the provider may append messages to this list,
 *            but shouldn't reinitialize it
 *            (input/output, since v 1).
 *
 * The structure that the plugin receives as a parameter of
 * #NAIExporterInterface.to_file () interface method.
 */
struct _NAIExporterFileParms {
	guint         version;
	NAObjectItem *exported;
	gchar        *folder;
	GQuark        format;
	gchar        *basename;
	GSList       *messages;
};

/**
 * NAIExporterBufferParms:
 * @version:  version of this structure (input, since v 1)
 * @exported: exported NAObjectItem-derived object (input, since v 1)
 * @format:   export format as a GQuark (input, since v 1)
 * @buffer:   buffer which contains the exported object (output, since v 1)
 * @messages: a #GSList list of localized strings;
 *            the provider may append messages to this list,
 *            but shouldn't reinitialize it
 *            (input/output, since v 1).
 *
 * The structure that the plugin receives as a parameter of
 * #NAIExporterInterface.to_buffer () interface method.
 */
struct _NAIExporterBufferParms {
	guint         version;
	NAObjectItem *exported;
	GQuark        format;
	gchar        *buffer;
	GSList       *messages;
};

GType na_iexporter_get_type( void );

G_END_DECLS

#endif /* __NAUTILUS_ACTIONS_API_NA_IEXPORTER_H__ */
