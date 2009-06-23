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

#include <string.h>
#include <glib/gi18n.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkentry.h>
#include <gtk/gtktogglebutton.h>
#include <glade/glade-xml.h>
#include "nact-editor.h"
#include "nact-profile-editor.h"
#include "nact-utils.h"
#include "nact-prefs.h"
#include "nact.h"

/* gui callback functions */
void profile_field_changed_cb (GObject *object, gpointer user_data);
void path_browse_button_clicked_cb (GtkButton *button, gpointer user_data);
void legend_button_toggled_cb (GtkToggleButton *button, gpointer user_data);
void add_scheme_clicked (GtkWidget* widget, gpointer user_data);
void remove_scheme_clicked (GtkWidget* widget, gpointer user_data);

static void     update_example_label (void);
static gboolean cell_edited (GtkTreeModel *model, const gchar *path_string, const gchar *new_text, gint column);
static void     scheme_desc_edited_cb (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data);
static void     scheme_edited_cb (GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer data);
static void     scheme_selection_toggled_cb (GtkCellRendererToggle *cell_renderer, gchar *path_str, gpointer user_data );

static void
update_example_label (void)
{
	nact_update_example_label( GLADE_EDIT_PROFILE_DIALOG_WIDGET );
}

void
nact_update_example_label( const gchar *dialog )
{
	/*static const char *thisfn = "nact_update_example_label";*/
	GtkWidget* label_widget = nact_get_glade_widget_from ("LabelExample", dialog );
	static gboolean init = FALSE;
	static gchar label_format_string[1024];
	gchar* tmp = nact_utils_parse_parameter( dialog );
	gchar* ex_str;

	if (!init)
	{
		g_stpcpy (label_format_string, gtk_label_get_label (GTK_LABEL (label_widget)));
		init = TRUE;
	}

	/* Convert special xml chars (&, <, >,...) to avoid warnings generated by Pango parser */
	ex_str = g_markup_printf_escaped (label_format_string, tmp);
	gtk_label_set_label (GTK_LABEL (label_widget), ex_str);
	g_free (ex_str);
	g_free (tmp);
}

void
profile_field_changed_cb (GObject *object, gpointer user_data)
{
	GtkWidget* editor = nact_get_glade_widget_from ("EditProfileDialog", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	GtkWidget* command_path = nact_get_glade_widget_from ("CommandPathEntry", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	const gchar *path = gtk_entry_get_text (GTK_ENTRY (command_path));

	update_example_label ();

	if (path && strlen (path) > 0)
		gtk_dialog_set_response_sensitive (GTK_DIALOG (editor), GTK_RESPONSE_OK, TRUE);
	else
		gtk_dialog_set_response_sensitive (GTK_DIALOG (editor), GTK_RESPONSE_OK, FALSE);
}

/*void
example_changed_cb (GObject *object, gpointer user_data)
{
}*/

void
path_browse_button_clicked_cb (GtkButton *button, gpointer user_data)
{
	nact_path_browse_button_clicked_cb( button, user_data, GLADE_EDIT_PROFILE_DIALOG_WIDGET );
}

void
nact_path_browse_button_clicked_cb (GtkButton *button, gpointer user_data, const gchar *dialog )
{
	gchar* last_dir;
	gchar* filename;
	GtkWidget* filechooser = nact_get_glade_widget_from ("FileChooserDialog", GLADE_FILECHOOSER_DIALOG_WIDGET);
	GtkWidget* entry = nact_get_glade_widget_from ("CommandPathEntry", dialog );
	gboolean set_current_location = FALSE;

	filename = (gchar*)gtk_entry_get_text (GTK_ENTRY (entry));
	if (filename != NULL && strlen (filename) > 0)
	{
		set_current_location = gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (filechooser), filename);
	}

	if (!set_current_location)
	{
		last_dir = nact_prefs_get_path_last_browsed_dir ();
		gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (filechooser), last_dir);
		g_free (last_dir);
	}

	switch (gtk_dialog_run (GTK_DIALOG (filechooser)))
	{
		case GTK_RESPONSE_OK :
			last_dir = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (filechooser));
			nact_prefs_set_path_last_browsed_dir (last_dir);
			g_free (last_dir);

			filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (filechooser));
			gtk_entry_set_text (GTK_ENTRY (entry), filename);
			g_free (filename);
		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_DELETE_EVENT:
			gtk_widget_hide (filechooser);
	}
}

