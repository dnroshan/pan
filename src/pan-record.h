/*
 * pan-record.h
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

#include "pan-annot.h"
#include <gio/gio.h>

G_BEGIN_DECLS

#define PAN_TYPE_RECORD pan_record_get_type ()
G_DECLARE_FINAL_TYPE (PanRecord, pan_record, PAN, RECORD, GObject)

PanRecord  *pan_record_new          (const gchar *file_name);
gchar      *pan_record_filename     (PanRecord *self);
GListStore *pan_record_annots       (PanRecord *self);
void        pan_record_set_filename (PanRecord *self, gchar *filename);
void        pan_record_set_annots   (PanRecord *self, GListStore *annots);

G_END_DECLS

