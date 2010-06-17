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

#include <glib/gi18n.h>

#include <api/na-object-api.h>

#include <core/na-io-provider.h>

#include "nact-gtk-utils.h"
#include "nact-iproperties-tab.h"
#include "nact-main-tab.h"

/* private interface data
 */
struct NactIPropertiesTabInterfacePrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

static gboolean st_initialized = FALSE;
static gboolean st_finalized   = FALSE;
static gboolean st_on_selection_change = FALSE;

static GType      register_type( void );
static void       interface_base_init( NactIPropertiesTabInterface *klass );
static void       interface_base_finalize( NactIPropertiesTabInterface *klass );

static void       on_tab_updatable_selection_changed( NactIPropertiesTab *instance, gint count_selected );
static void       on_tab_updatable_provider_changed( NactIPropertiesTab *instance, NAObjectItem *item );

static GtkButton *get_enabled_button( NactIPropertiesTab *instance );
static void       on_enabled_toggled( GtkToggleButton *button, NactIPropertiesTab *instance );
static void       on_readonly_toggled( GtkToggleButton *button, NactIPropertiesTab *instance );
static void       on_description_changed( GtkTextBuffer *buffer, NactIPropertiesTab *instance );
static void       on_shortcut_clicked( GtkButton *button, NactIPropertiesTab *instance );

static void       display_provider_name( NactIPropertiesTab *instance, NAObjectItem *item );

GType
nact_iproperties_tab_get_type( void )
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
	static const gchar *thisfn = "nact_iproperties_tab_register_type";
	GType type;

	static const GTypeInfo info = {
		sizeof( NactIPropertiesTabInterface ),
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

	type = g_type_register_static( G_TYPE_INTERFACE, "NactIPropertiesTab", &info, 0 );

	g_type_interface_add_prerequisite( type, BASE_WINDOW_TYPE );

	return( type );
}

static void
interface_base_init( NactIPropertiesTabInterface *klass )
{
	static const gchar *thisfn = "nact_iproperties_tab_interface_base_init";

	if( !st_initialized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		klass->private = g_new0( NactIPropertiesTabInterfacePrivate, 1 );

		st_initialized = TRUE;
	}
}

static void
interface_base_finalize( NactIPropertiesTabInterface *klass )
{
	static const gchar *thisfn = "nact_iproperties_tab_interface_base_finalize";

	if( st_initialized && !st_finalized ){

		g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

		st_finalized = TRUE;

		g_free( klass->private );
	}
}

void
nact_iproperties_tab_initial_load_toplevel( NactIPropertiesTab *instance )
{
	static const gchar *thisfn = "nact_iproperties_tab_initial_load_toplevel";

	g_return_if_fail( NACT_IS_IPROPERTIES_TAB( instance ));

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	}
}

void
nact_iproperties_tab_runtime_init_toplevel( NactIPropertiesTab *instance )
{
	static const gchar *thisfn = "nact_iproperties_tab_runtime_init_toplevel";
	GtkButton *enabled_button;
	GtkWidget *button;
	GtkWidget *label_widget;
	GtkTextBuffer *buffer;

	g_return_if_fail( NACT_IS_IPROPERTIES_TAB( instance ));

	if( st_initialized && !st_finalized ){

		g_debug( "%s: instance=%p", thisfn, ( void * ) instance );

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				MAIN_WINDOW_SIGNAL_SELECTION_CHANGED,
				G_CALLBACK( on_tab_updatable_selection_changed ));

		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( instance ),
				TAB_UPDATABLE_SIGNAL_PROVIDER_CHANGED,
				G_CALLBACK( on_tab_updatable_provider_changed ));

		enabled_button = get_enabled_button( instance );
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( enabled_button ),
				"toggled",
				G_CALLBACK( on_enabled_toggled ));

		label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionDescriptionText" );
		buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( label_widget ));
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( buffer ),
				"changed",
				G_CALLBACK( on_description_changed ));

		button = base_window_get_widget( BASE_WINDOW( instance ), "SuggestedShortcutButton" );
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( button ),
				"clicked",
				G_CALLBACK( on_shortcut_clicked ));

		button = base_window_get_widget( BASE_WINDOW( instance ), "ActionReadonlyButton" );
		base_window_signal_connect(
				BASE_WINDOW( instance ),
				G_OBJECT( button ),
				"toggled",
				G_CALLBACK( on_readonly_toggled ));
	}
}

