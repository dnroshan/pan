/*
 * pan-action-create.c
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

#include "pan-action-create.h"

struct _PanActionCreate
{
    PanAction parent;

    PanRecord *record;
    PanAnnot *annot;
};

static void undo    (PanAction *action);
static void redo    (PanAction *action);
static void dispose (GObject *self);

G_DEFINE_FINAL_TYPE (PanActionCreate, pan_action_create, PAN_TYPE_ACTION)

static void
pan_action_create_class_init (PanActionCreateClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    PanActionClass *action_class = PAN_ACTION_CLASS (klass);

    object_class->dispose = dispose;

    action_class->undo = undo;
    action_class->redo = redo;
}

static void
pan_action_create_init (PanActionCreate *self)
{
}

static void
dispose (GObject *self)
{
    PanActionCreate *action_create = PAN_ACTION_CREATE (self);

    g_clear_object (&action_create->annot);
    g_clear_object (&action_create->record);
    G_OBJECT_CLASS (pan_action_create_parent_class)->dispose (self);
}

static void
undo (PanAction *self)
{
    GListStore *annot_store;
    PanActionCreate *action_create = PAN_ACTION_CREATE (self);
    guint n_items;

    annot_store = pan_record_annots (action_create->record);
    n_items = g_list_model_get_n_items (G_LIST_MODEL (annot_store));
    g_list_store_remove (annot_store, n_items - 1);
}

static void
redo (PanAction *self)
{
    GListStore *annot_store;
    PanActionCreate *action_create = PAN_ACTION_CREATE (self);

    annot_store = pan_record_annots (action_create->record);
    g_list_store_append (annot_store, PAN_ACTION_CREATE (self)->annot);
}

PanAction *
pan_action_create_new (PanRecord *record, PanAnnot *annot)
{
    PanActionCreate *action_create;

    action_create = g_object_new (PAN_TYPE_ACTION_CREATE, NULL);
    action_create->annot = g_object_ref (annot);
    action_create->record = g_object_ref (record);

    return PAN_ACTION (action_create);
}