void
nact_show_legend_dialog( const gchar *dialog )
{
	GtkWidget* editor = nact_get_glade_widget_from ("EditActionDialog", dialog );
	GtkWidget *legend_dialog = nact_get_glade_widget_from ("LegendDialog", GLADE_LEGEND_DIALOG_WIDGET);
	gtk_window_set_deletable (GTK_WINDOW (legend_dialog), FALSE);
	gtk_window_set_transient_for (GTK_WINDOW (legend_dialog), GTK_WINDOW (editor));
	/* TODO: Get back the last position saved !! */
	gtk_widget_show (legend_dialog);
}

void
nact_hide_legend_dialog( const gchar *dialog )
{
	GtkWidget *legend_dialog = nact_get_glade_widget_from ("LegendDialog", GLADE_LEGEND_DIALOG_WIDGET);
	GtkWidget *legend_button = nact_get_glade_widget_from ("LegendButton", dialog );
	gtk_widget_hide (legend_dialog);
	/* TODO: Save the current position !! */

	/* Set the legend button state consistent for when the dialog is
	 * hidden by another mean (eg. close the edit profile dialog)
	 */
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (legend_button), FALSE);
}

void
legend_button_toggled_cb (GtkToggleButton *button, gpointer user_data)
{
	if (gtk_toggle_button_get_active (button))
	{
		nact_show_legend_dialog( GLADE_EDIT_PROFILE_DIALOG_WIDGET );
	}
	else
	{
		nact_hide_legend_dialog( GLADE_EDIT_PROFILE_DIALOG_WIDGET );
	}
}

/*
 * path_str is the index of the selected row, counted from zero
 */
static void
scheme_selection_toggled_cb (GtkCellRendererToggle *cell_renderer,
													  gchar *path_str,
													  gpointer user_data)
{
	static const char *thisfn = "scheme_selection_toggled_cb";
	GtkTreeIter iter;
	GtkTreePath* path;
	gboolean toggle_state;
	NactCallbackData *scheme_data = ( NactCallbackData * ) user_data;
	GtkTreeModel* model = gtk_tree_view_get_model( GTK_TREE_VIEW( scheme_data->listview ));
	g_debug( "%s: cell_renderer=%p, path_str='%s', scheme_data=%p, dialog='%s', model=%p",
			thisfn, cell_renderer, path_str, user_data, scheme_data->dialog, model );

	path = gtk_tree_path_new_from_string (path_str);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, SCHEMES_CHECKBOX_COLUMN, &toggle_state, -1);

	gtk_list_store_set (GTK_LIST_STORE (model), &iter, SCHEMES_CHECKBOX_COLUMN, !toggle_state, -1);

	/* --> Notice edition change */
	scheme_data->field_changed_cb (G_OBJECT (cell_renderer), NULL);
	scheme_data->update_example_label ();

	gtk_tree_path_free (path);
}

static gboolean
cell_edited (GtkTreeModel		   *model,
             const gchar         *path_string,
             const gchar         *new_text,
				 gint 					column)
{
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	GtkTreeIter iter;
	gchar* old_text;
	gboolean toggle_state;

	gtk_tree_model_get_iter (model, &iter, path);

	gtk_tree_model_get (model, &iter, SCHEMES_CHECKBOX_COLUMN, &toggle_state,
												 column, &old_text, -1);
	g_free (old_text);

	gtk_list_store_set (GTK_LIST_STORE (model), &iter, column,
							  g_strdup (new_text), -1);

	gtk_tree_path_free (path);

	return toggle_state;
}

static void
scheme_edited_cb (GtkCellRendererText *cell,
             const gchar         *path_string,
             const gchar         *new_text,
             gpointer             data)
{
	NactCallbackData *scheme_data = ( NactCallbackData * ) data;
	GtkTreeModel* model = gtk_tree_view_get_model( GTK_TREE_VIEW( scheme_data->listview ));

	if (cell_edited (model, path_string, new_text, SCHEMES_KEYWORD_COLUMN))
	{
		/* --> if column was checked, set the action has edited */
		scheme_data->field_changed_cb (G_OBJECT (cell), NULL);
		scheme_data->update_example_label ();
	}
}