void
nact_iproperties_tab_all_widgets_showed( NactIPropertiesTab *instance )
{
	static const gchar *thisfn = "nact_iproperties_tab_all_widgets_showed";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IPROPERTIES_TAB( instance ));

	if( st_initialized && !st_finalized ){
	}
}

void
nact_iproperties_tab_dispose( NactIPropertiesTab *instance )
{
	static const gchar *thisfn = "nact_iproperties_tab_dispose";

	g_debug( "%s: instance=%p", thisfn, ( void * ) instance );
	g_return_if_fail( NACT_IS_IPROPERTIES_TAB( instance ));

	if( st_initialized && !st_finalized ){
	}
}

static void
on_tab_updatable_selection_changed( NactIPropertiesTab *instance, gint count_selected )
{
	static const gchar *thisfn = "nact_iproperties_tab_on_tab_updatable_selection_changed";
	NAObjectItem *item;
	gboolean editable;
	gboolean enable_tab;
	GtkNotebook *notebook;
	GtkWidget *page;
	GtkWidget *title_widget, *label_widget, *shortcut_button;
	GtkButton *enabled_button;
	gboolean enabled_item;
	GtkToggleButton *readonly_button;
	GtkTextBuffer *buffer;
	gchar *label, *shortcut;

	g_debug( "%s: instance=%p, count_selected=%d", thisfn, ( void * ) instance, count_selected );
	g_return_if_fail( BASE_IS_WINDOW( instance ));
	g_return_if_fail( NACT_IS_IPROPERTIES_TAB( instance ));

	if( st_initialized && !st_finalized ){

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_SELECTED_ITEM, &item,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		g_return_if_fail( !item || NA_IS_OBJECT_ITEM( item ));

		enable_tab = ( count_selected == 1 );
		nact_main_tab_enable_page( NACT_MAIN_WINDOW( instance ), TAB_PROPERTIES, enable_tab );

		if( enable_tab ){
			st_on_selection_change = TRUE;

			notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( instance ), "MainNotebook" ));
			page = gtk_notebook_get_nth_page( notebook, TAB_ACTION );
			title_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionPropertiesTitle" );
			label_widget = gtk_notebook_get_tab_label( notebook, page );
			if( item && NA_IS_OBJECT_MENU( item )){
				gtk_label_set_label( GTK_LABEL( label_widget ), _( "Me_nu" ));
				gtk_label_set_markup( GTK_LABEL( title_widget ), _( "<b>Menu editable properties</b>" ));
			} else {
				gtk_label_set_label( GTK_LABEL( label_widget ), _( "_Action" ));
				gtk_label_set_markup( GTK_LABEL( title_widget ), _( "<b>Action editable properties</b>" ));
			}

			enabled_button = get_enabled_button( instance );
			enabled_item = item ? na_object_is_enabled( NA_OBJECT_ITEM( item )) : FALSE;
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( enabled_button ), enabled_item );
			nact_gtk_utils_set_editable( GTK_OBJECT( enabled_button ), editable );

			label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionDescriptionText" );
			buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( label_widget ));
			label = item ? na_object_get_description( item ) : g_strdup( "" );
			gtk_text_buffer_set_text( buffer, label, -1 );
			g_free( label );

			shortcut_button = base_window_get_widget( BASE_WINDOW( instance ), "SuggestedShortcutButton" );
			shortcut = na_object_get_shortcut( item );
			gtk_button_set_label( GTK_BUTTON( shortcut_button ), shortcut );
			g_free( shortcut );
			nact_gtk_utils_set_editable( GTK_OBJECT( shortcut_button ), editable );

			/* TODO: don't know how to edit a shortcut for now */
			gtk_widget_set_sensitive( shortcut_button, FALSE );

			/* read-only toggle only indicates the intrinsic writability status of this item
			 * _not_ the writability status of the provider
			 */
			readonly_button = GTK_TOGGLE_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionReadonlyButton" ));
			gtk_toggle_button_set_active( readonly_button, item ? na_object_is_readonly( item ) : FALSE );
			nact_gtk_utils_set_editable( GTK_OBJECT( readonly_button ), FALSE );

			label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionItemID" );
			label = item ? na_object_get_id( item ) : g_strdup( "" );
			gtk_label_set_text( GTK_LABEL( label_widget ), label );
			g_free( label );

			display_provider_name( instance, item );

			st_on_selection_change = FALSE;
		}
	}
}

