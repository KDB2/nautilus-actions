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

#include <string.h>
#include <uuid/uuid.h>

#include "na-action.h"
#include "na-gconf.h"
#include "na-pivot.h"
#include "na-iio-provider.h"
#include "na-ipivot-container.h"
#include "na-utils.h"

/* private class data
 */
struct NAPivotClassPrivate {
};

/* private instance data
 */
struct NAPivotPrivate {
	gboolean dispose_has_run;

	/* list of instances to be notified of an action modification
	 */
	GSList  *notified;

	/* list of interface providers
	 * needs to be in the instance rather than in the class to be able
	 * to pass NAPivot object to the IO provider, so that the later
	 * is able to have access to the former (and its list of actions)
	 */
	GSList  *providers;

	/* list of actions
	 */
	GSList  *actions;
};

/* We have a double stage notification system :
 *
 * 1. When the storage subsystems detects a change on an action, it
 *    must emit the "notify_pivot_of_action_changed" signal to notify
 *    NAPivot of this change ; when this signal is received, NAPivot
 *    then updates accordingly the list of actions it maintains.
 *
 * 2. When NAPivot has successfully updated its list of actions, it has
 *    to notify its consumers (its 'containers') in order they update
 *    themselves.
 */
enum {
	ACTION_CHANGED,
	LAST_SIGNAL
};

static GObjectClass *st_parent_class = NULL;
static gint          st_signals[ LAST_SIGNAL ] = { 0 };
static GTimeVal      st_last_event;
static guint         st_event_source_id = 0;
static gint          st_timeout_usec = 500000;

