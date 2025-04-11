/*
 * pan-action-delete.c
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

#include "pan-action-delete.h"

struct _PanActionDelete
{
    PanAction parent;

    PanRecord *record;
    PanAnnot *annot;
    guint pos;
};

static void undo    (PanAction *action);
static void redo    (PanAction *action);
static void dispose (GObject *self);

G_DEFINE_FINAL_TYPE (PanActionDelete, pan_action_delete, PAN_TYPE_ACTION)

static void
pan_action_delete_class_init (PanActionDeleteClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    PanActionClass *action_class = PAN_ACTION_CLASS (klass);

    object_class->dispose = dispose;

    action_class->undo = undo;
    action_class->redo = redo;
}

static void
pan_action_delete_init (PanActionDelete *self)
{
}

static void
dispose (GObject *self)
{
    PanActionDelete *action_create = PAN_ACTION_DELETE (self);

    g_clear_object (&action_create->annot);
    g_clear_object (&action_create->record);
    G_OBJECT_CLASS (pan_action_delete_parent_class)->dispose (self);
}

static void
undo (PanAction *self)
{
    GListStore *annot_store;
    PanActionDelete *action_delete = PAN_ACTION_DELETE (self);

    annot_store = pan_record_annots (action_delete->record);
    g_list_store_insert (annot_store, action_delete->pos, action_delete->annot);
}

static void
redo (PanAction *self)
{
    GListStore *annot_store;
    PanActionDelete *action_delete = PAN_ACTION_DELETE (self);

    annot_store = pan_record_annots (action_delete->record);
    g_list_store_remove (annot_store, action_delete->pos);
}

PanAction *
pan_action_delete_new (PanRecord *record, PanAnnot *annot, guint pos)
{
    PanActionDelete *action_create;

    action_create = g_object_new (PAN_TYPE_ACTION_DELETE, NULL);
    action_create->annot = g_object_ref (annot);
    action_create->record = g_object_ref (record);
    action_create->pos = pos;

    return PAN_ACTION (action_create);
}

