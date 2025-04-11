/*
 * pan-window.c
 *
 * Copyright 2025 Dilnavas Roshan <dilnavasroshan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"
#include <glib/gi18n.h>

#include "pan-document.h"
#include "pan-canvas.h"
#include "pan-window.h"
#include "pan-annot-view.h"

struct _PanWindow
{
    AdwApplicationWindow parent_instance;

    PanDocument *document;
    GSettings *settings;

    /* Template widgets */
    GtkColorButton *color_button;

    GtkScale *radius_scale;
    GtkScale *alpha_scale;

    AdwWindowTitle *window_title;

    GtkWidget *zoom_in_button;
    GtkWidget *zoom_out_button;
    GtkWidget *zoom_fit_button;
    GtkWidget *zoom_original_button;

    GtkWidget *first_button;
    GtkWidget *prev_button;
    GtkWidget *next_button;
    GtkWidget *last_button;

    GtkListView *file_list_view;
    GtkColumnView *annot_column_view;

    PanCanvas *canvas;
};

static void pan_window_dispose                (GObject *object);
static void pan_window_color_set_cb           (GtkColorButton *self,
                                               gpointer        user_data);
static void pan_window_radius_change_value_cb (GtkRange *range,
                                               gpointer  user_data);
static void pan_window_alpha_change_value_cb  (GtkRange *range,
                                               gpointer  user_data);
static void pan_window_new_action             (GSimpleAction *action,
                                               GVariant      *parameters,
                                               gpointer       user_data);
static void pan_window_open_action            (GSimpleAction *action,
                                               GVariant      *parameters,
                                               gpointer       user_data);
static void pan_window_save_action            (GSimpleAction *action,
                                               GVariant      *parameters,
                                               gpointer       user_data);
static void pan_window_save_as_action         (GSimpleAction *action,
                                               GVariant      *parameters,
                                               gpointer       user_data);
static void pan_window_folder_opened          (GObject      *source,
                                               GAsyncResult *result,
                                               gpointer      user_data);
static void pan_window_save_dialog_cb         (GObject      *source,
                                               GAsyncResult *result,
                                               gpointer      user_data);
static void pan_window_undo_action            (GSimpleAction *action,
                                               GVariant      *parameters,
                                               gpointer       user_data);
static void pan_window_redo_action            (GSimpleAction *action,
                                               GVariant      *parameters,
                                               gpointer       user_data);
static void file_list_selection_changed_cb    (GtkSelectionModel *selection_model,
                                               guint              position,
                                               gint               n_items,
                                               gpointer           user_data);
static void record_changed                    (PanWindow          *self,
                                               GtkSingleSelection *selection_model);

static void load_settings                     (PanWindow *self);
static void set_enable_action                 (PanWindow   *window,
                                               const gchar *action_name,
                                               gboolean     value);
static gboolean pan_window_close_request      (GtkWindow *window);

G_DEFINE_FINAL_TYPE (PanWindow, pan_window, ADW_TYPE_APPLICATION_WINDOW)

static GActionEntry window_actions[] =
{
    {"new",     pan_window_new_action    },
    {"open",    pan_window_open_action   },
    {"save",    pan_window_save_action   },
    {"save_as", pan_window_save_as_action},
    {"undo",    pan_window_undo_action,  },
    {"redo",    pan_window_redo_action   }
};

static void
pan_window_class_init (PanWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
    GtkWindowClass *window_class = GTK_WINDOW_CLASS (klass);

    object_class->dispose = pan_window_dispose;

    gtk_widget_class_set_template_from_resource (widget_class,
                                                 "/me/scratchspace/Pan/pan-window.ui");

    gtk_widget_class_bind_template_child (widget_class, PanWindow, canvas);

    gtk_widget_class_bind_template_child (widget_class, PanWindow, window_title);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, color_button);

    gtk_widget_class_bind_template_child (widget_class, PanWindow, radius_scale);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, alpha_scale);

    gtk_widget_class_bind_template_child (widget_class, PanWindow, zoom_in_button);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, zoom_out_button);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, zoom_fit_button);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, zoom_original_button);

    gtk_widget_class_bind_template_child (widget_class, PanWindow, first_button);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, prev_button);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, next_button);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, last_button);

    gtk_widget_class_bind_template_child (widget_class, PanWindow, file_list_view);
    gtk_widget_class_bind_template_child (widget_class, PanWindow, annot_column_view);

    window_class->close_request = pan_window_close_request;
}

