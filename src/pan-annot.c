/*
 * pan-annot.c
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

#include <json-glib/json-glib.h>
#include "pan-annot.h"

struct _PanAnnot
{
    GObject parent;

    guint x, y;
};

enum
{
    PROP_ZERO,
    PROP_X,
    PROP_Y,
    N_PROPS
};

static void pan_annot_get_property             (GObject    *object,
                                                guint       prop_id,
                                                GValue     *value,
                                                GParamSpec *pspec);

static void pan_annot_set_property             (GObject      *object,
                                                guint         prop_id,
                                                const GValue *value,
                                                GParamSpec   *pspec);
static void pan_record_serializable_iface_init (JsonSerializableIface *iface);

static GParamSpec *pan_annot_properties[N_PROPS] = {NULL, };

G_DEFINE_FINAL_TYPE_WITH_CODE (PanAnnot, pan_annot, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (JSON_TYPE_SERIALIZABLE, NULL));

static void
pan_annot_class_init (PanAnnotClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = pan_annot_get_property;
    object_class->set_property = pan_annot_set_property;

    pan_annot_properties[PROP_X] =
        g_param_spec_uint ("x", NULL, NULL,
                            0, G_MAXUINT, 0,
                            G_PARAM_READWRITE);

    pan_annot_properties[PROP_Y] =
        g_param_spec_uint ("y", NULL, NULL,
                            0, G_MAXUINT, 0,
                            G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, N_PROPS, pan_annot_properties);
}

static void
pan_annot_init (PanAnnot *self)
{
    self->x = 0.0;
    self->y = 0.0;
}

static void
pan_annot_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    PanAnnot *annot = PAN_ANNOT (object);

    switch (prop_id) {
    case PROP_X:
        g_value_set_uint (value, annot->x);
        break;
    case PROP_Y:
        g_value_set_uint (value, annot->y);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
pan_annot_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    PanAnnot *annot = PAN_ANNOT (object);

    switch (prop_id) {
    case PROP_X:
        annot->x = g_value_get_uint (value);
        break;
    case PROP_Y:
        annot->y = g_value_get_uint (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
pan_record_serializable_iface_init (JsonSerializableIface *iface)
{

}

PanAnnot *
pan_annot_new (guint x, guint y)
{
    PanAnnot *annot;

    annot = g_object_new (PAN_TYPE_ANNOT, "x", x, "y", y, NULL);
    return annot;
}

void
pan_annot_update (PanAnnot *self, guint x, guint y)
{
    g_return_if_fail (PAN_IS_ANNOT (self));
    g_return_if_fail (x >= 0.0 && x <= G_MAXFLOAT);
    g_return_if_fail (y >= 0.0 && y <= G_MAXFLOAT);

    self->x = x;
    self->y = y;

    g_object_notify_by_pspec (G_OBJECT (self), pan_annot_properties[PROP_X]);
    g_object_notify_by_pspec (G_OBJECT (self), pan_annot_properties[PROP_Y]);
}

void
pan_annot_translate (PanAnnot *self, guint dx, guint dy)
{
    g_return_if_fail (PAN_IS_ANNOT (self));

    pan_annot_update (self, self->x + dx, self->y + dy);
}

void
pan_annot_get_pos (PanAnnot *self, guint *x, guint *y)
{
    g_return_if_fail (PAN_IS_ANNOT (self));

    *x = self->x;
    *y = self->y;
}

guint
pan_annot_x (PanAnnot *self)
{
    g_return_val_if_fail (PAN_IS_ANNOT (self), -1);

    return self->x;
}

guint
pan_annot_y (PanAnnot *self)
{
    g_return_val_if_fail (PAN_IS_ANNOT (self), -1);

    return self->y;
}

void
pan_annot_set_x (PanAnnot *self, guint x)
{
    g_return_if_fail (PAN_IS_ANNOT (self));
    g_return_if_fail (x >= 0.0 && x <= G_MAXFLOAT);

    self->x = x;
    g_object_notify_by_pspec (G_OBJECT (self), pan_annot_properties[PROP_X]);
}

void
pan_annot_set_y (PanAnnot *self, guint y)
{
    g_return_if_fail (PAN_IS_ANNOT (self));
    g_return_if_fail (y >= 0.0 && y <= G_MAXFLOAT);

    self->y = y;
    g_object_notify_by_pspec (G_OBJECT (self), pan_annot_properties[PROP_Y]);
}

