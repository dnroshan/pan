/*
 * pan-annot-item.h
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

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PAN_TYPE_ANNOT_VIEW pan_annot_view_get_type ()
G_DECLARE_FINAL_TYPE (PanAnnotView, pan_annot_view, PAN, ANNOT_VIEW, GtkBox)

GtkWidget *pan_annot_view_new (void);
void       pan_annot_view_set (PanAnnotView *self, gdouble x, gdouble y);

G_END_DECLS

