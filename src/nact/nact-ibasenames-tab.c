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

#include <glib/gi18n.h>

#include <api/na-object-api.h>

#include "nact-main-tab.h"
#include "base-gtk-utils.h"
#include "nact-match-list.h"
#include "nact-ibasenames-tab.h"

/* private interface data
 */
struct _NactIBasenamesTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* the identifier of this notebook page in the Match dialog
 */
#define ITAB_NAME						"basenames"

/* data set against the instance
 */
typedef struct {
	gboolean on_selection_change;
}
	IBasenamesData;

#define IBASENAMES_TAB_PROP_DATA		"nact-ibasenames-tab-data"

static guint    st_initializations = 0;	/* interface initialization count */

static GType           register_type( void );
static void            interface_base_init( NactIBasenamesTabInterface *klass );
static void            interface_base_finalize( NactIBasenamesTabInterface *klass );

static void            on_base_initialize_gtk( NactIBasenamesTab *instance, GtkWindow *toplevel, gpointer user_data );
static void            on_base_initialize_window( NactIBasenamesTab *instance, gpointer user_data );

static void            on_main_selection_changed( NactIBasenamesTab *instance, GList *selected_items, gpointer user_data );

static void            on_matchcase_toggled( GtkToggleButton *button, BaseWindow *window );
static GSList         *get_basenames( void *context );
static void            set_basenames( void *context, GSList *filters );

static IBasenamesData *get_ibasenames_data( NactIBasenamesTab *instance );
static void            on_instance_finalized( gpointer user_data, NactIBasenamesTab *instance );

GType
nact_ibasenames_tab_get_type( void )
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
	static const gchar *thisfn = "nact_ibasenames_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NactIBasenamesTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NactIBasenamesTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_TYPE_WINDOW );

	return( type );
}

static void
interface_base_init( NactIBasenamesTabInterface *klass )
{
	static const gchar *thisfn = "nact_ibasenames_tab_interface_base_init";

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NactIBasenamesTabInterfacePrivate, 1 );
	}

	st_initializations += 1;
}

static void
interface_base_finalize( NactIBasenamesTabInterface *klass )
{
	static const gchar *thisfn = "nact_ibasenames_tab_interface_base_finalize";

	st_initializations -= 1;

	if( !st_initializations ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		g_free( klass->private );
	}
}

/*
 * nact_ibasenames_tab_init:
 * @instance: this #NactIBasenamesTab instance.
 *
 * Initialize the interface
 * Connect to #BaseWindow signals
 */
void
nact_ibasenames_tab_init( NactIBasenamesTab *instance )
{
	static const gchar *thisfn = "nact_ibasenames_tab_init";
	IBasenamesData *data;

	g_return_if_fail( NACT_IS_IBASENAMES_TAB( instance ));

	g_debug( "%s: instance=%p (%s)",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_GTK,
			G_CALLBACK( on_base_initialize_gtk ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_SIGNAL_INITIALIZE_WINDOW,
			G_CALLBACK( on_base_initialize_window ));

	nact_main_tab_init( NACT_MAIN_WINDOW( instance ), TAB_BASENAMES );

	data = get_ibasenames_data( instance );
	data->on_selection_change = FALSE;

	g_object_weak_ref( G_OBJECT( instance ), ( GWeakNotify ) on_instance_finalized, NULL );
}

/*
 * on_base_initialize_gtk:
 * @window: this #NactIBasenamesTab instance.
 *
 * Initializes the tab widget at initial load.
 */
static void
on_base_initialize_gtk( NactIBasenamesTab *instance, GtkWindow *toplevel, void *user_data )
{
	static const gchar *thisfn = "nact_ibasenames_tab_on_base_initialize_gtk";

	g_return_if_fail( NACT_IS_IBASENAMES_TAB( instance ));

	g_debug( "%s: instance=%p (%s), toplevel=%p, user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) toplevel,
			( void * ) user_data );

	nact_match_list_init_with_args(
			BASE_WINDOW( instance ),
			ITAB_NAME,
			TAB_BASENAMES,
			base_window_get_widget( BASE_WINDOW( instance ), "BasenamesTreeView" ),
			base_window_get_widget( BASE_WINDOW( instance ), "AddBasenameButton" ),
			base_window_get_widget( BASE_WINDOW( instance ), "RemoveBasenameButton" ),
			( pget_filters ) get_basenames,
			( pset_filters ) set_basenames,
			NULL,
			NULL,
			MATCH_LIST_MUST_MATCH_ONE_OF,
			_( "Basename filter" ),
			TRUE );
}

/*
 * on_base_initialize_window:
 * @window: this #NactIBasenamesTab instance.
 *
 * Initializes the tab widget at each time the widget will be displayed.
 * Connect signals and setup runtime values.
 */
static void
on_base_initialize_window( NactIBasenamesTab *instance, void *user_data )
{
	static const gchar *thisfn = "nact_ibasenames_tab_on_base_initialize_window";
	GtkWidget *button;

	g_return_if_fail( NACT_IS_IBASENAMES_TAB( instance ));

	g_debug( "%s: instance=%p (%s), user_data=%p",
			thisfn,
			( void * ) instance, G_OBJECT_TYPE_NAME( instance ),
			( void * ) user_data );

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			MAIN_SIGNAL_SELECTION_CHANGED,
			G_CALLBACK( on_main_selection_changed ));

	button = base_window_get_widget( BASE_WINDOW( instance ), "BasenamesMatchcaseButton" );
	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( button ),
			"toggled",
			G_CALLBACK( on_matchcase_toggled ));
}