static void
pan_window_init (PanWindow *self)
{
    g_type_ensure (PAN_TYPE_CANVAS);
    gtk_widget_init_template (GTK_WIDGET (self));

    g_action_map_add_action_entries (G_ACTION_MAP (self), window_actions,
                                     G_N_ELEMENTS (window_actions), self);

    g_signal_connect (self->color_button, "color-set", G_CALLBACK (pan_window_color_set_cb), self);
    g_signal_connect (self->radius_scale, "value-changed", G_CALLBACK (pan_window_radius_change_value_cb), self->canvas);
    g_signal_connect (self->alpha_scale, "value-changed", G_CALLBACK (pan_window_alpha_change_value_cb), self->canvas);

    g_signal_connect_swapped (self->zoom_in_button, "clicked", G_CALLBACK (pan_canvas_zoom_in), self->canvas);
    g_signal_connect_swapped (self->zoom_out_button, "clicked", G_CALLBACK (pan_canvas_zoom_out), self->canvas);
    g_signal_connect_swapped (self->zoom_original_button, "clicked", G_CALLBACK (pan_canvas_zoom_original), self->canvas);
    g_signal_connect_swapped (self->zoom_fit_button, "clicked", G_CALLBACK (pan_canvas_zoom_fit), self->canvas);
    g_signal_connect_swapped (self->first_button, "clicked", G_CALLBACK (pan_canvas_first), self->canvas);
    g_signal_connect_swapped (self->prev_button, "clicked", G_CALLBACK (pan_canvas_prev), self->canvas);
    g_signal_connect_swapped (self->next_button, "clicked", G_CALLBACK (pan_canvas_next), self->canvas);
    g_signal_connect_swapped (self->last_button, "clicked", G_CALLBACK (pan_canvas_last), self->canvas);

    set_enable_action (self, "undo", FALSE);
    set_enable_action (self, "redo", FALSE);
    set_enable_action (self, "save", FALSE);

    gtk_widget_set_sensitive (self->next_button, FALSE);
    gtk_widget_set_sensitive (self->prev_button, FALSE);
    gtk_widget_set_sensitive (self->first_button, FALSE);
    gtk_widget_set_sensitive (self->last_button, FALSE);
    gtk_widget_set_sensitive (self->zoom_in_button, FALSE);
    gtk_widget_set_sensitive (self->zoom_out_button, FALSE);
    gtk_widget_set_sensitive (self->zoom_original_button, FALSE);
    gtk_widget_set_sensitive (self->zoom_fit_button, FALSE);

    load_settings (self);
}

static void
pan_window_dispose (GObject *object)
{
    PanWindow *window = PAN_WINDOW (object);

    /* TODO: should free everything here. */

    g_clear_object (&window->document);
    g_clear_object (&window->settings);

    G_OBJECT_CLASS (pan_window_parent_class)->dispose (object);
}

static void
load_settings (PanWindow *self)
{
    GtkAdjustment *adjustment;
    gdouble r, g, b;
    GdkRGBA color;
    g_autoptr (GVariant) color_value = NULL;
    g_autoptr (GVariant) size = NULL;

    self->settings = g_settings_new ("me.scratchspace.Pan");

    g_object_get (self->radius_scale, "adjustment", &adjustment, NULL);
    g_settings_bind (self->settings, "radius", adjustment, "value", G_SETTINGS_BIND_DEFAULT);

    g_object_get (self->alpha_scale, "adjustment", &adjustment, NULL);
    g_settings_bind (self->settings, "alpha", adjustment, "value", G_SETTINGS_BIND_DEFAULT);

    color_value = g_settings_get_value (self->settings, "color");
    g_variant_get (color_value, "(ddd)", &r, &g, &b);
    color.red   = CLAMP (r, 0.0, 1.0);
    color.green = CLAMP (g, 0.0, 1.0);
    color.blue  = CLAMP (b, 0.0, 1.0);
    color.alpha = 1.0;
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (self->color_button), &color);
    G_GNUC_END_IGNORE_DEPRECATIONS

    pan_canvas_set_alpha (self->canvas, gtk_range_get_value (GTK_RANGE (self->alpha_scale)));
    pan_canvas_set_color (self->canvas, &color);
}

static void
file_list_selection_changed_cb (GtkSelectionModel *selection_model,
                                guint              position,
                                gint               n_items,
                                gpointer           user_data)
{
    record_changed (user_data, GTK_SINGLE_SELECTION (selection_model));
}

static void
record_changed (PanWindow          *self,
                GtkSingleSelection *selection_model)
{
    PanRecord *record;
    char *filename;

    record = gtk_single_selection_get_selected_item (selection_model);
    filename = pan_record_filename (record);
    adw_window_title_set_subtitle (self->window_title, filename);

    gtk_column_view_set_model (self->annot_column_view,
                               GTK_SELECTION_MODEL (pan_canvas_get_annot_selection_model (self->canvas)));

}