static void
scheme_desc_edited_cb (GtkCellRendererText *cell,
             const gchar         *path_string,
             const gchar         *new_text,
             gpointer             data)
{
	NactCallbackData *scheme_data = ( NactCallbackData * ) data;
	GtkTreeModel* model = gtk_tree_view_get_model( GTK_TREE_VIEW( scheme_data->listview ));
	cell_edited (model, path_string, new_text, SCHEMES_DESC_COLUMN);
}

static void
scheme_list_selection_changed_cb (GtkTreeSelection *selection, gpointer user_data)
{
	NactCallbackData *scheme_data = ( NactCallbackData * ) user_data;
	GtkWidget *delete_button = nact_get_glade_widget_from( "RemoveSchemeButton", scheme_data->dialog );

	if (gtk_tree_selection_count_selected_rows (selection) > 0) {
		gtk_widget_set_sensitive (delete_button, TRUE);
	} else {
		gtk_widget_set_sensitive (delete_button, FALSE);
	}
}

void
add_scheme_clicked (GtkWidget* widget, gpointer user_data)
{
	nact_add_scheme_clicked( widget, user_data, GLADE_EDIT_PROFILE_DIALOG_WIDGET );
}

void
nact_add_scheme_clicked (GtkWidget* widget, gpointer user_data, const gchar* dialog )
{
	GtkWidget* listview = nact_get_glade_widget_from( "SchemesTreeView", dialog );
	GtkTreeModel* model = gtk_tree_view_get_model (GTK_TREE_VIEW (listview));
	GtkTreeIter row;

	gtk_list_store_append (GTK_LIST_STORE (model), &row);
	gtk_list_store_set (GTK_LIST_STORE (model), &row, SCHEMES_CHECKBOX_COLUMN, FALSE,
												/* i18n notes : scheme name set for a new entry in the scheme list */
												SCHEMES_KEYWORD_COLUMN, _("new-scheme"),
												SCHEMES_DESC_COLUMN, _("New Scheme Description"), -1);
}

void
remove_scheme_clicked (GtkWidget* widget, gpointer user_data)
{
	nact_remove_scheme_clicked( widget, user_data, GLADE_EDIT_PROFILE_DIALOG_WIDGET );
}

void
nact_remove_scheme_clicked (GtkWidget* widget, gpointer user_data, const gchar *dialog )
{
	GtkWidget* listview = nact_get_glade_widget_from ("SchemesTreeView", dialog);
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (listview));
	GtkTreeModel* model = gtk_tree_view_get_model (GTK_TREE_VIEW (listview));
	GList* selected_values_path = NULL;
	GtkTreeIter iter;
	GtkTreePath* path;
	GList* list_iter;
	gboolean toggle_state;

	selected_values_path = gtk_tree_selection_get_selected_rows (selection, &model);

	for (list_iter = selected_values_path; list_iter; list_iter = list_iter->next)
	{
		path = (GtkTreePath*)(list_iter->data);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_tree_model_get (model, &iter, SCHEMES_CHECKBOX_COLUMN, &toggle_state, -1);

		gtk_list_store_remove (GTK_LIST_STORE (model), &iter);

		if (toggle_state)
		{
			/* --> if column was checked, set the action has edited */
			profile_field_changed_cb (G_OBJECT (widget), NULL);
			update_example_label ();
		}
	}

	g_list_foreach (selected_values_path, (GFunc)gtk_tree_path_free, NULL);
	g_list_free (selected_values_path);
}

