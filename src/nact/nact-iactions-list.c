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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gdk/gdkkeysyms.h>

#include <common/na-action.h>
#include <common/na-pivot.h>

#include "nact-application.h"
#include "nact-iactions-list.h"

/* private interface data
 */
struct NactIActionsListInterfacePrivate {
};

/* column ordering
 */
enum {
	IACTIONS_LIST_ICON_COLUMN = 0,
	IACTIONS_LIST_LABEL_COLUMN,
	IACTIONS_LIST_UUID_COLUMN,
	IACTIONS_LIST_N_COLUMN
};

/* data set against GObject
 */
#define SEND_SELECTION_CHANGED_MESSAGE	"iactions-list-send-selection-changed-message"
#define IS_FILLING_LIST					"iactions-list-is-filling-list"

static GType      register_type( void );
static void       interface_base_init( NactIActionsListInterface *klass );
static void       interface_base_finalize( NactIActionsListInterface *klass );

static void       v_on_selection_changed( GtkTreeSelection *selection, gpointer user_data );
static gboolean   v_on_button_press_event( GtkWidget *widget, GdkEventButton *event, gpointer data );
static gboolean   v_on_key_press_event( GtkWidget *widget, GdkEventKey *event, gpointer data );

static void       do_initial_load_widget( NactWindow *window );
static void       do_runtime_init_widget( NactWindow *window );
static void       do_fill_actions_list( NactWindow *window );
static GtkWidget *get_actions_list_widget( NactWindow *window );

GType
nact_iactions_list_get_type( void )
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
	static const gchar *thisfn = "nact_iactions_list_register_type";
	g_debug( "%s", thisfn );

	static const GTypeInfo info = {
		sizeof( NactIActionsListInterface ),
		( GBaseInitFunc ) interface_base_init,
		( GBaseFinalizeFunc ) interface_base_finalize,
		NULL,
		NULL,
		NULL,
		0,
		0,
		NULL
	};

	GType type = g_type_register_static( G_TYPE_INTERFACE, "NactIActionsList", &info, 0 );

	g_type_interface_add_prerequisite( type, G_TYPE_OBJECT );

	return( type );
}

static void
interface_base_init( NactIActionsListInterface *klass )
{
	static const gchar *thisfn = "nact_iactions_list_interface_base_init";
	static gboolean initialized = FALSE;

	if( !initialized ){
		g_debug( "%s: klass=%p", thisfn, klass );

		klass->private = g_new0( NactIActionsListInterfacePrivate, 1 );

		klass->initial_load_widget = do_initial_load_widget;
		klass->runtime_init_widget = do_runtime_init_widget;
		klass->on_selection_changed = NULL;

		initialized = TRUE;
	}
}

static void
interface_base_finalize( NactIActionsListInterface *klass )
{
	static const gchar *thisfn = "nact_iactions_list_interface_base_finalize";
	static gboolean finalized = FALSE ;

	if( !finalized ){
		g_debug( "%s: klass=%p", thisfn, klass );

		g_free( klass->private );

		finalized = TRUE;
	}
}

/**
 * Allocates and initializes the ActionsList widget.
 */
void
nact_iactions_list_initial_load( NactWindow *window )
{
	g_assert( NACT_IS_IACTIONS_LIST( window ));

	nact_iactions_list_set_send_selection_changed_on_fill_list( window, FALSE );
	nact_iactions_list_set_is_filling_list( window, FALSE );

	if( NACT_IACTIONS_LIST_GET_INTERFACE( window )->initial_load_widget ){
		NACT_IACTIONS_LIST_GET_INTERFACE( window )->initial_load_widget( window );
	} else {
		do_initial_load_widget( window );
	}
}

/**
 * Allocates and initializes the ActionsList widget.
 */
