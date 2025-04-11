/*
 * pan-widget.h
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

#include "pan-document.h"
#include <adwaita.h>

G_BEGIN_DECLS

#define PAN_TYPE_CANVAS pan_canvas_get_type ()
G_DECLARE_FINAL_TYPE (PanCanvas, pan_canvas, PAN, CANVAS, GtkWidget)

void pan_canvas_set_radius    (PanCanvas *self,
                               gdouble radius);
void pan_canvas_set_color     (PanCanvas *self,
                               GdkRGBA *color);
void pan_canvas_set_alpha     (PanCanvas *self,
                               gdouble alpha);
void pan_canvas_set_document  (PanCanvas *self,
                               PanDocument *document);
void pan_canvas_zoom_in       (PanCanvas *self);
void pan_canvas_zoom_out      (PanCanvas *self);
void pan_canvas_zoom_original (PanCanvas *self);
void pan_canvas_zoom_fit      (PanCanvas *self);
void pan_canvas_first         (PanCanvas *self);
void pan_canvas_next          (PanCanvas *self);
void pan_canvas_prev          (PanCanvas *self);
void pan_canvas_last          (PanCanvas *self);
void pan_canvas_undo          (PanCanvas *self);
void pan_canvas_redo          (PanCanvas *self);

GtkSingleSelection *pan_canvas_get_record_selection_model (PanCanvas *self);
GtkSingleSelection *pan_canvas_get_annot_selection_model  (PanCanvas *self);

G_END_DECLS