static void
on_tab_updatable_provider_changed( NactIPropertiesTab *instance, NAObjectItem *item )
{
	static const gchar *thisfn = "nact_iproperties_tab_on_tab_updatable_provider_changed";

	g_debug( "%s: instance=%p, item=%p", thisfn, ( void * ) instance, ( void * ) item );

	if( st_initialized && !st_finalized ){

		display_provider_name( instance, item );
	}
}

static GtkButton *
get_enabled_button( NactIPropertiesTab *instance )
{
	return( GTK_BUTTON( base_window_get_widget( BASE_WINDOW( instance ), "ActionEnabledButton" )));
}

static void
on_enabled_toggled( GtkToggleButton *button, NactIPropertiesTab *instance )
{
	static const gchar *thisfn = "nact_iproperties_tab_on_enabled_toggled";
	NAObjectItem *edited;
	gboolean enabled;
	gboolean editable;

	if( !st_on_selection_change ){
		g_debug( "%s: button=%p, instance=%p", thisfn, ( void * ) button, ( void * ) instance );

		g_object_get(
				G_OBJECT( instance ),
				TAB_UPDATABLE_PROP_SELECTED_ITEM, &edited,
				TAB_UPDATABLE_PROP_EDITABLE, &editable,
				NULL );

		if( edited && NA_IS_OBJECT_ITEM( edited )){

			enabled = gtk_toggle_button_get_active( button );

			if( editable ){
				na_object_set_enabled( edited, enabled );
				g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );

			} else {
				g_signal_handlers_block_by_func(( gpointer ) button, on_enabled_toggled, instance );
				gtk_toggle_button_set_active( button, !enabled );
				g_signal_handlers_unblock_by_func(( gpointer ) button, on_enabled_toggled, instance );
			}
		}
	}
}

static void
on_readonly_toggled( GtkToggleButton *button, NactIPropertiesTab *instance )
{
	static const gchar *thisfn = "nact_iproperties_tab_on_readonly_toggled";
	gboolean active;

	if( !st_on_selection_change ){
		g_debug( "%s: button=%p, instance=%p", thisfn, ( void * ) button, ( void * ) instance );

		active = gtk_toggle_button_get_active( button );

		g_signal_handlers_block_by_func(( gpointer ) button, on_readonly_toggled, instance );
		gtk_toggle_button_set_active( button, !active );
		g_signal_handlers_unblock_by_func(( gpointer ) button, on_readonly_toggled, instance );
	}
}

static void
on_description_changed( GtkTextBuffer *buffer, NactIPropertiesTab *instance )
{
	NAObjectItem *edited;
	GtkTextIter start, end;
	gchar *text;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_SELECTED_ITEM, &edited,
			NULL );

	gtk_text_buffer_get_start_iter( buffer, &start );
	gtk_text_buffer_get_end_iter( buffer, &end );
	text = gtk_text_buffer_get_text( buffer, &start, &end, TRUE );
	na_object_set_description( edited, text );
	g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );
}

static void
on_shortcut_clicked( GtkButton *button, NactIPropertiesTab *instance )
{
	NAObjectItem *item;

	g_object_get(
			G_OBJECT( instance ),
			TAB_UPDATABLE_PROP_SELECTED_ITEM, &item,
			NULL );

	/*g_signal_emit_by_name( G_OBJECT( instance ), TAB_UPDATABLE_SIGNAL_ITEM_UPDATED, edited, FALSE );*/
}

static void
display_provider_name( NactIPropertiesTab *instance, NAObjectItem *item )
{
	GtkWidget *label_widget;
	gchar *label;
	NAIOProvider *provider;

	label_widget = base_window_get_widget( BASE_WINDOW( instance ), "ActionItemProvider" );
	label = NULL;
	if( item ){
		provider = na_object_get_provider( item );
		if( provider ){
			label = na_io_provider_get_name( provider );
		}
	}
	if( !label ){
		label = g_strdup( "" );
	}
	gtk_label_set_text( GTK_LABEL( label_widget ), label );
	g_free( label );
	gtk_widget_set_sensitive( label_widget, item != NULL );
}
