/*
 * pan-annot-view.c
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

#include "pan-annot-view.h"

struct _PanAnnotView
{
    GtkBox parent;

    GtkWidget *label_x;
    GtkWidget *label_y;
};

static void dispose (GObject *object);

G_DEFINE_FINAL_TYPE (PanAnnotView, pan_annot_view, GTK_TYPE_BOX)

static void
pan_annot_view_class_init (PanAnnotViewClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = dispose;
}

static void
pan_annot_view_init (PanAnnotView *self)
{
    self->label_x = gtk_label_new ("");
    self->label_y = gtk_label_new ("");

    g_object_ref_sink (self->label_x);
    g_object_ref_sink (self->label_y);

    gtk_box_append (GTK_BOX (self), self->label_x);
    gtk_box_append (GTK_BOX (self), self->label_y);

    gtk_orientable_set_orientation (GTK_ORIENTABLE (self), GTK_ORIENTATION_HORIZONTAL);
    gtk_box_set_spacing (GTK_BOX (self), 8);
    gtk_box_set_homogeneous (GTK_BOX (self), TRUE);
}

static void
dispose (GObject *object)
{
    PanAnnotView *annot_view = PAN_ANNOT_VIEW (object);

    g_clear_object (&annot_view->label_x);
    g_clear_object (&annot_view->label_x);

    G_OBJECT_CLASS (pan_annot_view_parent_class)->dispose (object);
}

GtkWidget *
pan_annot_view_new (void)
{
    return g_object_new (PAN_TYPE_ANNOT_VIEW, NULL);
}

void
pan_annot_view_set (PanAnnotView *self,
                    gdouble x,
                    gdouble y)
{
    char *buf;

    g_return_if_fail (PAN_IS_ANNOT_VIEW (self));

    buf = g_strdup_printf ("X: %d", (int) x);
    gtk_label_set_label (GTK_LABEL (self->label_x), buf);
    g_free (buf);

    buf = g_strdup_printf ("Y: %d", (int) y);
    gtk_label_set_label (GTK_LABEL (self->label_y), buf);
    g_free (buf);
}