static void
on_main_selection_changed( NactIBasenamesTab *instance, GList *selected_items, gpointer user_data )
{
	NAIContext *context;
	gboolean editable;
	gboolean enable_tab;
	GtkToggleButton *matchcase_button;
	gboolean matchcase;
	IBasenamesData *data;

	g_object_get( G_OBJECT( instance ),
			MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
			NULL );

	enable_tab = ( context != NULL );
	nact_main_tab_enable_page( NACT_MAIN_WINDOW( instance ), TAB_BASENAMES, enable_tab );

	data = get_ibasenames_data( NACT_IBASENAMES_TAB( instance ));
	data->on_selection_change = TRUE;

	matchcase_button = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "BasenamesMatchcaseButton" ));
	matchcase = context ? na_object_get_matchcase( context ) : FALSE;
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( matchcase_button ), matchcase );
	base_gtk_utils_set_editable( G_OBJECT( matchcase_button ), editable );

	data->on_selection_change = FALSE;
}

static void
on_matchcase_toggled( GtkToggleButton *button, BaseWindow *window )
{
	NAIContext *context;
	gboolean editable;
	gboolean matchcase;
	IBasenamesData *data;

	data = get_ibasenames_data( NACT_IBASENAMES_TAB( window ));

	if( !data->on_selection_change ){
		g_object_get( G_OBJECT( window ),
				MAIN_PROP_CONTEXT, &context, MAIN_PROP_EDITABLE, &editable,
				NULL );

		if( context ){
			matchcase = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_matchcase( context, matchcase );
				g_signal_emit_by_name( G_OBJECT( window ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, context, 0 );

			} else {
				g_signal_handlers_block_by_func(( gpointer ) button, on_matchcase_toggled, window );
				gtk_toggle_button_set_active( button, !matchcase );
				g_signal_handlers_unblock_by_func(( gpointer ) button, on_matchcase_toggled, window );
			}
		}
	}
}

static GSList *
get_basenames( void *context )
{
	return( na_object_get_basenames( context ));
}

static void
set_basenames( void *context, GSList *filters )
{
	na_object_set_basenames( context, filters );
}

static IBasenamesData *
get_ibasenames_data( NactIBasenamesTab *instance )
{
	IBasenamesData *data;

	data = ( IBasenamesData * ) g_object_get_data( G_OBJECT( instance ), IBASENAMES_TAB_PROP_DATA );

	if( !data ){
		data = g_new0( IBasenamesData, 1 );
		g_object_set_data( G_OBJECT( instance ), IBASENAMES_TAB_PROP_DATA, data );
	}

	return( data );
}

static void
on_instance_finalized( gpointer user_data, NactIBasenamesTab *instance )
{
	static const gchar *thisfn = "nact_ibasenames_tab_on_instance_finalized";
	IBasenamesData *data;

	g_debug( "%s: instance=%p, user_data=%p", thisfn, ( void * ) instance, ( void * ) user_data );

	data = get_ibasenames_data( instance );

	g_free( data );
}