static GType    register_type( void );
static void     class_init( NAPivotClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static GSList  *register_interface_providers( const NAPivot *pivot );
static void     instance_dispose( GObject *object );
static void     free_containers( GSList *list );
static void     instance_finalize( GObject *object );

static void     action_changed_handler( NAPivot *pivot, gpointer user_data );
static gboolean on_actions_changed_timeout( gpointer user_data );
static gulong   time_val_diff( const GTimeVal *recent, const GTimeVal *old );

GType
na_pivot_get_type( void )
{
	static GType object_type = 0;

	if( !object_type ){
		object_type = register_type();
	}

	return( object_type );
}

static GType
register_type( void )
{
	static GTypeInfo info = {
		sizeof( NAPivotClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NAPivot ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	return( g_type_register_static( G_TYPE_OBJECT, "NAPivot", &info, 0 ));
}

static void
class_init( NAPivotClass *klass )
{
	static const gchar *thisfn = "na_pivot_class_init";
	g_debug( "%s: klass=%p", thisfn, klass );

	st_parent_class = g_type_class_peek_parent( klass );

	GObjectClass *object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NAPivotClassPrivate, 1 );

	/* register the signal and its default handler
	 * this signal should be sent by the IIOProviders when an actions
	 * has changed in the underlying storage subsystem
	 */
	st_signals[ ACTION_CHANGED ] = g_signal_new_class_handler(
				NA_IIO_PROVIDER_SIGNAL_ACTION_CHANGED,
				G_TYPE_FROM_CLASS( klass ),
				G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
				( GCallback ) action_changed_handler,
				NULL,
				NULL,
				g_cclosure_marshal_VOID__POINTER,
				G_TYPE_NONE,
				1,
				G_TYPE_POINTER
	);
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "na_pivot_instance_init";
	g_debug( "%s: instance=%p, klass=%p", thisfn, instance, klass );

	g_assert( NA_IS_PIVOT( instance ));
	NAPivot* self = NA_PIVOT( instance );

	self->private = g_new0( NAPivotPrivate, 1 );
	self->private->dispose_has_run = FALSE;
	self->private->providers = register_interface_providers( self );
	self->private->actions = na_iio_provider_read_actions( G_OBJECT( self ));
}

static GSList *
register_interface_providers( const NAPivot *pivot )
{
	static const gchar *thisfn = "na_pivot_register_interface_providers";
	g_debug( "%s", thisfn );

	GSList *list = NULL;

	list = g_slist_prepend( list, na_gconf_new( G_OBJECT( pivot )));

	return( list );
}

static void
instance_dispose( GObject *object )
{
	static const gchar *thisfn = "na_pivot_instance_dispose";
	g_debug( "%s: object=%p", thisfn, object );

	g_assert( NA_IS_PIVOT( object ));
	NAPivot *self = NA_PIVOT( object );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		/* release list of containers to be notified */
		free_containers( self->private->notified );

		/* release list of actions */
		na_pivot_free_actions( self->private->actions );
		self->private->actions = NULL;

		/* chain up to the parent class */
		G_OBJECT_CLASS( st_parent_class )->dispose( object );
	}
}

static void
free_containers( GSList *containers )
{
	GSList *ic;
	for( ic = containers ; ic ; ic = ic->next )
		;
	g_slist_free( containers );
}

static void
instance_finalize( GObject *object )
{
	static const gchar *thisfn = "na_pivot_instance_finalize";
	g_debug( "%s: object=%p", thisfn, object );

	g_assert( NA_IS_PIVOT( object ));
	NAPivot *self = ( NAPivot * ) object;

	/* release the interface providers */
	GSList *ip;
	for( ip = self->private->providers ; ip ; ip = ip->next ){
		g_object_unref( G_OBJECT( ip->data ));
	}
	g_slist_free( self->private->providers );
	self->private->providers = NULL;

	g_free( self->private );

	/* chain call to parent class */
	if((( GObjectClass * ) st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( object );
	}
}

/**
 * Allocates a new NAPivot object.
 *
 * @target: the GObject which will handled Nautilus notification, and
 * should be notified when an actions is added, modified or removed in
 * one of the underlying storage subsystems.
 *
 * The target object will receive a "notify_nautilus_of_action_changed"
 * message, without any parameter.
 */
NAPivot *
na_pivot_new( const GObject *target )
{
	NAPivot *pivot = g_object_new( NA_PIVOT_TYPE, NULL );

	if( target ){
		pivot->private->notified = g_slist_prepend( pivot->private->notified, ( gpointer ) target );
	}

	return( pivot );
}

/**
 * Returns the list of providers of the required interface.
 *
 * This function is called by interfaces API in order to find the
 * list of providers registered for this given interface.
 *
 * @pivot: this instance.
 *
 * @type: the type of searched interface.
 */
GSList *
na_pivot_get_providers( const NAPivot *pivot, GType type )
{
	static const gchar *thisfn = "na_pivot_get_providers";
	g_debug( "%s", thisfn );

	g_assert( NA_IS_PIVOT( pivot ));

	GSList *list = NULL;
	GSList *ip;
	for( ip = pivot->private->providers ; ip ; ip = ip->next ){
		if( G_TYPE_CHECK_INSTANCE_TYPE( G_OBJECT( ip->data ), type )){
			list = g_slist_prepend( list, ip->data );
		}
	}

	return( list );
}

/**
 * Return the list of actions.
 *
 * @pivot: this NAPivot object.
 *
 * The returned list is owned by this NAPivot object, and should not
 * be freed, nor unref by the caller.
 */
GSList *
na_pivot_get_actions( const NAPivot *pivot )
{
	g_assert( NA_IS_PIVOT( pivot ));
	return( pivot->private->actions );
}

/**
 * Free a list of actions.
 *
 * @list: the GSList of NAActions to be freed.
 */
void
na_pivot_free_actions( GSList *actions )
{
	GSList *ia;
	for( ia = actions ; ia ; ia = ia->next ){
		g_object_unref( NA_ACTION( ia->data ));
	}
	g_slist_free( actions );
}

/**
 * Return the specified action.
 *
 * @pivot: this NAPivot object.
 *
 * @uuid: required globally unique identifier (uuid).
 *
 * Returns the specified NAAction object, or NULL if not found.
 *
 * The returned pointer is owned by NAPivot, and should not be freed
 * nor unref by the caller.
 */
GObject *
na_pivot_get_action( NAPivot *pivot, const gchar *uuid )
{
	GSList *ia;
	NAAction *act;
	GObject *found = NULL;
	uuid_t uua, uub;
	gchar *uuid_act;

	g_assert( NA_IS_PIVOT( pivot ));

	uuid_parse( uuid, uua );
	for( ia = pivot->private->actions ; ia ; ia = ia->next ){
		act = NA_ACTION( ia->data );
		uuid_act = na_action_get_uuid( act );
		uuid_parse( uuid_act, uub );
		g_free( uuid_act );
		if( !uuid_compare( uua, uub )){
			found = G_OBJECT( act );
			break;
		}
	}

	return( found );
}

/**
 * Write an action.
 *
 * @pivot: this NAPivot object.
 *
 * @action: action to be written by the storage subsystem.
 *
 * @message: the I/O provider can allocate and store here an error
 * message.
 *
 * Returns TRUE if the write is successfull, FALSE else.
 */
gboolean
na_pivot_write_action( NAPivot *pivot, const GObject *action, gchar **message )
{
	g_assert( NA_IS_PIVOT( pivot ));
	g_assert( NA_IS_ACTION( action ));
	g_assert( message );
	return( na_iio_provider_write_action( G_OBJECT( pivot ), action, message ));
}

/*
 * this handler is trigerred by IIOProviders when an action is changed
 * in the underlying storage subsystems
 * we don't care of updating our internal list with each and every
 * atomic modification
 * instead we wait for the end of notifications serie, and then reload
 * the whole list of actions
 */
static void
action_changed_handler( NAPivot *self, gpointer user_data  )
{
	/*static const gchar *thisfn = "na_pivot_action_changed_handler";
	g_debug( "%s: self=%p, data=%p", thisfn, self, user_data );*/

	g_assert( NA_IS_PIVOT( self ));
	g_assert( user_data );
	if( self->private->dispose_has_run ){
		return;
	}

	/* set a timeout to notify nautilus at the end of the serie */
	g_get_current_time( &st_last_event );
	if( !st_event_source_id ){
		st_event_source_id = g_timeout_add_seconds( 1, ( GSourceFunc ) on_actions_changed_timeout, self );
	}
}

/*
 * this timer is set when we receive the first event of a serie
 * we continue to loop until last event is at least one half of a
 * second old
 *
 * there is no race condition here as we are not multithreaded
 * or .. is there ?
 */
static gboolean
on_actions_changed_timeout( gpointer user_data )
{
	/*static const gchar *thisfn = "na_pivot_on_actions_changed_timeout";
	g_debug( "%s: pivot=%p", thisfn, user_data );*/

	GTimeVal now;

	g_assert( NA_IS_PIVOT( user_data ));
	NAPivot *pivot = NA_PIVOT( user_data );

	g_get_current_time( &now );
	gulong diff = time_val_diff( &now, &st_last_event );
	if( diff < st_timeout_usec ){
		return( TRUE );
	}

	na_pivot_free_actions( pivot->private->actions );
	pivot->private->actions = na_iio_provider_read_actions( G_OBJECT( pivot ));

	GSList *ic;
	for( ic = pivot->private->notified ; ic ; ic = ic->next ){
		na_ipivot_container_notify( NA_IPIVOT_CONTAINER( ic->data ));
	}

	st_event_source_id = 0;
	return( FALSE );
}

/*
 * returns the difference in microseconds.
 */
static gulong
time_val_diff( const GTimeVal *recent, const GTimeVal *old )
{
	gulong microsec = 1000000 * ( recent->tv_sec - old->tv_sec );
	microsec += recent->tv_usec  - old->tv_usec;
	return( microsec );
}

/**
 * Free a NAPivotValue structure and its content.
 */
void
na_pivot_free_notify( NAPivotNotify *npn )
{
	if( npn ){
		if( npn->type ){
			switch( npn->type ){

				case NA_PIVOT_STR:
					g_free(( gchar * ) npn->data );
					break;

				case NA_PIVOT_BOOL:
					break;

				case NA_PIVOT_STRLIST:
					na_utils_free_string_list(( GSList * ) npn->data );
					break;

				default:
					g_debug( "na_pivot_free_notify: uuid=%s, profile=%s, parm=%s, type=%d",
							npn->uuid, npn->profile, npn->parm, npn->type );
					g_assert_not_reached();
					break;
			}
		}
		g_free( npn->uuid );
		g_free( npn->profile );
		g_free( npn->parm );
		g_free( npn );
	}
}

/**
 * Records a container which has to be notified of a modification of
 * the list of actions.
 */
void
na_pivot_add_notified( NAPivot *pivot, GObject *container )
{
	pivot->private->notified = g_slist_prepend( pivot->private->notified, container );
}
