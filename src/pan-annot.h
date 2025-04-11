/*
 * pan-annot.h
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

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define PAN_TYPE_ANNOT pan_annot_get_type ()
G_DECLARE_FINAL_TYPE (PanAnnot, pan_annot, PAN, ANNOT, GObject)

PanAnnot *pan_annot_new       (guint x,
                               guint y);
void      pan_annot_update    (PanAnnot *self,
                               guint     x,
                               guint     y);
void      pan_annot_translate (PanAnnot *self,
                               guint     dx,
                               guint     dy);
void      pan_annot_get_pos   (PanAnnot *self,
                               guint    *x,
                               guint    *y);
void      pan_annot_set_x     (PanAnnot *self,
                               guint     x);
void      pan_annot_set_y     (PanAnnot *self,
                               guint     y);
guint     pan_annot_x         (PanAnnot *self);
guint     pan_annot_y         (PanAnnot *self);

G_END_DECLS