static void
pan_window_color_set_cb (GtkColorButton *self, gpointer user_data)
{
    GdkRGBA color;
    PanWindow *pan_window = user_data;
    GVariant *color_value = NULL;

    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (self), &color);
    G_GNUC_END_IGNORE_DEPRECATIONS
    pan_canvas_set_color (pan_window->canvas, &color);

    color_value = g_variant_new ("(ddd)", color.red, color.green, color.blue);
    g_settings_set_value (pan_window->settings, "color", color_value);
}

static void
pan_window_radius_change_value_cb (GtkRange *range, gpointer user_data)
{
    PanCanvas *canvas = user_data;
    pan_canvas_set_radius (canvas, gtk_range_get_value (range));
}

static void
pan_window_alpha_change_value_cb (GtkRange *range, gpointer user_data)
{
    PanCanvas *canvas = user_data;
    pan_canvas_set_alpha (canvas, gtk_range_get_value (range));
}

static void
pan_window_folder_opened (GObject      *source,
                          GAsyncResult *result,
                          gpointer      user_data)
{
    GFile *file;
    GError *error = NULL;
    GtkSingleSelection *record_selection;
    PanWindow *window;

    window = user_data;

    file = gtk_file_dialog_select_folder_finish (GTK_FILE_DIALOG (source),
                                                 result, &error);
    if (!file) {
        g_error_free (error);
        return;
    }

    window->document = pan_document_new (file);
    pan_canvas_set_document (window->canvas, window->document);
    record_selection = pan_canvas_get_record_selection_model (window->canvas);
    g_signal_connect (GTK_SELECTION_MODEL (record_selection), "selection-changed",
                      G_CALLBACK (file_list_selection_changed_cb), window);
    gtk_list_view_set_model (window->file_list_view, GTK_SELECTION_MODEL (record_selection));

    record_changed (window, record_selection);

    set_enable_action (window, "undo", TRUE);
    set_enable_action (window, "redo", TRUE);
    set_enable_action (window, "save", TRUE);

    gtk_widget_set_sensitive (window->next_button, TRUE);
    gtk_widget_set_sensitive (window->prev_button, TRUE);
    gtk_widget_set_sensitive (window->first_button, TRUE);
    gtk_widget_set_sensitive (window->last_button, TRUE);
    gtk_widget_set_sensitive (window->zoom_in_button, TRUE);
    gtk_widget_set_sensitive (window->zoom_out_button, TRUE);
    gtk_widget_set_sensitive (window->zoom_original_button, TRUE);
    gtk_widget_set_sensitive (window->zoom_fit_button, TRUE);
}

static void
pan_window_file_opened (GObject      *source,
                          GAsyncResult *result,
                          gpointer      user_data)
{
    GFile *file;
    GError *error = NULL;
    PanWindow *window;
    gchar *path;
    GtkSingleSelection *record_selection;
    window = user_data;

    file = gtk_file_dialog_open_finish (GTK_FILE_DIALOG (source),
                                                 result, &error);
    if (!file) {
        g_error_free (error);
        return;
    }

    path = g_file_get_path (file);
    window->document = pan_document_open (path);
    pan_canvas_set_document (window->canvas, window->document);
    record_selection = pan_canvas_get_record_selection_model (window->canvas);
    g_signal_connect (GTK_SELECTION_MODEL (record_selection), "selection-changed",
                      G_CALLBACK (file_list_selection_changed_cb), window);
    gtk_list_view_set_model (window->file_list_view, GTK_SELECTION_MODEL (record_selection));

    record_changed (window, record_selection);
    set_enable_action (window, "undo", TRUE);
    set_enable_action (window, "redo", TRUE);
    set_enable_action (window, "save", TRUE);

    gtk_widget_set_sensitive (window->next_button, TRUE);
    gtk_widget_set_sensitive (window->prev_button, TRUE);
    gtk_widget_set_sensitive (window->first_button, TRUE);
    gtk_widget_set_sensitive (window->last_button, TRUE);
    gtk_widget_set_sensitive (window->zoom_in_button, TRUE);
    gtk_widget_set_sensitive (window->zoom_out_button, TRUE);
    gtk_widget_set_sensitive (window->zoom_original_button, TRUE);
    gtk_widget_set_sensitive (window->zoom_fit_button, TRUE);
}

static void
pan_window_new_action (GSimpleAction *action,
                       GVariant      *parameters,
                       gpointer       user_data)
{
    GtkFileDialog *file_dialog;

    file_dialog = gtk_file_dialog_new ();
    gtk_file_dialog_select_folder (file_dialog, GTK_WINDOW (user_data),
                                   NULL, pan_window_folder_opened, user_data);
    g_object_unref (G_OBJECT (file_dialog));
}