void
nact_create_schemes_selection_list( NactCallbackData *callback_data )
{
	static const char *thisfn = "nact_create_schemes_selection_list";
	g_debug( "%s: callback_data=%p", thisfn, callback_data );

	GtkWidget* listview = nact_get_glade_widget_from ("SchemesTreeView", callback_data->dialog );
	GSList* iter;
	GSList* schemes_list = nact_prefs_get_schemes_list ();
	GtkListStore* model;
	GtkTreeIter row;
	GtkTreeViewColumn *column;
	GtkCellRenderer* toggled_cell;
	GtkCellRenderer* text_cell;

	model = gtk_list_store_new (SCHEMES_N_COLUMN, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING);

	callback_data->listview = listview;

	for (iter = schemes_list; iter; iter = iter->next)
	{
		gchar** tokens = g_strsplit ((gchar*)iter->data, "|", 2);

		gtk_list_store_append (model, &row);
		gtk_list_store_set (model, &row, SCHEMES_CHECKBOX_COLUMN, FALSE,
													SCHEMES_KEYWORD_COLUMN, tokens[0],
												   SCHEMES_DESC_COLUMN, tokens[1], -1);

		g_strfreev (tokens);
	}

	g_slist_foreach (schemes_list, (GFunc)g_free, NULL);
	g_slist_free (schemes_list);

	gtk_tree_view_set_model (GTK_TREE_VIEW (listview), GTK_TREE_MODEL (model));
	g_object_unref (model);

	toggled_cell = gtk_cell_renderer_toggle_new ();

	g_signal_connect (G_OBJECT (toggled_cell), "toggled",
			G_CALLBACK (scheme_selection_toggled_cb), callback_data );

	column = gtk_tree_view_column_new_with_attributes ("",
							   toggled_cell,
							   "active", SCHEMES_CHECKBOX_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listview), column);

	text_cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (text_cell), "editable", TRUE, NULL);

	g_signal_connect (G_OBJECT (text_cell), "edited",
			G_CALLBACK (scheme_edited_cb), callback_data );

	column = gtk_tree_view_column_new_with_attributes (_("Scheme"),
							   text_cell,
							   "text", SCHEMES_KEYWORD_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listview), column);

	text_cell = gtk_cell_renderer_text_new ();
	g_object_set (G_OBJECT (text_cell), "editable", TRUE, NULL);

	g_signal_connect (G_OBJECT (text_cell), "edited",
			G_CALLBACK (scheme_desc_edited_cb), callback_data );

	column = gtk_tree_view_column_new_with_attributes (_("Description"),
							   text_cell,
							   "text", SCHEMES_DESC_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listview), column);

	g_signal_connect (G_OBJECT (gtk_tree_view_get_selection (GTK_TREE_VIEW (listview))), "changed",
			  G_CALLBACK (scheme_list_selection_changed_cb), callback_data );
}

gboolean
nact_reset_schemes_list (GtkTreeModel* scheme_model, GtkTreePath *path,
											GtkTreeIter* iter, gpointer data)
{
	gtk_list_store_set (GTK_LIST_STORE (scheme_model), iter, SCHEMES_CHECKBOX_COLUMN, FALSE, -1);

	return FALSE; /* Don't stop looping */
}

/*static gboolean get_all_schemes_list (GtkTreeModel* scheme_model, GtkTreePath *path,
													  GtkTreeIter* iter, gpointer data)
{
	GSList** list = data;
	gchar* scheme;
	gchar* desc;
	gchar* scheme_desc;

	gtk_tree_model_get (scheme_model, iter, SCHEMES_KEYWORD_COLUMN, &scheme,
														 SCHEMES_DESC_COLUMN, &desc, -1);

	scheme_desc = g_strjoin ("|", scheme, desc, NULL);

	(*list) = g_slist_append ((*list), scheme_desc);

	g_free (scheme);
	g_free (desc);

	return FALSE; // Don't stop looping
}*/

