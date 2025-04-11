/*
 * pan-action-move.c
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

#include "pan-action-move.h"
#include "pan-annot.h"

struct _PanActionMove
{
    PanAction parent;

    PanAnnot *annot;
    guint old_x, old_y;
    guint new_x, new_y;
};

static void dispose (GObject *self);
static void undo (PanAction *self);
static void redo (PanAction *self);

G_DEFINE_FINAL_TYPE (PanActionMove, pan_action_move, PAN_TYPE_ACTION);

static void
pan_action_move_class_init (PanActionMoveClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    PanActionClass *action_class = PAN_ACTION_CLASS (klass);

    object_class->dispose = dispose;

    action_class->undo = undo;
    action_class->redo = redo;
}

static void
pan_action_move_init (PanActionMove *self)
{
}

static void
dispose (GObject *self)
{
    PanActionMove *action_move = PAN_ACTION_MOVE (self);

    g_clear_object (&action_move->annot);
    G_OBJECT_CLASS (pan_action_move_parent_class)->dispose (self);
}

PanAction *
pan_action_move_new (PanAnnot *annot,
                     guint     old_x,
                     guint     old_y,
                     guint     new_x,
                     guint     new_y)
{
    PanActionMove *action_move;

    action_move = g_object_new (PAN_TYPE_ACTION_MOVE, NULL);

    action_move->annot = g_object_ref (annot);
    action_move->old_x = old_x;
    action_move->old_y = old_y;
    action_move->new_x = new_x;
    action_move->new_y = new_y;

    return PAN_ACTION (action_move);
}

static void
undo (PanAction *self)
{
    PanActionMove *action_move = PAN_ACTION_MOVE (self);

    pan_annot_set_x (action_move->annot, action_move->old_x);
    pan_annot_set_y (action_move->annot, action_move->old_y);
}

static void
redo (PanAction *self)
{
    PanActionMove *action_move = PAN_ACTION_MOVE (self);

    pan_annot_set_x (action_move->annot, action_move->new_x);
    pan_annot_set_y (action_move->annot, action_move->new_y);
}

