/*
 * pan-document.c
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

#include <glib.h>
#include <json-glib/json-glib.h>
#include "pan-document.h"

struct _PanDocument
{
    GObject parent;

    gchar *path;
    gboolean is_dirty;
    GListStore *records;
    gboolean dirty;
};

enum
{
    PROP_ZERO,
    PROP_PATH,
    PROP_RECORDS,
    N_PROPS
};

static GParamSpec *pan_document_props[N_PROPS] = {NULL, };

static void         pan_document_get_property                (GObject    *object,
                                                              guint       property_id,
                                                              GValue     *value,
                                                              GParamSpec *pspec);
static void         pan_document_set_property                (GObject      *object,
                                                              guint         property_id,
                                                              const GValue *value,
                                                              GParamSpec   *pspec);
static void         pan_document_dispose                     (GObject *object);
static void         pan_document_finalize                    (GObject *object);
static void         pan_document_serializable_interface_init (JsonSerializableIface *iface);
static gboolean     pan_document_deserialize_property        (JsonSerializable *serializable,
                                                              const gchar      *property_name,
                                                              GValue           *value,
                                                              GParamSpec       *pspec,
                                                              JsonNode         *property_node);
static JsonNode    *pan_document_serialize_property          (JsonSerializable *serializable,
                                                              const gchar      *property_name,
                                                              const GValue     *value,
                                                              GParamSpec       *pspec);

G_DEFINE_FINAL_TYPE_WITH_CODE (PanDocument, pan_document, G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (JSON_TYPE_SERIALIZABLE,
                                                      pan_document_serializable_interface_init))

static void
pan_document_class_init (PanDocumentClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = pan_document_get_property;
    object_class->set_property = pan_document_set_property;
    object_class->dispose      = pan_document_dispose;
    object_class->finalize     = pan_document_finalize;

    pan_document_props[PROP_RECORDS] =
        g_param_spec_object ("records", NULL, NULL,
                             G_TYPE_LIST_STORE,
                             G_PARAM_READWRITE);
    pan_document_props[PROP_PATH] =
        g_param_spec_string ("path",
                             NULL, NULL, "",
                             G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, N_PROPS, pan_document_props);
}

static void
pan_document_init (PanDocument *self)
{
    self->is_dirty = FALSE;
    self->records  = g_list_store_new (PAN_TYPE_RECORD);
}

static void
pan_document_dispose (GObject *object)
{
    PanDocument *document = PAN_DOCUMENT (object);

    g_clear_object (&document->records);
    G_OBJECT_CLASS (pan_document_parent_class)->dispose (object);
}

static void
pan_document_finalize (GObject *object)
{
    PanDocument *document = PAN_DOCUMENT (object);

    g_free (document->path);
    G_OBJECT_CLASS (pan_document_parent_class)->finalize (object);
}


static void
pan_document_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    PanDocument *document = PAN_DOCUMENT (object);

    switch (property_id) {
    case PROP_PATH:
        g_value_set_string (value, document->path);
        break;
    case PROP_RECORDS:
        g_value_set_object (value, document->records);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
pan_document_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    PanDocument *document = PAN_DOCUMENT (object);

    switch (property_id) {
    case PROP_PATH:
        if (document->path)
            g_free (document->path);
        document->path = g_strdup (g_value_get_string (value));
        break;
    case PROP_RECORDS:
        if (document->records)
            g_object_unref (document->records);
        document->records = g_value_get_object (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}

static void
pan_document_serializable_interface_init (JsonSerializableIface *iface)
{
    iface->serialize_property = pan_document_serialize_property;
    iface->deserialize_property = pan_document_deserialize_property;
}


PanDocument *
pan_document_new (GFile *file)
{
    PanDocument *doc;
    GFileEnumerator *file_enumerator;
    GFileInfo *file_info;
    GError *error = NULL;
    PanRecord *record;
    const gchar *filename;

    doc = g_object_new (PAN_TYPE_DOCUMENT, NULL);
    doc->path = g_strdup (g_file_get_path (file));
    file_enumerator = g_file_enumerate_children (file, "", 0, NULL, &error);
    if (!file_enumerator) {
        g_error ("%s", error->message);
        g_error_free (error);
        return NULL;
    }

    error = NULL;
    while ((file_info = g_file_enumerator_next_file (file_enumerator,
                                                     NULL, &error)) != NULL) {
        if (g_file_info_get_file_type (file_info) != G_FILE_TYPE_REGULAR)
            continue;
        filename = g_file_info_get_name (file_info);
        record = pan_record_new (filename);
        g_list_store_append (doc->records, record);
        g_object_unref (G_OBJECT (file_info));
    }
    if (error) {
        g_error ("%s", error->message);
        g_error_free (error);
        return NULL;
    }

    g_object_unref (file_enumerator);

    return doc;
}

GListStore *
pan_document_records (PanDocument *self)
{
    g_return_val_if_fail (PAN_IS_DOCUMENT (self), NULL);

    return self->records;
}

gchar *
pan_document_get_root_path (PanDocument *self)
{
    g_return_val_if_fail (PAN_IS_DOCUMENT (self), NULL);

    return self->path;
}

void
pan_document_save (PanDocument *self,
                   gchar       *path)
{
    GError *error = NULL;
    gchar *data;
    gsize size;

    g_return_if_fail (PAN_IS_DOCUMENT (self));

    data = json_gobject_to_data (G_OBJECT (self), &size);
    g_file_set_contents (path, data, size, &error);
    self->dirty = FALSE;
    g_free (data);
}

static gboolean
pan_document_deserialize_property (JsonSerializable *serializable,
                                   const gchar      *property_name,
                                   GValue           *value,
                                   GParamSpec       *pspec,
                                   JsonNode         *property_node)
{
    JsonNode *node;
    JsonArray *array;
    PanRecord *record;
    GListStore *record_store;
    guint n;

    if (!g_strcmp0 (property_name, "records")) {
        record_store = g_list_store_new (PAN_TYPE_RECORD);
        array = json_node_get_array (property_node);
        n = json_array_get_length (array);
        for (int i = 0; i < n; i++) {
            node = json_array_get_element (array, i);
            record = PAN_RECORD (json_gobject_deserialize (PAN_TYPE_RECORD, node));
            g_list_store_append (record_store, record);
        }
        g_value_set_object (value, record_store);
        return TRUE;
    }
    return json_serializable_default_deserialize_property (serializable, property_name, value, pspec, property_node);
}

static JsonNode *
pan_document_serialize_property (JsonSerializable *serializable,
                                 const gchar      *property_name,
                                 const GValue     *value,
                                 GParamSpec       *pspec)
{
    JsonNode *node, *child;
    JsonArray *array;
    GListStore *records_store;
    PanRecord *record;
    guint n;

    if (!g_strcmp0 (property_name, "records")) {
        records_store = g_value_get_object (value);
        n = g_list_model_get_n_items (G_LIST_MODEL (records_store));
        node = json_node_new (JSON_NODE_ARRAY);
        array = json_array_new ();
        for (guint i = 0; i < n; i++) {
            record = g_list_model_get_item (G_LIST_MODEL (records_store), i);
            child = json_gobject_serialize (G_OBJECT (record));
            json_array_add_element (array, child);
        }
        json_node_set_array (node, array);

        return node;

    }
    return json_serializable_default_serialize_property (serializable, property_name, value, pspec);
}

PanDocument *
pan_document_open (gchar *path)
{
    gchar *data;
    gsize size;
    GError *error = NULL;
    PanDocument *document;

    g_file_get_contents (path,  &data, &size, &error);
    document = PAN_DOCUMENT (json_gobject_from_data (PAN_TYPE_DOCUMENT, data, size, &error));
    return document;
}

gboolean
pan_document_is_dirty (PanDocument *self)
{
    g_return_val_if_fail (PAN_IS_DOCUMENT (self), FALSE);
    return self->dirty;
}

void
pan_document_set_dirty (PanDocument *self,
                        gboolean     dirty)
{
    g_return_if_fail (PAN_IS_DOCUMENT (self));

    self->dirty = dirty;
}

