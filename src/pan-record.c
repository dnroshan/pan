/*
 * pan-record.c
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
#include "pan-record.h"

struct _PanRecord
{
    GObject parent;

    gchar *filename;
    GListStore *annots;
};

enum
{
    PROP_ZERO,
    PROP_FILENAME,
    PROP_ANNOTS,
    N_PROPS
};

static void         pan_record_get_properties          (GObject    *object,
                                                        guint       property_id,
                                                        GValue     *value,
                                                        GParamSpec *pspec);

static void         pan_record_set_properties          (GObject      *object,
                                                        guint         property_id,
                                                        const GValue *value,
                                                        GParamSpec   *pspec);
static void         pan_record_serializable_iface_init (JsonSerializableIface *iface);
static gboolean     pan_record_deserialize_property    (JsonSerializable* serializable,
                                                        const gchar* property_name,
                                                        GValue* value,
                                                        GParamSpec* pspec,
                                                        JsonNode* property_node);
static JsonNode    *pan_record_serialize_property      (JsonSerializable* serializable,
                                                        const gchar* property_name,
                                                        const GValue* value,
                                                        GParamSpec* pspec);

static void         pan_record_dispose                 (GObject *object);
static void         pan_record_finalize                (GObject *object);

static GParamSpec *pan_record_properties[N_PROPS] = {NULL, };

G_DEFINE_FINAL_TYPE_WITH_CODE (PanRecord, pan_record, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (JSON_TYPE_SERIALIZABLE,
                                                      pan_record_serializable_iface_init))

static void
pan_record_class_init (PanRecordClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = pan_record_get_properties;
    object_class->set_property = pan_record_set_properties;
    object_class->dispose      = pan_record_dispose;
    object_class->finalize     = pan_record_finalize;

    pan_record_properties[PROP_FILENAME] =
        g_param_spec_string ("filename", NULL, NULL, "", G_PARAM_READWRITE);

    pan_record_properties[PROP_ANNOTS] =
        g_param_spec_object ("annots", NULL, NULL, G_TYPE_LIST_STORE, G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, N_PROPS, pan_record_properties);
}

static void
pan_record_init (PanRecord *self)
{
    self->filename = g_strdup ("");
    self->annots = g_list_store_new (PAN_TYPE_ANNOT);
}

static void
pan_record_get_properties (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    PanRecord *record = PAN_RECORD (object);

    switch (property_id) {
    case PROP_FILENAME:
        g_value_set_string (value, record->filename);
        break;
    case PROP_ANNOTS:
        g_value_set_object (value, g_object_ref (record->annots));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
pan_record_set_properties (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    PanRecord *record = PAN_RECORD (object);

    switch (property_id) {
    case PROP_FILENAME:
        if (record->filename)
            g_free (record->filename);
        record->filename = g_strdup (g_value_get_string (value));
        break;
    case PROP_ANNOTS:
        if (record->annots)
            g_object_unref (record->annots);
        record->annots = g_object_ref (g_value_get_object (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
pan_record_dispose (GObject *object)
{
    PanRecord *record = PAN_RECORD (object);

    g_clear_object (&record->annots);
    G_OBJECT_CLASS (pan_record_parent_class)->dispose (object);
}

static void
pan_record_finalize (GObject *object)
{
    PanRecord *record = PAN_RECORD (object);

    g_free (record->filename);
    G_OBJECT_CLASS (pan_record_parent_class)->finalize (object);
}

static void
pan_record_serializable_iface_init (JsonSerializableIface *iface)
{
    iface->serialize_property = pan_record_serialize_property;
    iface->deserialize_property = pan_record_deserialize_property;
}

PanRecord *
pan_record_new (const gchar *filename)
{
    return g_object_new (PAN_TYPE_RECORD, "filename", filename, NULL);
}

gchar *
pan_record_filename (PanRecord *self)
{
    g_return_val_if_fail (PAN_IS_RECORD (self), NULL);

    return self->filename;
}

GListStore *
pan_record_annots (PanRecord *self)
{
    g_return_val_if_fail (PAN_IS_RECORD (self), NULL);

    return self->annots;
}

void
pan_record_set_filename (PanRecord *self,
                         gchar *filename)
{
    g_return_if_fail (PAN_IS_RECORD (self));

    g_object_set (self, "filename", filename, NULL);
}

void
pan_record_set_annots (PanRecord *self,
                       GListStore *annots)
{
    g_return_if_fail (PAN_IS_RECORD (self));

    g_object_set (self, "annots", annots, NULL);
}

static gboolean
pan_record_deserialize_property (JsonSerializable *serializable,
                                 const gchar      *property_name,
                                 GValue           *value,
                                 GParamSpec       *pspec,
                                 JsonNode         *property_node)
{
    JsonNode *node;
    JsonArray *array;
    PanAnnot *annot;
    GListStore *annot_store;
    guint n;

    if (!g_strcmp0 (property_name, "annots")) {
        annot_store = g_list_store_new (PAN_TYPE_ANNOT);
        array = json_node_get_array (property_node);
        n = json_array_get_length (array);
        for (int i = 0; i < n; i++) {
            node = json_array_get_element (array, i);
            annot = PAN_ANNOT (json_gobject_deserialize (PAN_TYPE_ANNOT, node));
            g_list_store_append (annot_store, annot);
        }
        g_value_set_object (value, annot_store);
        return TRUE;
    }
    return json_serializable_default_deserialize_property (serializable, property_name, value, pspec, property_node);
}

static JsonNode *
pan_record_serialize_property (JsonSerializable *serializable,
                               const gchar      *property_name,
                               const GValue     *value,
                               GParamSpec       *pspec)
{
    PanRecord *record = PAN_RECORD (serializable);
    JsonNode *node, *child;
    JsonArray *array;
    PanAnnot *annot;
    guint n;

    if (!g_strcmp0 (property_name, "annots")) {
        n = g_list_model_get_n_items (G_LIST_MODEL (record->annots));
        node = json_node_new (JSON_NODE_ARRAY);
        array = json_array_new ();
        for (guint i = 0; i < n; i++) {
            annot = g_list_model_get_item (G_LIST_MODEL (record->annots), i);
            child = json_gobject_serialize (G_OBJECT (annot));
            json_array_add_element (array, child);
        }
        json_node_set_array (node, array);
        return node;
    }

    return json_serializable_default_serialize_property (serializable, property_name, value, pspec);
}