void
nact_set_action_schemes (gchar* action_scheme, GtkTreeModel* scheme_model)
{
	GtkTreeIter iter;
	gboolean iter_ok = FALSE;
	gboolean found = FALSE;
	gchar* scheme;

	iter_ok = gtk_tree_model_get_iter_first (scheme_model, &iter);

	while (iter_ok && !found)
	{
		gtk_tree_model_get (scheme_model, &iter, SCHEMES_KEYWORD_COLUMN, &scheme, -1);

		if (g_ascii_strcasecmp (action_scheme, scheme) == 0)
		{
			gtk_list_store_set (GTK_LIST_STORE (scheme_model), &iter, SCHEMES_CHECKBOX_COLUMN, TRUE, -1);
			found = TRUE;
		}

		g_free (scheme);

		iter_ok = gtk_tree_model_iter_next (scheme_model, &iter);
	}

	if (!found)
	{
		gtk_list_store_append (GTK_LIST_STORE (scheme_model), &iter);
		gtk_list_store_set (GTK_LIST_STORE (scheme_model), &iter, SCHEMES_CHECKBOX_COLUMN, TRUE,
													SCHEMES_KEYWORD_COLUMN, action_scheme,
												   SCHEMES_DESC_COLUMN, "", -1);
	}
}

void
nact_set_action_match_string_list (GtkEntry* entry, GSList* basenames, const gchar* default_string)
{
	GSList* iter;
	gchar* entry_text;
	gchar* tmp;

	iter = basenames;
	if (!iter)
	{
		entry_text = g_strdup (default_string);
	}
	else
	{
		entry_text = g_strdup ((gchar*)iter->data);
		iter = iter->next;

		while (iter)
		{
			tmp = g_strjoin (" ; ", entry_text, (gchar*)iter->data, NULL);
			g_free (entry_text);
			entry_text = tmp;
			iter = iter->next;
		}
	}

	gtk_entry_set_text (entry, entry_text);
}

GSList*
nact_get_action_match_string_list (const gchar* patterns, const gchar* default_string)
{
	gchar** tokens;
	gchar** iter;
	GSList* list = NULL;
	gchar* tmp;
	gchar *source = g_strdup( patterns );

	tmp = g_strstrip( source );
	if (strlen (tmp) == 0)
	{
		list = g_slist_append (list, g_strdup (default_string));
	}
	else
	{
		tokens = g_strsplit( source, ";", -1);

		iter = tokens;
		while (*iter != NULL)
		{
			tmp = g_strstrip (*iter);
			list = g_slist_append (list, g_strdup (tmp));
			iter++;
		}

		g_strfreev (tokens);
	}
	g_free( source );
	return list;
}

