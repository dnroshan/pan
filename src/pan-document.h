/*
 * pan-document.h
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

#include <glib.h>
#include <gio/gio.h>
#include "pan-record.h"

G_BEGIN_DECLS

#define PAN_TYPE_DOCUMENT pan_document_get_type ()
G_DECLARE_FINAL_TYPE (PanDocument, pan_document, PAN, DOCUMENT, GObject)

PanDocument *pan_document_new           (GFile *file);
GListStore  *pan_document_records       (PanDocument *self);
PanDocument *pan_document_open          (gchar *path);
void         pan_document_save          (PanDocument *self,
                                         gchar *path);
gchar       *pan_document_get_root_path (PanDocument *self);
gboolean     pan_document_is_dirty      (PanDocument *self);
void         pan_document_set_dirty     (PanDocument *self,
                                         gboolean     dirty);

G_END_DECLS

