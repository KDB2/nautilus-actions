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

#include <core/na-io-provider.h>

#include "base-gtk-utils.h"
#include "nact-main-statusbar.h"

typedef struct {
	guint         event_source_id;
	guint         context_id;
	GtkStatusbar *bar;
}
	StatusbarTimeoutDisplayStruct;

#define LOCKED_IMAGE					PKGUIDIR "/locked.png"

static GtkStatusbar *get_statusbar( const NactMainWindow *window );
static gboolean      display_timeout( StatusbarTimeoutDisplayStruct *stds );
static void          display_timeout_free( StatusbarTimeoutDisplayStruct *stds );

/**
 * nact_main_statusbar_initialize_gtk_toplevel:
 * @window: the #NactMainWindow.
 *
 * Initial loading of the UI.
 */
void
nact_main_statusbar_initialize_gtk_toplevel( NactMainWindow *window )
{
	static const gchar *thisfn = "nact_main_statusbar_initialize_gtk_toplevel";
	gint width, height;
	GtkStatusbar *bar;
	GtkFrame *frame;
/* gtk_widget_size_request() is deprecated since Gtk+ 3.0
 * see http://library.gnome.org/devel/gtk/unstable/GtkWidget.html#gtk-widget-render-icon
 * and http://git.gnome.org/browse/gtk+/commit/?id=07eeae15825403037b7df139acf9bfa104d5559d
 */
#if GTK_CHECK_VERSION( 2, 91, 7 )
	GtkRequisition minimal_size, natural_size;
#else
	GtkRequisition requisition;
#endif

	g_debug( "%s: window=%p", thisfn, ( void * ) window );

	gtk_icon_size_lookup( GTK_ICON_SIZE_MENU, &width, &height );

	bar = get_statusbar( window );
	frame = GTK_FRAME( base_window_get_widget( BASE_WINDOW( window ), "ActionLockedFrame" ));

#if GTK_CHECK_VERSION( 2, 91, 7 )
	gtk_widget_get_preferred_size( GTK_WIDGET( bar ), &minimal_size, &natural_size );
	gtk_widget_set_size_request( GTK_WIDGET( bar ), natural_size.width, height+8 );
#else
	gtk_widget_size_request( GTK_WIDGET( bar ), &requisition );
	gtk_widget_set_size_request( GTK_WIDGET( bar ), requisition.width, height+8 );
#endif

	gtk_widget_set_size_request( GTK_WIDGET( frame ), width+4, height+4 );
	gtk_frame_set_shadow_type( frame, GTK_SHADOW_IN );
}

/**
 * nact_main_statusbar_display_status:
 * @window: the #NactMainWindow instance.
 * @context: the context to be displayed.
 * @status: the message.
 *
 * Push a message.
 */
void
nact_main_statusbar_display_status( NactMainWindow *window, const gchar *context, const gchar *status )
{
	static const gchar *thisfn = "nact_main_statusbar_display_status";
	GtkStatusbar *bar;

	g_debug( "%s: window=%p, context=%s, status=%s", thisfn, ( void * ) window, context, status );

	if( !status || !g_utf8_strlen( status, -1 )){
		return;
	}

	bar = get_statusbar( window );

	if( bar ){
		guint context_id = gtk_statusbar_get_context_id( bar, context );
		gtk_statusbar_push( bar, context_id, status );
	}
}

/**
 * nact_main_statusbar_display_with_timeout:
 * @window: the #NactMainWindow instance.
 * @context: the context to be displayed.
 * @status: the message.
 *
 * Push a message.
 * Automatically pop it after a timeout.
 * The timeout is not suspended when another message is pushed onto the
 * previous one.
 */
void
nact_main_statusbar_display_with_timeout( NactMainWindow *window, const gchar *context, const gchar *status )
{
	static const gchar *thisfn = "nact_main_statusbar_display_with_timeout";
	GtkStatusbar *bar;
	StatusbarTimeoutDisplayStruct *stds;

	g_debug( "%s: window=%p, context=%s, status=%s", thisfn, ( void * ) window, context, status );

	if( !status || !g_utf8_strlen( status, -1 )){
		return;
	}

	bar = get_statusbar( window );

	if( bar ){
		guint context_id = gtk_statusbar_get_context_id( bar, context );
		gtk_statusbar_push( bar, context_id, status );

		stds = g_new0( StatusbarTimeoutDisplayStruct, 1 );
		stds->context_id = context_id;
		stds->bar = bar;
		stds->event_source_id = g_timeout_add_seconds_full(
				G_PRIORITY_DEFAULT,
				10,
				( GSourceFunc ) display_timeout,
				stds,
				( GDestroyNotify ) display_timeout_free );
	}
}

/**
 * nact_main_statusbar_hide_status:
 * @window: the #NactMainWindow instance.
 * @context: the context to be hidden.
 *
 * Hide the specified context.
 */
void
nact_main_statusbar_hide_status( NactMainWindow *window, const gchar *context )
{
	GtkStatusbar *bar;

	bar = get_statusbar( window );

	if( bar ){
		guint context_id = gtk_statusbar_get_context_id( bar, context );
		gtk_statusbar_pop( bar, context_id );
	}
}

/**
 * nact_main_statusbar_set_locked:
 * @window: the #NactMainWindow instance.
 * @provider: whether the current provider is locked (read-only).
 * @item: whether the current item is locked (read-only).
 *
 * Displays the writability status of the current item as an image.
 * Installs the corresponding tooltip.
 */
void
nact_main_statusbar_set_locked( NactMainWindow *window, gboolean readonly, gint reason )
{
	static const gchar *thisfn = "nact_main_statusbar_set_locked";
	GtkStatusbar *bar;
	GtkFrame *frame;
	GtkImage *image;
	gchar *tooltip;
	gboolean set_pixbuf;

	g_debug( "%s: window=%p, readonly=%s, reason=%d", thisfn, ( void * ) window, readonly ? "True":"False", reason );

	set_pixbuf = TRUE;
	bar = get_statusbar( window );
	frame = GTK_FRAME( base_window_get_widget( BASE_WINDOW( window ), "ActionLockedFrame" ));
	image = GTK_IMAGE( base_window_get_widget( BASE_WINDOW( window ), "ActionLockedImage" ));

	if( bar && frame && image ){

		tooltip = g_strdup( "" );

		if( readonly ){
			gtk_image_set_from_file( image, LOCKED_IMAGE );
			set_pixbuf = FALSE;
			g_free( tooltip );
			tooltip = na_io_provider_get_readonly_tooltip( reason );
		}

		gtk_widget_set_tooltip_text( GTK_WIDGET( image ), tooltip );
		g_free( tooltip );
	}

	if( set_pixbuf ){
		base_gtk_utils_render( NULL, image, GTK_ICON_SIZE_MENU );
	}
}

/*
 * Returns the status bar widget
 */
static GtkStatusbar *
get_statusbar( const NactMainWindow *window )
{
	GtkWidget *statusbar;

	statusbar = base_window_get_widget( BASE_WINDOW( window ), "MainStatusbar" );

	return( GTK_STATUSBAR( statusbar ));
}

static gboolean
display_timeout( StatusbarTimeoutDisplayStruct *stds )
{
	gboolean keep_source = FALSE;

	gtk_statusbar_pop( stds->bar, stds->context_id );

	return( keep_source );
}

static void
display_timeout_free( StatusbarTimeoutDisplayStruct *stds )
{
	g_debug( "nact_main_statusbar_display_timeout_free: stds=%p", ( void * ) stds );

	g_free( stds );
}