void
nact_iactions_list_runtime_init( NactWindow *window )
{
	g_assert( NACT_IS_IACTIONS_LIST( window ));

	if( NACT_IACTIONS_LIST_GET_INTERFACE( window )->runtime_init_widget ){
		NACT_IACTIONS_LIST_GET_INTERFACE( window )->runtime_init_widget( window );
	} else {
		do_runtime_init_widget( window );
	}
}

/**
 * Fill the listbox with current actions.
 */
void
nact_iactions_list_fill( NactWindow *window )
{
	g_assert( NACT_IS_IACTIONS_LIST( window ));

	nact_iactions_list_set_is_filling_list( window, TRUE );

	if( NACT_IACTIONS_LIST_GET_INTERFACE( window )->fill_actions_list ){
		NACT_IACTIONS_LIST_GET_INTERFACE( window )->fill_actions_list( window );
	} else {
		do_fill_actions_list( window );
	}

	nact_iactions_list_set_is_filling_list( window, FALSE );
}

/**
 * Set the selection to the named action.
 * If not found, we select the first following, else the previous one.
 */
void
nact_iactions_list_set_selection( NactWindow *window, const gchar *uuid, const gchar *label )
{
	static const gchar *thisfn = "nact_iactions_list_set_selection";
	g_debug( "%s: window=%p, uuid=%s, label=%s", thisfn, window, uuid, label );

	GtkWidget *list = get_actions_list_widget( window );
	GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( list ));
	GtkTreeModel *model = gtk_tree_view_get_model( GTK_TREE_VIEW( list ));
	GtkTreeIter iter, previous;

	gtk_tree_selection_unselect_all( selection );

	gboolean iterok = gtk_tree_model_get_iter_first( model, &iter );
	gboolean found = FALSE;
	gchar *iter_uuid, *iter_label;
	gint count = 0;

	while( iterok && !found ){
		count += 1;
		gtk_tree_model_get(
				model,
				&iter,
				IACTIONS_LIST_UUID_COLUMN, &iter_uuid, IACTIONS_LIST_LABEL_COLUMN, &iter_label,
				-1 );

		gint ret_uuid = g_ascii_strcasecmp( iter_uuid, uuid );
		gint ret_label = g_utf8_collate( iter_label, label );
		if(( ret_uuid == 0 && ret_label == 0 ) || ret_label > 0 ){
			gtk_tree_selection_select_iter( selection, &iter );
			found = TRUE;

		} else {
			previous = iter;
		}

		g_free( iter_uuid );
		g_free( iter_label );
		iterok = gtk_tree_model_iter_next( model, &iter );
	}

	if( !found && count ){
		gtk_tree_selection_select_iter( selection, &previous );
	}
}

/**
 * Reset the focus on the ActionsList listbox.
 */
void
nact_iactions_list_set_focus( NactWindow *window )
{
	GtkWidget *list = get_actions_list_widget( window );
	gtk_widget_grab_focus( list );
}

/**
 * Returns the currently selected action.
 */
GObject *
nact_iactions_list_get_selected_action( NactWindow *window )
{
	GObject *action = NULL;

	GtkWidget *list = get_actions_list_widget( window );
	GtkTreeSelection *selection = gtk_tree_view_get_selection( GTK_TREE_VIEW( list ));

	GtkTreeIter iter;
	GtkTreeModel *model;

	if( gtk_tree_selection_get_selected( selection, &model, &iter )){

		gchar *uuid;
		gtk_tree_model_get( model, &iter, IACTIONS_LIST_UUID_COLUMN, &uuid, -1 );
		action = nact_window_get_action( window, uuid );
		g_free (uuid);
	}

	return( action );
}

/**
 * Should the IActionsList interface trigger a 'on_selection_changed'
 * message when the ActionsList list is filled ?
 */
void
nact_iactions_list_set_send_selection_changed_on_fill_list( NactWindow *window, gboolean send_message )
{
	g_assert( NACT_IS_IACTIONS_LIST( window ));
	g_object_set_data( G_OBJECT( window ), SEND_SELECTION_CHANGED_MESSAGE, GINT_TO_POINTER( send_message ));
}

