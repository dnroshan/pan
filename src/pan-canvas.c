/*
 * pan-widget.c
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
#include "pan-canvas.h"
#include "pan-action.h"
#include "pan-action-create.h"
#include "pan-action-move.h"
#include "pan-action-delete.h"

#define ZOOM_DELTA          0.1
#define MAX_ZOOM_FACTOR     10.0
#define MIN_ZOOM_FACTOR     0.1
#define BOX_PADDING         5

struct _PanCanvas
{
    GtkWidget parent;

    gdouble radius;
    GdkTexture *image;

    GdkRGBA color;
    GdkRGBA hover_color;
    GdkRGBA selection_color;
    GdkRGBA accent_color;

    guint annot_idx;

    gint mode;
    gint state;
    gboolean is_dragging;

    PanDocument *document;
    guint record_idx;
    guint n_records;

    PanRecord *selected_record;
    PanAnnot *selected_annot;
    PanAnnot *hover_annot;

    GtkAdjustment *hadjustment;
    GtkAdjustment *vadjustment;
    GtkScrollablePolicy hscroll_policy;
    GtkScrollablePolicy vscroll_policy;

    GdkCursor *normal_cursor;
    GdkCursor *hand_cursor;
    GdkCursor *move_cursor;

    GtkSingleSelection *record_selection;
    GtkSingleSelection *annot_selection;

    GtkWidget *context_menu;

    guint prev_x, prev_y;
    guint old_x, old_y;

    gfloat zoom_factor;

    GSList *undo_stack;
    GSList *redo_stack;

    AdwStyleManager *style_manager;
};

enum
{
    PROP_ZERO,
    PROP_HADJUSTMENT,
    PROP_VADJUSTMENT,
    PROP_HSCROLL_POLICY,
    PROP_VSCROLL_POLICY,
    PROP_RADIUS,
    PROP_DOCUMENT,
    N_PROPS
};

static void                  pan_canvas_get_property                               (GObject    *object,
                                                                                    guint       prop_id,
                                                                                    GValue     *value,
                                                                                    GParamSpec   *pspec);
static void                  pan_canvas_set_property                               (GObject      *object,
                                                                                    guint         prop_id,
                                                                                    const GValue *value,
                                                                                    GParamSpec   *pspec);
static void                  pan_canvas_dispose                                    (GObject *object);
static void                  pan_canvas_finalize                                   (GObject *object);
static void                  pan_canvas_measure                                    (GtkWidget     *widget,
                                                                                    GtkOrientation orientation,
                                                                                    gint           for_size,
                                                                                    gint          *minimum,
                                                                                    gint          *natural,
                                                                                    gint          *minimum_bseline,
                                                                                    gint          *natural_baseline);
static void                  pan_canvas_snapshot                                   (GtkWidget   *widget,
                                                                                    GtkSnapshot *snapshot);
static void                  pan_canvas_size_allocate                              (GtkWidget *widget,
                                                                                    gint       width,
                                                                                    gint       height,
                                                                                    gint       baseline);
static void                  pan_canvas_button_press_cb                            (PanCanvas *self,
                                                                                    gint       n_press,
                                                                                    gdouble    x,
                                                                                    gdouble    y,
                                                                                    gpointer   user_data);
static void                  pan_canvas_sec_button_press_cb                        (PanCanvas *self,
                                                                                    gint       n_press,
                                                                                    gdouble    x,
                                                                                    gdouble    y,
                                                                                    gpointer   user_data);
static void                  pan_canvas_button_released_cb                         (PanCanvas *self,
                                                                                    gint       n_press,
                                                                                    gdouble    x,
                                                                                    gdouble    y,
                                                                                    gpointer   user_data);
static void                  pan_canvas_pointer_motion_cb                          (PanCanvas *self,
                                                                                    gdouble    x,
                                                                                    gdouble    y,
                                                                                    gpointer   user_data);
static void                  pan_canvas_pointer_enter_cb                           (PanCanvas *self,
                                                                                    gdouble    x,
                                                                                    gdouble    y,
                                                                                    gpointer   user_data);
static void                  pan_canvas_pointer_leave_cb                           (PanCanvas *self,
                                                                                    gdouble    x,
                                                                                    gdouble    y,
                                                                                    gpointer   user_data);
static void                  pan_canvas_set_adjustment                             (PanCanvas     *self,
                                                                                    GtkAdjustment *adjustment,
                                                                                    GtkOrientation orientation);
static void                  pan_canvas_disct_adjustment                           (PanCanvas     *self,
                                                                                    GtkOrientation orientation);
static void                  pan_canvas_adjustment_cb                              (GtkAdjustment *adjustment,
                                                                                    gpointer       user_data);
static void                  pan_canvas_create_action_cb                           (GtkWidget     *widget,
                                                                                    const char    *action_name,
                                                                                    GVariant      *parameter);
static void                  pan_canvas_delete_action_cb                           (GtkWidget   *widget,
                                                                                    const char  *action_name,
                                                                                    GVariant    *parameter);
static gboolean              pan_canvas_key_press_cb                               (PanCanvas      *self,
                                                                                    guint           keyval,
                                                                                    guint           keycode,
                                                                                    GdkModifierType state,
                                                                                    gpointer        user_data);
static void                  accent_color_notify_cb                                (GObject    *self,
                                                                                    GParamSpec *pspec,
                                                                                    gpointer    user_data);
static void                  pan_widget_annot_selection_changed_cb                 (GtkSelectionModel *model,
                                                                                    guint              position,
                                                                                    guint              n_items,
                                                                                    gpointer           user_data);
static void                  set_up_context_menu                                   (PanCanvas *self);
static void                  load_record                                           (PanCanvas *self);
static void                  load_image                                            (PanCanvas *self,
                                                                                    gchar     *img_path);
static GtkSizeRequestMode    pan_canvas_get_request_mode                           (GtkWidget *widget);
static GdkCursor            *load_cursor                                           (const gchar *resource_path,
                                                                                    guint        hotspot_x,
                                                                                    guint        hotspot_y);
static void                  stack_push_action                                     (GSList    **stack,
                                                                                    PanAction  *action);
static PanAction            *stack_pop_action                                      (GSList **stack);
static void                  stack_clear                                           (GSList **stack);

G_DEFINE_FINAL_TYPE_WITH_CODE (PanCanvas, pan_canvas, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_SCROLLABLE, NULL))

static void
pan_canvas_class_init (PanCanvasClass *klass)
{
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;

    object_class = G_OBJECT_CLASS (klass);
    widget_class = GTK_WIDGET_CLASS (klass);

    object_class->get_property = pan_canvas_get_property;
    object_class->set_property = pan_canvas_set_property;
    object_class->dispose = pan_canvas_dispose;
    object_class->finalize = pan_canvas_finalize;

    widget_class->size_allocate = pan_canvas_size_allocate;
    widget_class->get_request_mode = pan_canvas_get_request_mode;
    widget_class->measure = pan_canvas_measure;
    widget_class->snapshot = pan_canvas_snapshot;

    g_object_class_override_property (object_class, PROP_HADJUSTMENT, "hadjustment");
    g_object_class_override_property (object_class, PROP_VADJUSTMENT, "vadjustment");
    g_object_class_override_property (object_class, PROP_HSCROLL_POLICY, "hscroll-policy");
    g_object_class_override_property (object_class, PROP_VSCROLL_POLICY, "vscroll-policy");

    g_object_class_install_property (object_class, PROP_RADIUS,
                                     g_param_spec_uint ("radius", NULL, NULL,
                                                        0, G_MAXUINT, 10,
                                                        G_PARAM_READWRITE));

    g_object_class_install_property (object_class, PROP_DOCUMENT,
                                     g_param_spec_object ("document", NULL, NULL,
                                                          PAN_TYPE_DOCUMENT,
                                                          G_PARAM_READWRITE));

    gtk_widget_class_install_action (widget_class, "pan-widget.create",
                                     NULL, pan_canvas_create_action_cb);
    gtk_widget_class_install_action (widget_class, "pan-widget.delete",
                                     NULL, pan_canvas_delete_action_cb);
}

static void
pan_canvas_init (PanCanvas *self)
{
    GtkEventController *key_controller;
    GtkEventController *pointer_controller;
    GtkGesture *button_controller;
    GtkGesture *sec_button_controller;
    AdwAccentColor accent_color;

    gtk_widget_set_focusable (GTK_WIDGET (self), TRUE);
    gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

    key_controller = gtk_event_controller_key_new ();
    gtk_widget_add_controller (GTK_WIDGET (self), key_controller);
    g_signal_connect_swapped (key_controller, "key-pressed", G_CALLBACK (pan_canvas_key_press_cb), self);

    button_controller = gtk_gesture_click_new ();
    gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (button_controller));
    g_signal_connect_swapped (button_controller, "pressed", G_CALLBACK (pan_canvas_button_press_cb), self);
    g_signal_connect_swapped (button_controller, "released", G_CALLBACK (pan_canvas_button_released_cb), self);

    sec_button_controller = gtk_gesture_click_new ();
    gtk_gesture_single_set_button (GTK_GESTURE_SINGLE (sec_button_controller), GDK_BUTTON_SECONDARY);
    gtk_widget_add_controller (GTK_WIDGET (self), GTK_EVENT_CONTROLLER (sec_button_controller));
    g_signal_connect_swapped (sec_button_controller, "pressed", G_CALLBACK (pan_canvas_sec_button_press_cb), self);

    pointer_controller = gtk_event_controller_motion_new ();
    gtk_widget_add_controller (GTK_WIDGET (self), pointer_controller);
    g_signal_connect_swapped (pointer_controller, "motion", G_CALLBACK (pan_canvas_pointer_motion_cb), self);
    g_signal_connect_swapped (pointer_controller, "enter", G_CALLBACK (pan_canvas_pointer_enter_cb), self);
    g_signal_connect_swapped (pointer_controller, "leave", G_CALLBACK (pan_canvas_pointer_leave_cb), self);

    self->annot_idx   = 0;
    self->radius      = 10.0;
    self->zoom_factor = 1.0;

    self->color.red   = 1.0;
    self->color.green = 0.0;
    self->color.blue  = 0.0;
    self->color.alpha = 0.8;

    self->selection_color.red   = 0.2;
    self->selection_color.green = 0.2;
    self->selection_color.blue  = 0.2;
    self->selection_color.alpha = 0.8;

    self->hover_color.red   = 0.2;
    self->hover_color.blue  = 0.2;
    self->hover_color.green = 0.2;
    self->hover_color.alpha = 0.8;

    self->image            = NULL;
    self->hadjustment      = NULL;
    self->vadjustment      = NULL;
    self->document         = NULL;
    self->record_selection = NULL;
    self->annot_selection  = NULL;

    self->is_dragging = FALSE;

    self->undo_stack = NULL;
    self->redo_stack = NULL;

    self->normal_cursor = gdk_cursor_new_from_name ("crosshair", NULL);
    self->hand_cursor   = gdk_cursor_new_from_name ("grab", NULL);
    self->move_cursor   = gdk_cursor_new_from_name ("grabbing", NULL);

    gtk_widget_set_cursor (GTK_WIDGET (self), self->normal_cursor);

    set_up_context_menu (self);

    self->style_manager = adw_style_manager_get_default ();
    accent_color = adw_style_manager_get_accent_color (self->style_manager);
    adw_accent_color_to_rgba (accent_color, &self->accent_color);
    g_signal_connect (self->style_manager, "notify::accent-color", G_CALLBACK (accent_color_notify_cb), self);
}

static void
pan_canvas_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    PanCanvas *canvas;

    canvas = PAN_CANVAS (object);
    switch (prop_id) {
    case PROP_HADJUSTMENT:
        g_value_set_object (value, canvas->hadjustment);
        break;
    case PROP_VADJUSTMENT:
        g_value_set_object (value, canvas->vadjustment);
        break;
    case PROP_RADIUS:
        g_value_set_uint (value, canvas->radius);
        break;
    case PROP_HSCROLL_POLICY:
        g_value_set_enum (value, canvas->hscroll_policy);
        break;
    case PROP_VSCROLL_POLICY:
        g_value_set_enum (value, canvas->vscroll_policy);
        break;
    case PROP_DOCUMENT:
        g_value_set_object (value, canvas->document);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
pan_canvas_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    PanCanvas *canvas;

    canvas = PAN_CANVAS (object);
    switch (prop_id) {
    case PROP_HADJUSTMENT:
        pan_canvas_set_adjustment (canvas, g_value_get_object (value),
                                   GTK_ORIENTATION_HORIZONTAL);
        break;
    case PROP_VADJUSTMENT:
        pan_canvas_set_adjustment (canvas, g_value_get_object (value),
                                   GTK_ORIENTATION_VERTICAL);
        break;
    case PROP_RADIUS:
        canvas->radius = g_value_get_uint (value);
        break;
    case PROP_HSCROLL_POLICY:
        canvas->hscroll_policy = g_value_get_enum (value);
        break;
    case PROP_VSCROLL_POLICY:
        canvas->vscroll_policy = g_value_get_enum (value);
        break;
    case PROP_DOCUMENT:
        pan_canvas_set_document (canvas, g_value_get_object (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
pan_canvas_dispose (GObject *object)
{
    PanCanvas *canvas;

    canvas = PAN_CANVAS (object);

    g_clear_pointer (&canvas->context_menu, gtk_widget_unparent);
    g_clear_object (&canvas->hadjustment);
    g_clear_object (&canvas->vadjustment);
    g_clear_object (&canvas->normal_cursor);
    g_clear_object (&canvas->hand_cursor);
    g_clear_object (&canvas->move_cursor);
    g_clear_object (&canvas->image);
    g_clear_object (&canvas->document);
    g_clear_object (&canvas->record_selection);
    g_clear_object (&canvas->annot_selection);

    G_OBJECT_CLASS (pan_canvas_parent_class)->dispose (object);
}

static void
pan_canvas_finalize (GObject *object)
{
    PanCanvas *canvas;

    canvas = PAN_CANVAS (object);

    stack_clear (&canvas->undo_stack);
    stack_clear (&canvas->redo_stack);

    G_OBJECT_CLASS (pan_canvas_parent_class)->finalize (object);
}

static void
pan_canvas_set_adjustment (PanCanvas     *self,
                           GtkAdjustment *adjustment,
                           GtkOrientation orientation)
{
    if (!adjustment)
        adjustment = gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    pan_canvas_disct_adjustment (self, orientation);
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
        self->hadjustment = adjustment;
    else
        self->vadjustment = adjustment;

    g_object_ref_sink (adjustment);
    g_signal_connect (adjustment, "value-changed",
                      G_CALLBACK (pan_canvas_adjustment_cb), self);
}

static void
pan_canvas_disct_adjustment (PanCanvas     *self,
                             GtkOrientation orientation)
{
    GtkAdjustment *adjustment;

    adjustment = orientation == GTK_ORIENTATION_HORIZONTAL ? self->hadjustment
                                                           : self->vadjustment;

    if (adjustment) {
        g_signal_handlers_disconnect_by_func (adjustment,
                                              pan_canvas_adjustment_cb, self);
        g_object_unref (adjustment);
    }
}

static void
accent_color_notify_cb (GObject *self,
                        GParamSpec *pspec,
                        gpointer user_data)
{
    AdwAccentColor accent_color;
    PanCanvas *canvas = user_data;

    accent_color = adw_style_manager_get_accent_color (canvas->style_manager);
    adw_accent_color_to_rgba (accent_color, &canvas->accent_color);
    gtk_widget_queue_draw (GTK_WIDGET (canvas));
}

static void
pan_canvas_adjustment_cb (GtkAdjustment *adjustment,
                          gpointer user_data)
{
    gtk_widget_queue_allocate (GTK_WIDGET (user_data));
}

static GtkSizeRequestMode
pan_canvas_get_request_mode (GtkWidget *widget)
{
    return GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}

static void
pan_canvas_measure (GtkWidget     *widget,
                    GtkOrientation orientation,
                    gint           for_size,
                    gint          *minimum,
                    gint          *natural,
                    gint          *minimum_bseline,
                    gint          *natural_baseline)
{
    PanCanvas *canvas = PAN_CANVAS (widget);

    if (!canvas->image) {
        *minimum = -1;
        return;
    }

    if (orientation == GTK_ORIENTATION_HORIZONTAL) {
        *minimum = *natural = gdk_texture_get_width (canvas->image);
    }
}

static void
pan_canvas_snapshot (GtkWidget   *self,
                     GtkSnapshot *snapshot)
{
    GskPathBuilder *path_builder;
    GskPath *path;
    GskStroke *stroke;
    PanCanvas *canvas;
    gint width, height;
    gint scroll_x, scroll_y;
    guint n_annots;
    GListStore *annot_store;
    PanAnnot *annot;
    guint x, y;
    const gfloat dash= 1.0;

    canvas = PAN_CANVAS (self);
    if (!canvas->document)
        return;

    scroll_x = -gtk_adjustment_get_value (canvas->hadjustment);
    scroll_y = -gtk_adjustment_get_value (canvas->vadjustment);

    gtk_snapshot_scale (snapshot,
                        canvas->zoom_factor, canvas->zoom_factor);

    if (canvas->image) {
        width = gdk_texture_get_width (canvas->image);
        height = gdk_texture_get_height (canvas->image);
        gtk_snapshot_append_texture (snapshot, canvas->image,
                                     &GRAPHENE_RECT_INIT (scroll_x, scroll_y,
                                                          width, height));
    }

    annot_store = pan_record_annots (canvas->selected_record);
    n_annots = g_list_model_get_n_items (G_LIST_MODEL (annot_store));
    for (guint i = 0; i < n_annots; i++) {
        annot = g_list_model_get_item (G_LIST_MODEL (annot_store), i);
        path_builder = gsk_path_builder_new ();
        pan_annot_get_pos (annot, &x, &y);
        x += scroll_x;
        y += scroll_y;
        gsk_path_builder_add_circle (path_builder, &GRAPHENE_POINT_INIT (x, y),
                                     canvas->radius);
        path = gsk_path_builder_free_to_path (path_builder);
        gtk_snapshot_append_fill (snapshot, path, GSK_FILL_RULE_WINDING,
                                  &canvas->color);
        gsk_path_unref (path);
    }

    if (canvas->selected_annot) {
        path_builder = gsk_path_builder_new ();
        pan_annot_get_pos (canvas->selected_annot, &x, &y);
        x += scroll_x;
        y += scroll_y;
        gsk_path_builder_add_rect (path_builder,
                                   &GRAPHENE_RECT_INIT (x - canvas->radius - BOX_PADDING,
                                                        y - canvas->radius - BOX_PADDING,
                                                        2 * (canvas->radius + BOX_PADDING),
                                                        2 * (canvas->radius + BOX_PADDING)));
        path = gsk_path_builder_free_to_path (path_builder);
        stroke = gsk_stroke_new (3);
        gsk_stroke_set_dash (stroke, &dash, 1);
        gtk_snapshot_append_stroke (snapshot, path, stroke, &canvas->accent_color);
        gsk_path_unref (path);
        gsk_stroke_free (stroke);
    }

    if (canvas->hover_annot && canvas->hover_annot != canvas->selected_annot) {
        path_builder = gsk_path_builder_new ();
        pan_annot_get_pos (canvas->hover_annot, &x, &y);
        x += scroll_x;
        y += scroll_y;
        gsk_path_builder_add_circle (path_builder, &GRAPHENE_POINT_INIT (x, y),
                                     canvas->radius);
        path = gsk_path_builder_free_to_path (path_builder);
        stroke = gsk_stroke_new (5);
        gtk_snapshot_append_stroke (snapshot, path, stroke, &canvas->accent_color);
        gsk_path_unref (path);
        gsk_stroke_free (stroke);
    }
}

static void
pan_canvas_size_allocate (GtkWidget *widget,
                          gint       width,
                          gint       height,
                          gint       baseline)
{
    PanCanvas *canvas;
    int img_width, img_height;
    int value;

    canvas = PAN_CANVAS (widget);
    if (!canvas->image)
        return;

    img_width = gdk_texture_get_width (canvas->image);
    img_height = gdk_texture_get_height (canvas->image);

    img_width = (int) (img_width * canvas->zoom_factor);
    img_height = (int) (img_height * canvas->zoom_factor);

    value = gtk_adjustment_get_value (canvas->hadjustment);
    gtk_adjustment_configure (canvas->hadjustment, value, 0, img_width,
                              width * 0.1, width, width);

    value = gtk_adjustment_get_value (canvas->vadjustment);
    gtk_adjustment_configure (canvas->vadjustment, value, 0, img_height,
                              height * 0.1, height, height);
}

static void
delete_annot (PanCanvas *self)
{
    GListStore *annots_store;
    PanAction *action;
    int pos;

    annots_store = pan_record_annots (self->selected_record);
    pos = gtk_single_selection_get_selected (self->annot_selection);
    action = pan_action_delete_new (self->selected_record, self->selected_annot, pos);
    stack_clear (&self->redo_stack);
    stack_push_action (&self->undo_stack, action);
    g_list_store_remove (annots_store, pos);
    self->selected_annot = NULL;
    self->hover_annot = NULL;
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

static gboolean
pan_canvas_key_press_cb (PanCanvas      *self,
                         guint           keyval,
                         guint           keycode,
                         GdkModifierType state,
                         gpointer        user_data)
{
    guint dx = 0,
          dy = 0;

    if (!self->selected_record)
        return FALSE;

    switch (keyval) {
    case GDK_KEY_Up:
        dy = -1;
        break;
    case GDK_KEY_Down:
        dy = 1;
        break;
    case GDK_KEY_Left:
        dx = -1;
        break;
    case GDK_KEY_Right:
        dx = 1;
        break;
    case GDK_KEY_Delete:
        delete_annot (self);
        return TRUE;
    default:
        return FALSE;
    }

    pan_annot_translate (self->selected_annot, dx, dy);
    gtk_widget_queue_draw (GTK_WIDGET (self));
    return TRUE;
}

static void
pan_canvas_button_press_cb (PanCanvas *self,
                            gint       n_press,
                            gdouble    x,
                            gdouble    y,
                            gpointer   user_data)
{
    PanAnnot *annot;
    GListStore *annot_store;
    guint n_annots;
    guint annot_x, annot_y;
    PanAction *action;
    int scroll_x, scroll_y;

    if (!self->document)
        return;

    scroll_x = gtk_adjustment_get_value (self->hadjustment);
    scroll_y = gtk_adjustment_get_value (self->vadjustment);

    x = (int) (x / self->zoom_factor + scroll_x);
    y = (int) (y / self->zoom_factor + scroll_y);

    gtk_widget_grab_focus (GTK_WIDGET (self));

    annot_store = pan_record_annots (self->selected_record);
    n_annots = g_list_model_get_n_items (G_LIST_MODEL (annot_store));
    for (guint i = 0; i < n_annots; i++) {
        annot = g_list_model_get_item (G_LIST_MODEL (annot_store), i);
        pan_annot_get_pos (annot, &annot_x, &annot_y);
        if (graphene_point_near (&GRAPHENE_POINT_INIT (x, y),
                                 &GRAPHENE_POINT_INIT (annot_x, annot_y),
                                 self->radius)) {
            self->selected_annot = annot;
            self->is_dragging = TRUE;
            self->prev_x = x;
            self->prev_y = y;
            self->old_x = x;
            self->old_y = y;
            gtk_selection_model_select_item (GTK_SELECTION_MODEL (self->annot_selection), i, TRUE);
            gtk_widget_set_cursor (GTK_WIDGET (self), self->move_cursor);
            gtk_widget_queue_draw (GTK_WIDGET (self));
            return;
        }
    }

    annot = pan_annot_new (x, y);
    g_list_store_append (annot_store, annot);
    action = pan_action_create_new (self->selected_record, annot);
    stack_push_action (&self->undo_stack, action);
    stack_clear (&self->redo_stack);
    self->selected_annot = annot;
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

static void
pan_canvas_button_released_cb (PanCanvas *self,
                               gint       n_press,
                               gdouble    x,
                               gdouble    y,
                               gpointer   user_data)
{
    PanAction *action;
    guint new_x, new_y;

    if (self->is_dragging) {
        self->is_dragging = FALSE;
        gtk_widget_set_cursor (GTK_WIDGET (self), self->hand_cursor);

        if (self->old_x != x || self->old_y != y) {
            pan_annot_get_pos (self->selected_annot, &new_x, &new_y);
            action = pan_action_move_new (self->selected_annot, self->old_x, self->old_y, new_x, new_y);
            stack_push_action (&self->undo_stack, action);
            stack_clear (&self->redo_stack);
        }
    }
}

static void
pan_canvas_pointer_motion_cb (PanCanvas *self,
                              gdouble    x,
                              gdouble    y,
                              gpointer   user_data)
{
    gint scroll_x, scroll_y;
    PanAnnot *annot;
    GListStore *annot_store;
    guint dx, dy;
    guint annot_x, annot_y;
    guint n_annots;

    if (!self->document || !self->selected_record)
        return;

    scroll_x = gtk_adjustment_get_value (self->hadjustment);
    scroll_y = gtk_adjustment_get_value (self->vadjustment);

    x = (int) (x / self->zoom_factor + scroll_x);
    y = (int) (y / self->zoom_factor + scroll_y);

    if (self->is_dragging) {
        dx = x - self->prev_x;
        dy = y - self->prev_y;
        self->prev_x = x;
        self->prev_y = y;
        pan_annot_translate (self->selected_annot, dx, dy);
        gtk_widget_queue_draw (GTK_WIDGET (self));
        return;
    }

    annot_store = pan_record_annots (self->selected_record);
    n_annots = g_list_model_get_n_items (G_LIST_MODEL (annot_store));
    for (guint i = 0; i < n_annots; i++) {
        annot = g_list_model_get_item (G_LIST_MODEL (annot_store), i);
        pan_annot_get_pos (annot, &annot_x, &annot_y);
        if (graphene_point_near (&GRAPHENE_POINT_INIT (x, y), &GRAPHENE_POINT_INIT (annot_x, annot_y), self->radius)) {
            self->hover_annot = annot;
            gtk_widget_set_cursor (GTK_WIDGET (self), self->hand_cursor);
            gtk_widget_queue_draw (GTK_WIDGET (self));
            return;
        }
    }

    if (self->hover_annot) {
        self->hover_annot = NULL;
        gtk_widget_set_cursor (GTK_WIDGET (self), self->normal_cursor);
        gtk_widget_queue_draw (GTK_WIDGET (self));
    }
}

static void
pan_canvas_pointer_enter_cb (PanCanvas *self,
                             gdouble    x,
                             gdouble    y,
                             gpointer   user_data)
{
}

static void
pan_canvas_pointer_leave_cb (PanCanvas *self,
                             gdouble    x,
                             gdouble    y,
                             gpointer   user_data)
{
}

G_GNUC_UNUSED
static GdkCursor *
load_cursor (const gchar *resource_path,
             guint        hotspot_x,
             guint        hotspot_y)
{
    GdkTexture *texture;

    texture = gdk_texture_new_from_resource (resource_path);
    return gdk_cursor_new_from_texture (texture, hotspot_x, hotspot_y, NULL);
}

void
pan_canvas_set_radius (PanCanvas *self,
                       gdouble    radius)
{
    g_return_if_fail (PAN_IS_CANVAS (self));
    g_return_if_fail (radius >= 1 && radius <= 100);

    self->radius = radius;
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
pan_canvas_set_alpha (PanCanvas *self,
                      gdouble    alpha)
{
    g_return_if_fail (PAN_IS_CANVAS (self));
    g_return_if_fail (alpha >= 0.0 && alpha <= 1.0);

    self->color.alpha = alpha;
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
pan_canvas_set_color (PanCanvas *self,
                      GdkRGBA   *color)
{
    g_return_if_fail (PAN_IS_CANVAS (self));

    self->color.red = color->red;
    self->color.green = color->green;
    self->color.blue = color->blue;
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
pan_canvas_zoom_in (PanCanvas *self)
{
    g_return_if_fail (PAN_IS_CANVAS (self));

    if (self->zoom_factor + ZOOM_DELTA <= MAX_ZOOM_FACTOR)
        self->zoom_factor += ZOOM_DELTA;
    gtk_widget_queue_allocate (GTK_WIDGET (self));
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
pan_canvas_zoom_out (PanCanvas *self)
{
    g_return_if_fail (PAN_IS_CANVAS (self));

    if (self->zoom_factor - ZOOM_DELTA >= MIN_ZOOM_FACTOR)
        self->zoom_factor -= ZOOM_DELTA;
    gtk_widget_queue_allocate (GTK_WIDGET (self));
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
pan_canvas_zoom_original (PanCanvas *self)
{
    g_return_if_fail (PAN_IS_CANVAS (self));

    self->zoom_factor = 1.0;
    gtk_widget_queue_allocate (GTK_WIDGET (self));
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
pan_canvas_zoom_fit (PanCanvas *self)
{
    gint img_width, img_height;
    gint viewport_width, viewport_height;
    gboolean scale_x, scale_y;
    gfloat x_ratio, y_ratio;

    g_return_if_fail (PAN_IS_CANVAS (self));

    img_width = gdk_texture_get_width (self->image);
    img_height = gdk_texture_get_height (self->image);
    viewport_width = gtk_widget_get_width (GTK_WIDGET (self));
    viewport_height = gtk_widget_get_height (GTK_WIDGET (self));

    scale_x = img_width > viewport_width ? TRUE : FALSE;
    scale_y = img_height > viewport_height ? TRUE : FALSE;
    x_ratio = viewport_width / (float) img_width;
    y_ratio = viewport_height / (float) img_height;

    if (scale_x && scale_y)
        self->zoom_factor = MIN (x_ratio, y_ratio);
    else if (scale_x)
        self->zoom_factor = x_ratio;
    else if (scale_y)
        self->zoom_factor = y_ratio;

    if (scale_x || scale_y) {
        gtk_widget_queue_allocate (GTK_WIDGET (self));
        gtk_widget_queue_draw (GTK_WIDGET (self));
    }
}

static void
set_up_context_menu (PanCanvas *self)
{
    GMenu *menu;

    menu = g_menu_new ();
    g_menu_append (menu, "Create", "pan-widget.create");
    g_menu_append (menu, "Delete", "pan-widget.delete");

    self->context_menu = gtk_popover_menu_new_from_model (G_MENU_MODEL (menu));
    gtk_widget_set_parent (self->context_menu, GTK_WIDGET (self));
    g_object_unref (menu);
}

static void
pan_canvas_sec_button_press_cb (PanCanvas *self,
                                gint       n_press,
                                gdouble    x,
                                gdouble    y,
                                gpointer   user_data)
{
    GdkRectangle rect = {x, y, 1, 1};
    GtkPopover  *context_menu = GTK_POPOVER (self->context_menu);

    gtk_popover_set_pointing_to (context_menu, &rect);
    gtk_popover_popup (context_menu);
}

static void
pan_canvas_create_action_cb (GtkWidget  *widget,
                             const char *action_name,
                             GVariant   *parameter)
{
    g_print ("create action activated from pan widget\n");
}

static void
pan_canvas_delete_action_cb (GtkWidget  *widget,
                             const char *action_name,
                             GVariant   *parameter)
{
    delete_annot (PAN_CANVAS (widget));
}

static void
load_record (PanCanvas *self)
{
    GListStore *annots_store;
    gchar *root_path, *filename, *img_path;

    self->selected_record = gtk_single_selection_get_selected_item (self->record_selection);
    annots_store = pan_record_annots (self->selected_record);

    if (self->annot_selection) {
        g_signal_handlers_disconnect_by_func (self->annot_selection, pan_widget_annot_selection_changed_cb, self);
        g_object_unref (self->annot_selection);
    }

    self->annot_selection = gtk_single_selection_new (G_LIST_MODEL (g_object_ref (annots_store)));
    g_signal_connect (GTK_SELECTION_MODEL (self->annot_selection), "selection-changed", G_CALLBACK (pan_widget_annot_selection_changed_cb), self);

    root_path = pan_document_get_root_path (self->document);
    filename = pan_record_filename (self->selected_record);
    img_path = g_strjoin ("/", root_path, filename, NULL);

    load_image (self, img_path);

    stack_clear (&self->undo_stack);
    stack_clear (&self->redo_stack);

    gtk_widget_queue_allocate (GTK_WIDGET (self));
    gtk_widget_queue_draw (GTK_WIDGET (self));

    g_free (img_path);
}

static void
record_changed (GtkSelectionModel *self,
                guint              position,
                guint              n_items,
                gpointer           user_data)
{
    load_record (PAN_CANVAS (user_data));
}

void
pan_canvas_set_document (PanCanvas   *self,
                         PanDocument *document)
{
    GListStore *records;

    g_return_if_fail (PAN_IS_CANVAS (self));
    g_return_if_fail (PAN_IS_DOCUMENT (document));

    g_clear_object (&self->document);
    g_clear_object (&self->record_selection);
    g_clear_object (&self->annot_selection);

    self->document = g_object_ref (document);

    records = pan_document_records (self->document);
    self->record_selection = gtk_single_selection_new (G_LIST_MODEL (g_object_ref (records)));
    gtk_single_selection_set_autoselect (self->record_selection, TRUE);
    g_signal_connect (GTK_SELECTION_MODEL (self->record_selection),
                     "selection-changed", G_CALLBACK (record_changed), self);

    // self->n_records = g_list_model_get_n_items (G_LIST_MODEL (records));
    self->record_idx = 0;

    load_record (self);

    g_object_notify (G_OBJECT (self), "document");
}

static void
load_image (PanCanvas *self,
            gchar     *img_path)
{
    g_clear_object (&self->image);
    self->image = gdk_texture_new_from_filename (img_path, NULL);
    if (!self->image)
        g_warning ("set_image: Invalid image file: %s",  img_path);
}

void
pan_canvas_first (PanCanvas *self)
{
    g_return_if_fail (PAN_IS_CANVAS (self));

    gtk_single_selection_set_selected (self->record_selection, 0);
}

void
pan_canvas_prev (PanCanvas *self)
{
    guint pos;

    g_return_if_fail (PAN_IS_CANVAS (self));

    pos = gtk_single_selection_get_selected (self->record_selection);
    if (pos >= 1)
        gtk_single_selection_set_selected (self->record_selection, pos - 1);
}

void
pan_canvas_next (PanCanvas *self)
{
    guint pos, n;

    g_return_if_fail (PAN_IS_CANVAS (self));

    g_object_get (self->record_selection, "n-items", &n, NULL);
    pos = gtk_single_selection_get_selected (self->record_selection);
    if (pos + 1 < n)
        gtk_single_selection_set_selected (self->record_selection, pos + 1);
}

void
pan_canvas_last (PanCanvas *self)
{
    guint n;

    g_return_if_fail (PAN_IS_CANVAS (self));

    g_object_get (self->record_selection, "n-items", &n, NULL);
    if (n >= 1)
        gtk_single_selection_set_selected (self->record_selection, n - 1);
}

GtkSingleSelection *
pan_canvas_get_record_selection_model (PanCanvas *self)
{
    g_return_val_if_fail (PAN_IS_CANVAS (self), NULL);
    g_assert (self->record_selection);

    return g_object_ref (self->record_selection);
}

GtkSingleSelection *
pan_canvas_get_annot_selection_model (PanCanvas *self)
{
    g_return_val_if_fail (PAN_IS_CANVAS (self), NULL);
    // g_assert (self->annot_selection);

    return self->annot_selection;
}


static void
pan_widget_annot_selection_changed_cb (GtkSelectionModel *model,
                                       guint              position,
                                       guint              n_items,
                                       gpointer           user_data)
{
    PanCanvas *canvas = user_data;

    canvas->selected_annot = gtk_single_selection_get_selected_item (GTK_SINGLE_SELECTION (model));
    gtk_widget_queue_draw (GTK_WIDGET (user_data));
}

static void
stack_push_action (GSList    **stack,
                   PanAction  *action)
{
    *stack = g_slist_prepend (*stack, action);
}

static PanAction *
stack_pop_action (GSList **stack)
{
    PanAction *action;

    if (!*stack)
        return NULL;
    action = (*stack)->data;
    *stack = g_slist_remove_link (*stack, *stack);
    return action;
}

static void
stack_clear (GSList **stack)
{
    if (!*stack) return;

    g_slist_free_full (*stack, g_object_unref);
    *stack = NULL;
}

void
pan_canvas_undo (PanCanvas *self)
{
    PanAction *action;

    g_return_if_fail (PAN_IS_CANVAS (self));

    action = stack_pop_action (&self->undo_stack);
    if (!action) return;
    pan_action_undo (action);
    stack_push_action (&self->redo_stack, action);
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

void
pan_canvas_redo (PanCanvas *self)
{
    PanAction *action;

    g_return_if_fail (PAN_IS_CANVAS (self));

    action = stack_pop_action (&self->redo_stack);
    if (!action) return;
    pan_action_redo (action);
    stack_push_action (&self->undo_stack, action);
    gtk_widget_queue_draw (GTK_WIDGET (self));
}

