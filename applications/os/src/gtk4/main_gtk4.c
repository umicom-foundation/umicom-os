/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/gtk4/main_gtk4.c
 *
 * PURPOSE:
 *   Create the first Umicom OS GTK4 desktop shell using reusable Framework GTK4 components.
 *
 * Created by: Sammy Hegab
 * Organisation: Umicom Foundation
 * Licence: MIT
 *---------------------------------------------------------------------------*/

/* BEGINNER NOTE:
 * This file keeps one responsibility small and explicit. Read the public
 * structure/function declarations first, then follow the implementation in
 * the matching source file.
 */
#include <gtk/gtk.h>
#include "umicom/os/desktop_shell.h"

#include "umicom/ui/components/component.h"
#include "umicom/ui/gtk4/component_factory.h"

/* The original component-only welcome window is retained for comparison.
 * The shared desktop composition below now exposes Framework's existing
 * notes, checkpoints and system tools without duplicating their ownership. */
#if 0
static GtkWidget *create_component(UmiUiComponentKind kind,
                                   const char *id,
                                   const char *text)
{
    UmiUiComponentSpec spec = umi_ui_component_spec_default(kind);
    (void)umi_ui_component_spec_set_id(&spec, id);
    if (text != NULL) {
        (void)umi_ui_component_spec_set_text(&spec, text);
    }
    return umi_gtk4_component_create(&spec);
}

static void activate(GtkApplication *application, void *user_data)
{
    (void)user_data;

    UmiUiComponentSpec window_spec =
        umi_ui_component_spec_default(UMI_UI_COMPONENT_WINDOW);
    (void)umi_ui_component_spec_set_id(&window_spec, "umicom-os-main");
    (void)umi_ui_component_spec_set_text(&window_spec, "Umicom OS");
    window_spec.width = 1100;
    window_spec.height = 720;
    GtkWidget *window = umi_gtk4_component_create(&window_spec);
    gtk_window_set_application(GTK_WINDOW(window), application);

    UmiUiComponentSpec root_spec =
        umi_ui_component_spec_default(UMI_UI_COMPONENT_BOX);
    (void)umi_ui_component_spec_set_id(&root_spec, "desktop-root");
    root_spec.orientation = UMI_UI_VERTICAL;
    root_spec.spacing = 12;
    GtkWidget *root = umi_gtk4_component_create(&root_spec);

    GtkWidget *welcome = create_component(
        UMI_UI_COMPONENT_LABEL,
        "welcome",
        "Umicom OS desktop shell — Framework GTK4 components");
    gtk_box_append(GTK_BOX(root), welcome);
    gtk_window_set_child(GTK_WINDOW(window), root);
    gtk_window_present(GTK_WINDOW(window));
}

#endif

/* Repeated application activation focuses the existing shell. Child tools
 * keep their own Framework lifetimes and can remain open independently. */
static void activate(GtkApplication *application, void *user_data)
{
    (void)user_data;
    for (GList *item = gtk_application_get_windows(application); item != NULL; item = item->next) {
        GtkWindow *window = item->data;
        if (g_strcmp0(gtk_widget_get_name(GTK_WIDGET(window)), "umicom-os-main") == 0) {
            gtk_window_present(window); return;
        }
    }
    GtkWindow *window = umi_os_desktop_shell_create(application);
    if (window != NULL) gtk_window_present(window);
}

int main(int argc, char **argv)
{
    GtkApplication *application = gtk_application_new(
        "foundation.umicom.os",
        G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(application, "activate", G_CALLBACK(activate), NULL);
    const int status = g_application_run(G_APPLICATION(application), argc, argv);
    g_object_unref(application);
    return status;
}