/**
 * Is the IActionsList interface currently filling the ActionsList ?
 */
void
nact_iactions_list_set_is_filling_list( NactWindow *window, gboolean is_filling )
{
	g_assert( NACT_IS_IACTIONS_LIST( window ));
	g_object_set_data( G_OBJECT( window ), IS_FILLING_LIST, GINT_TO_POINTER( is_filling ));
}

static void
v_on_selection_changed( GtkTreeSelection *selection, gpointer user_data )
{
	g_assert( NACT_IS_IACTIONS_LIST( user_data ));
	g_assert( BASE_IS_WINDOW( user_data ));

	NactIActionsList *instance = NACT_IACTIONS_LIST( user_data );

	g_assert( NACT_IS_WINDOW( user_data ));
	gboolean send_message = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( user_data ), SEND_SELECTION_CHANGED_MESSAGE ));
	gboolean is_filling = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( user_data ), IS_FILLING_LIST ));

	if( send_message || !is_filling ){
		if( NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_selection_changed ){
			NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_selection_changed( selection, user_data );
		}
	}
}

static gboolean
v_on_button_press_event( GtkWidget *widget, GdkEventButton *event, gpointer user_data )
{
	/*static const gchar *thisfn = "nact_iactions_list_v_on_button_pres_event";
	g_debug( "%s: widget=%p, event=%p, user_data=%p", thisfn, widget, event, user_data );*/

	g_assert( NACT_IS_IACTIONS_LIST( user_data ));
	g_assert( NACT_IS_WINDOW( user_data ));

	gboolean stop = FALSE;
	NactIActionsList *instance = NACT_IACTIONS_LIST( user_data );

	if( NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_button_press_event ){
		stop = NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_button_press_event( widget, event, user_data );
	}

	if( !stop ){
		if( event->type == GDK_2BUTTON_PRESS ){
			if( NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_double_click ){
				stop = NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_double_click( widget, event, user_data );
			}
		}
	}
	return( stop );
}

static gboolean
v_on_key_press_event( GtkWidget *widget, GdkEventKey *event, gpointer user_data )
{
	static const gchar *thisfn = "nact_iactions_list_v_on_key_pres_event";
	g_debug( "%s: widget=%p, event=%p, user_data=%p", thisfn, widget, event, user_data );

	g_assert( NACT_IS_IACTIONS_LIST( user_data ));
	g_assert( NACT_IS_WINDOW( user_data ));

	gboolean stop = FALSE;
	NactIActionsList *instance = NACT_IACTIONS_LIST( user_data );

	if( NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_key_press_event ){
		stop = NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_key_press_event( widget, event, user_data );
	}

	if( !stop ){
		if( event->keyval == GDK_Return || event->keyval == GDK_KP_Enter ){
			if( NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_enter_key_pressed ){
				stop = NACT_IACTIONS_LIST_GET_INTERFACE( instance )->on_enter_key_pressed( widget, event, user_data );
			}
		}
	}
	return( stop );
}

void
do_initial_load_widget( NactWindow *window )
{
	static const gchar *thisfn = "nact_iactions_list_do_initial_load_widget";
	g_debug( "%s: window=%p", thisfn, window );

	GtkListStore *model;
	GtkTreeViewColumn *column;

	g_assert( BASE_IS_WINDOW( window ));

	GtkWidget *widget = get_actions_list_widget( window );

	/* create the model */
	model = gtk_list_store_new( IACTIONS_LIST_N_COLUMN, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING );
	gtk_tree_view_set_model( GTK_TREE_VIEW( widget ), GTK_TREE_MODEL( model ));
	nact_iactions_list_fill( window );
	g_object_unref( model );

	/* create visible columns on the tree view */
	column = gtk_tree_view_column_new_with_attributes(
			"icon", gtk_cell_renderer_pixbuf_new(), "pixbuf", IACTIONS_LIST_ICON_COLUMN, NULL );
	gtk_tree_view_append_column( GTK_TREE_VIEW( widget ), column );

	column = gtk_tree_view_column_new_with_attributes(
			"label", gtk_cell_renderer_text_new(), "text", IACTIONS_LIST_LABEL_COLUMN, NULL );
	gtk_tree_view_append_column( GTK_TREE_VIEW( widget ), column );
}