static gboolean
open_profile_editor (NautilusActionsConfigAction *action, gchar* profile_name, NautilusActionsConfigActionProfile* action_profile, gboolean is_new)
{
	gboolean ret = FALSE;
	static gboolean init = FALSE;
	GtkWidget* editor;
	GladeXML *gui;
	/*gchar *label;*/
	GList* aligned_widgets = NULL;
	GList* iter;
	GSList* list;
	GtkSizeGroup* label_size_group;
	GtkSizeGroup* button_size_group;
	GtkWidget /* *menu_icon,*/ *scheme_listview;
	/*GtkWidget *menu_label, *menu_tooltip, *menu_profiles_list*/;
	GtkWidget *command_path, *command_params, *test_patterns, *match_case, *test_mimetypes;
	GtkWidget *only_files, *only_folders, *both, *accept_multiple;
	gint width, height /*, x, y*/;
	GtkTreeModel* scheme_model;
	static NactCallbackData *scheme_data = NULL;

	if (!init)
	{
		/* load the GUI */
		gui = nact_get_glade_xml_object (GLADE_EDIT_PROFILE_DIALOG_WIDGET);
		if (!gui) {
			g_error (_("Could not load interface for Nautilus Actions Config Tool"));
			return FALSE;
		}

		scheme_data = g_new0( NactCallbackData, 1 );
		scheme_data->dialog = GLADE_EDIT_PROFILE_DIALOG_WIDGET;
		scheme_data->field_changed_cb = profile_field_changed_cb;
		scheme_data->update_example_label = update_example_label;
		nact_create_schemes_selection_list ( scheme_data );

		glade_xml_signal_autoconnect(gui);

		aligned_widgets = nact_get_glade_widget_prefix_from ("ALabelAlign", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
		label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		for (iter = aligned_widgets; iter; iter = iter->next)
		{
			gtk_size_group_add_widget (label_size_group, GTK_WIDGET (iter->data));
		}

		aligned_widgets = nact_get_glade_widget_prefix_from ("CLabelAlign", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
		label_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		for (iter = aligned_widgets; iter; iter = iter->next)
		{
			gtk_size_group_add_widget (label_size_group, GTK_WIDGET (iter->data));
		}
		button_size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
		gtk_size_group_add_widget (button_size_group,
											nact_get_glade_widget_from ("PathBrowseButton",
																		GLADE_EDIT_PROFILE_DIALOG_WIDGET));
		gtk_size_group_add_widget (button_size_group,
											nact_get_glade_widget_from ("LegendButton",
																		GLADE_EDIT_PROFILE_DIALOG_WIDGET));
		/* free memory */
		g_object_unref (gui);
		init = TRUE;
	}

	editor = nact_get_glade_widget_from ("EditProfileDialog", GLADE_EDIT_PROFILE_DIALOG_WIDGET);

	if (is_new)
	{
		gtk_window_set_title (GTK_WINDOW (editor), _("Add a New Profile"));
	}
	else
	{
		gchar* title = g_strdup_printf (_("Edit Profile \"%s\""), profile_name);
		gtk_window_set_title (GTK_WINDOW (editor), title);
		g_free (title);
	}

	/* Get the default dialog size */
	gtk_window_get_default_size (GTK_WINDOW (editor), &width, &height);

	/* FIXME: update preference data for profile editor dialog

	// Override with preferred one, if any
	nact_prefs_get_edit_dialog_size (&width, &height);

	gtk_window_resize (GTK_WINDOW (editor), width, height);

	if (nact_prefs_get_edit_dialog_position (&x, &y))
	{
		gtk_window_move (GTK_WINDOW (editor), x, y);
	}
	*/

	command_path = nact_get_glade_widget_from ("CommandPathEntry", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	/*printf ("path : %s\n", action_profile->path);*/
	gtk_entry_set_text (GTK_ENTRY (command_path), action_profile->path);
	/*g_print ("toto42\n");*/

	command_params = nact_get_glade_widget_from ("CommandParamsEntry", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	/*g_print ("toto43\n");
	printf ("params : %s\n", action_profile->parameters);*/
	gtk_entry_set_text (GTK_ENTRY (command_params), action_profile->parameters);
	/*g_print ("toto44\n");*/

	test_patterns = nact_get_glade_widget_from ("PatternEntry", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	nact_set_action_match_string_list (GTK_ENTRY (test_patterns), action_profile->basenames, "*");

	match_case = nact_get_glade_widget_from ("MatchCaseButton", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (match_case), action_profile->match_case);

	test_mimetypes = nact_get_glade_widget_from ("MimeTypeEntry", GLADE_EDIT_PROFILE_DIALOG_WIDGET);

	nact_set_action_match_string_list (GTK_ENTRY (test_mimetypes), action_profile->mimetypes, "*/*");

	only_folders = nact_get_glade_widget_from ("OnlyFoldersButton", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (only_folders), action_profile->is_dir);

	only_files = nact_get_glade_widget_from ("OnlyFilesButton", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (only_files), action_profile->is_file);

	both = nact_get_glade_widget_from ("BothButton", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (both), action_profile->is_file && action_profile->is_dir);

	accept_multiple = nact_get_glade_widget_from ("AcceptMultipleButton", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (accept_multiple), action_profile->accept_multiple_files);

	scheme_listview = nact_get_glade_widget_from ("SchemesTreeView", GLADE_EDIT_PROFILE_DIALOG_WIDGET);
	scheme_model = gtk_tree_view_get_model (GTK_TREE_VIEW (scheme_listview));
	gtk_tree_model_foreach (scheme_model, (GtkTreeModelForeachFunc) nact_reset_schemes_list, NULL);
	g_slist_foreach (action_profile->schemes, (GFunc) nact_set_action_schemes, scheme_model);

	/* run the dialog */
	gtk_dialog_set_response_sensitive (GTK_DIALOG (editor), GTK_RESPONSE_OK, FALSE);
	profile_field_changed_cb( NULL, NULL );

	switch (gtk_dialog_run (GTK_DIALOG (editor))) {
	case GTK_RESPONSE_OK :
		nautilus_actions_config_action_profile_set_path (action_profile, gtk_entry_get_text (GTK_ENTRY (command_path)));
		nautilus_actions_config_action_profile_set_parameters (action_profile, gtk_entry_get_text (GTK_ENTRY (command_params)));

		list = nact_get_action_match_string_list (gtk_entry_get_text (GTK_ENTRY (test_patterns)), "*");
		nautilus_actions_config_action_profile_set_basenames (action_profile, list);
		g_slist_foreach (list, (GFunc) g_free, NULL);
		g_slist_free (list);

		nautilus_actions_config_action_profile_set_match_case (
			action_profile, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (match_case)));

		list = nact_get_action_match_string_list (gtk_entry_get_text (GTK_ENTRY (test_mimetypes)), "*/*");
		nautilus_actions_config_action_profile_set_mimetypes (action_profile, list);
		g_slist_foreach (list, (GFunc) g_free, NULL);
		g_slist_free (list);

		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (only_files))) {
			nautilus_actions_config_action_profile_set_is_file (action_profile, TRUE);
			nautilus_actions_config_action_profile_set_is_dir (action_profile, FALSE);
		} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (only_folders))) {
			nautilus_actions_config_action_profile_set_is_file (action_profile, FALSE);
			nautilus_actions_config_action_profile_set_is_dir (action_profile, TRUE);
		} else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (both))) {
			nautilus_actions_config_action_profile_set_is_file (action_profile, TRUE);
			nautilus_actions_config_action_profile_set_is_dir (action_profile, TRUE);
		}

		nautilus_actions_config_action_profile_set_accept_multiple (
			action_profile, gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (accept_multiple)));

		list = NULL;
		gtk_tree_model_foreach (scheme_model, (GtkTreeModelForeachFunc)nact_utils_get_action_schemes_list, &list);
		nautilus_actions_config_action_profile_set_schemes (action_profile, list);
		g_slist_foreach (list, (GFunc) g_free, NULL);
		g_slist_free (list);

		if (is_new)
		{
			ret = nautilus_actions_config_action_add_profile (action, profile_name, action_profile, NULL);
		}
		else
		{
			nautilus_actions_config_action_replace_profile (action, profile_name, action_profile);
			ret = TRUE;
		}
		break;
	case GTK_RESPONSE_DELETE_EVENT:
	case GTK_RESPONSE_CANCEL :
		if (!is_new)
		{
			/* Edition mode : Free the duplicated profile */
			nautilus_actions_config_action_profile_free (action_profile);
		}
		ret = FALSE;
		break;
	}

	/*
	// Save preferences
	list = NULL;
	gtk_tree_model_foreach (scheme_model, (GtkTreeModelForeachFunc)get_all_schemes_list, &list);
	nact_prefs_set_schemes_list (list);
	g_slist_foreach (list, (GFunc) g_free, NULL);
	g_slist_free (list);

	nact_prefs_set_edit_dialog_size (GTK_WINDOW (editor));
	nact_prefs_set_edit_dialog_position (GTK_WINDOW (editor));
	*/

	nact_hide_legend_dialog( GLADE_EDIT_PROFILE_DIALOG_WIDGET );
	gtk_widget_hide (editor);

	return ret;
}

gboolean
nact_profile_editor_new_profile (NautilusActionsConfigAction* action)
{
	gboolean val = FALSE;
	gchar* new_profile_name;
	gchar* new_profile_desc_name;
	nautilus_actions_config_action_get_new_default_profile_name (action, &new_profile_name, &new_profile_desc_name);
	NautilusActionsConfigActionProfile *action_profile = nautilus_actions_config_action_profile_new_default ();
	nautilus_actions_config_action_profile_set_desc_name (action_profile, new_profile_desc_name);

	/*printf ("Profile Name : %s (%s)\n", new_profile_desc_name, new_profile_name);*/
	val = open_profile_editor (action, new_profile_name, action_profile, TRUE);
	g_free (new_profile_name);
	g_free (new_profile_desc_name);

	return val;
}

gboolean
nact_profile_editor_edit_profile (NautilusActionsConfigAction *action, gchar* profile_name, NautilusActionsConfigActionProfile* action_profile)
{
	return open_profile_editor (action, profile_name, action_profile, FALSE);
}