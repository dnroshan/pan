/*
 * pan-application.c
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

#include "config.h"
#include <glib/gi18n.h>

#include "pan-application.h"
#include "pan-window.h"

struct _PanApplication
{
    AdwApplication parent_instance;

    GtkWindow *window;
};

G_DEFINE_FINAL_TYPE (PanApplication, pan_application, ADW_TYPE_APPLICATION)

PanApplication *
pan_application_new (const char *application_id, GApplicationFlags flags)
{
    g_return_val_if_fail (application_id != NULL, NULL);

    return g_object_new (PAN_TYPE_APPLICATION,
                         "application-id",
                         application_id,
                         "flags",
                         flags,
                         NULL);
}

static void
pan_application_activate (GApplication *self)
{
    PanApplication *app = PAN_APPLICATION (self);


    g_assert (PAN_IS_APPLICATION (app));

    app->window = gtk_application_get_active_window (GTK_APPLICATION (app));

    if (app->window == NULL)
        app->window = g_object_new (PAN_TYPE_WINDOW, "application", app, NULL);

    gtk_window_present (app->window);
}

static void
pan_application_class_init (PanApplicationClass *klass)
{
    GApplicationClass *app_class = G_APPLICATION_CLASS (klass);

    app_class->activate = pan_application_activate;
}

static void
pan_application_about_action (GSimpleAction *action,
                              GVariant *parameter,
                              gpointer user_data)
{
    static const char *developers[] = {"Dilnavas Roshan", NULL};
    PanApplication *self = user_data;
    GtkWindow *window = NULL;

    g_assert (PAN_IS_APPLICATION (self));

    window = gtk_application_get_active_window (GTK_APPLICATION (self));
    adw_show_about_dialog (GTK_WIDGET (window),
                           "application-name",
                           "Pan",
                           "application-icon",
                           "me.scratchspace.Pan",
                           "developer-name",
                           "Dilnavas Roshan",
                           "translator-credits",
                           _ ("translator-credits"),
                           "license-type",
                           GTK_LICENSE_GPL_3_0,
                           "version",
                           "0.6.0",
                           "developers",
                           developers,
                           "copyright",
                           "© 2025 Bigtec Labs",
                           NULL);
}

static void
pan_application_quit_action (GSimpleAction *action,
                             GVariant *parameter,
                             gpointer user_data)
{
    PanApplication *self = user_data;

    g_assert (PAN_IS_APPLICATION (self));

    pan_window_close (PAN_WINDOW (self->window));
}

static void
pan_application_preferences_action (GSimpleAction *action,
                                    GVariant *parameter,
                                    gpointer user_data)
{
    PanApplication *self = user_data;

    g_assert (PAN_IS_APPLICATION (self));

    g_print ("Preferences action activated\n");
}

static const GActionEntry app_actions[] =
{
    {"quit",        pan_application_quit_action},
    {"about",       pan_application_about_action},
    {"preferences", pan_application_preferences_action}
};

static void
pan_application_init (PanApplication *self)
{
    g_action_map_add_action_entries (G_ACTION_MAP (self),
                                     app_actions,
                                     G_N_ELEMENTS (app_actions),
                                     self);
    gtk_application_set_accels_for_action (
        GTK_APPLICATION (self),
        "app.quit",
        (const char *[]){"<primary>q", NULL});

    gtk_application_set_accels_for_action (
        GTK_APPLICATION (self),
        "win.undo",
        (const char *[]){"<Ctrl>z", NULL});

    gtk_application_set_accels_for_action (
        GTK_APPLICATION (self),
        "win.redo",
        (const char *[]){"<Ctrl><Shift>z", NULL});

    gtk_application_set_accels_for_action (
        GTK_APPLICATION (self),
        "win.new",
        (const char *[]){"<Ctrl>o", NULL});
}

