/*
 * pan-action.c
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

#include "pan-action.h"

G_DEFINE_ABSTRACT_TYPE (PanAction, pan_action, G_TYPE_OBJECT);

static void
pan_action_class_init (PanActionClass *klass)
{
    klass->undo = NULL;
    klass->redo = NULL;
}

static void
pan_action_init (PanAction *self)
{
}

void
pan_action_undo (PanAction *self)
{
    PanActionClass *klass;

    g_return_if_fail (PAN_IS_ACTION (self));

    klass = PAN_ACTION_GET_CLASS (self);
    if (klass->undo)
        klass->undo (self);
}

void
pan_action_redo (PanAction *self)
{
    PanActionClass *klass;

    g_return_if_fail (PAN_IS_ACTION (self));

    klass = PAN_ACTION_GET_CLASS (self);
    if (klass->redo)
        klass->redo (self);
}