void
do_runtime_init_widget( NactWindow *window )
{
	static const gchar *thisfn = "nact_iactions_list_do_runtime_init_widget";
	g_debug( "%s: window=%p", thisfn, window );

	g_assert( BASE_IS_WINDOW( window ));

	GtkWidget *widget = get_actions_list_widget( window );

	/* set up selection */
	g_signal_connect(
			G_OBJECT( gtk_tree_view_get_selection( GTK_TREE_VIEW( widget ))),
			"changed",
			G_CALLBACK( v_on_selection_changed ),
			window );

	/* catch press 'Enter' */
	g_signal_connect(
			G_OBJECT( widget ),
			"key-press-event",
			G_CALLBACK( v_on_key_press_event ),
			window );

	/* catch double-click */
	g_signal_connect(
			G_OBJECT( widget ),
			"button-press-event",
			G_CALLBACK( v_on_button_press_event ),
			window );
}

static void
do_fill_actions_list( NactWindow *window )
{
	static const gchar *thisfn = "nact_iactions_list_do_fill_actions_list";
	g_debug( "%s: window=%p", thisfn, window );

	GtkWidget *widget = get_actions_list_widget( window );
	GtkListStore *model = GTK_LIST_STORE( gtk_tree_view_get_model( GTK_TREE_VIEW( widget )));
	gtk_list_store_clear( model );

	GSList *actions = nact_window_get_actions( window );
	GSList *ia;
	/*g_debug( "%s: actions has %d elements", thisfn, g_slist_length( actions ));*/

	for( ia = actions ; ia != NULL ; ia = ia->next ){
		GtkTreeIter iter;
		GtkStockItem item;
		GdkPixbuf* icon = NULL;

		NAAction *action = NA_ACTION( ia->data );
		gchar *uuid = na_action_get_uuid( action );
		gchar *label = na_action_get_label( action );
		gchar *iconname = na_action_get_icon( action );

		/* TODO: use the same algorythm than Nautilus to find and
		 * display an icon + move the code to NAAction class +
		 * remove na_action_get_verified_icon_name
		 */
		if( iconname ){
			if( gtk_stock_lookup( iconname, &item )){
				icon = gtk_widget_render_icon( widget, iconname, GTK_ICON_SIZE_MENU, NULL );

			} else if( g_file_test( iconname, G_FILE_TEST_EXISTS )
				   && g_file_test( iconname, G_FILE_TEST_IS_REGULAR )){
				gint width;
				gint height;
				GError* error = NULL;

				gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &width, &height);
				icon = gdk_pixbuf_new_from_file_at_size( iconname, width, height, &error );
				if( error ){
					g_warning( "%s: iconname=%s, error=%s", thisfn, iconname, error->message );
					g_error_free( error );
					error = NULL;
					icon = NULL;
				}
			}
		}
		gtk_list_store_append( model, &iter );
		gtk_list_store_set( model, &iter,
				    IACTIONS_LIST_ICON_COLUMN, icon,
				    IACTIONS_LIST_LABEL_COLUMN, label,
				    IACTIONS_LIST_UUID_COLUMN, uuid,
				    -1);

		g_free( iconname );
		g_free( label );
		g_free( uuid );
	}
	/*g_debug( "%s: at end, actions has %d elements", thisfn, g_slist_length( actions ));*/
}

static GtkWidget *
get_actions_list_widget( NactWindow *window )
{
	return( base_window_get_widget( BASE_WINDOW( window ), "ActionsList" ));
}