static void
pan_window_open_action (GSimpleAction *action,
                        GVariant      *parameters,
                        gpointer       user_data)
{
    GtkFileDialog *file_dialog;
    GListStore *filters;
    GtkFileFilter *json_filter;

    filters = g_list_store_new (GTK_TYPE_FILE_FILTER);

    json_filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (json_filter, "JSON");
    gtk_file_filter_add_mime_type (json_filter, "text/json");
    gtk_file_filter_add_pattern (json_filter, "*.json");
    gtk_file_filter_add_suffix (json_filter, "json");

    g_list_store_append (filters, json_filter);

    file_dialog = gtk_file_dialog_new ();
    gtk_file_dialog_set_filters (file_dialog, G_LIST_MODEL (filters));
    gtk_file_dialog_open (file_dialog, GTK_WINDOW (user_data),
                                   NULL, pan_window_file_opened, user_data);
    g_object_unref (G_OBJECT (file_dialog));
}

static void
pan_window_save_action (GSimpleAction *action,
                       GVariant       *parameters,
                       gpointer        user_data)
{
    GtkFileDialog *save_dialog;
    GListStore *filters;
    GtkFileFilter *json_filter;

    filters = g_list_store_new (GTK_TYPE_FILE_FILTER);

    json_filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (json_filter, "JSON");
    gtk_file_filter_add_mime_type (json_filter, "text/json");
    gtk_file_filter_add_pattern (json_filter, "*.json");
    gtk_file_filter_add_suffix (json_filter, "json");

    g_list_store_append (filters, json_filter);

    save_dialog = gtk_file_dialog_new ();
    gtk_file_dialog_set_filters (save_dialog, G_LIST_MODEL (filters));
    gtk_file_dialog_set_initial_name (save_dialog, "annotations.json");
    gtk_file_dialog_save (save_dialog, GTK_WINDOW (user_data), NULL,
                         pan_window_save_dialog_cb, user_data);
    g_object_unref (save_dialog);
}

static void
pan_window_save_as_action (GSimpleAction *action,
                           GVariant      *parameters,
                           gpointer       user_data)
{
    g_print ("save as action activated");
}

static void
pan_window_undo_action (GSimpleAction *action,
                        GVariant      *parameters,
                        gpointer       user_data)
{
    PanWindow *window = user_data;

    pan_canvas_undo (window->canvas);
}

static void pan_window_redo_action (GSimpleAction *action,
                                    GVariant      *parameters,
                                    gpointer       user_data)
{
    PanWindow *window = user_data;

    pan_canvas_redo (window->canvas);
}

static void
set_enable_action (PanWindow   *window,
                   const gchar *action_name,
                   gboolean     value)
{
    GAction *action;

    action = g_action_map_lookup_action (G_ACTION_MAP (window), action_name);
    g_assert (action);
    g_simple_action_set_enabled (G_SIMPLE_ACTION (action), value);
}

static void
pan_window_save_dialog_cb (GObject       *source,
                           GAsyncResult  *result,
                           gpointer       user_data)
{
    GFile *file;
    GError *error = NULL;
    PanWindow *window = user_data;
    gchar *path;

    file = gtk_file_dialog_save_finish (GTK_FILE_DIALOG (source), result, &error);
    if (!file) return;
    path = g_file_get_path (file);
    pan_document_save (window->document, path);
    g_object_unref (file);
    g_free (path);
}

static void
alert_dialog_cb (AdwAlertDialog *dialog,
                 gchar          *response,
                 gpointer        user_data)
{
    if (!g_strcmp0 (response, "save")) {

    } else if (!g_strcmp0 (response, "discard")) {
        g_application_quit (g_application_get_default ());
    }
}

void
pan_window_close (PanWindow *self)
{
    AdwDialog *dialog;

    dialog = adw_alert_dialog_new (_("Save Changes"), NULL);
    adw_alert_dialog_format_body (ADW_ALERT_DIALOG (dialog), _("You have unsaved changes. Do you want to save it before exiting?"));
    adw_alert_dialog_add_responses (ADW_ALERT_DIALOG (dialog), "cancel", _("Cancel"), "discard", _("Discard"), "save", _("Save") , NULL);
    adw_alert_dialog_set_response_appearance (ADW_ALERT_DIALOG (dialog), "discard", ADW_RESPONSE_DESTRUCTIVE);
    adw_alert_dialog_set_default_response (ADW_ALERT_DIALOG (dialog), "cancel");
    adw_alert_dialog_set_close_response (ADW_ALERT_DIALOG (dialog), "cancel");
    adw_alert_dialog_set_prefer_wide_layout (ADW_ALERT_DIALOG (dialog), TRUE);
    g_signal_connect (dialog, "response", G_CALLBACK (alert_dialog_cb), self);
    adw_dialog_present (dialog, GTK_WIDGET (self));
}

static gboolean
pan_window_close_request (GtkWindow *window)
{
    pan_window_close (PAN_WINDOW (window));

    return TRUE;
}

