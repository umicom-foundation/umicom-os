/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/tests/test_desktop_shell.c
 * PURPOSE: Verify desktop composition without opening storage or collecting host data.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/os/desktop_shell.h"
#include "umicom/test_runtime/check.h"
#include <string.h>
#include <stdlib.h>

/* Native descendants can outlive a window through an external reference.
 * Inspect names rather than relying on a theme's intermediate widget layout. */
static GtkWidget *find(GtkWidget *root, const char *name)
{
    if (strcmp(gtk_widget_get_name(root), name) == 0) return root;
    for (GtkWidget *child = gtk_widget_get_first_child(root); child != NULL;
         child = gtk_widget_get_next_sibling(child)) {
        GtkWidget *match = find(child, name); if (match != NULL) return match;
    }
    return NULL;
}
int main(void)
{
    if (!gtk_init_check()) return 77;
    GtkWindow *window = umi_os_desktop_shell_create(NULL);
    UMI_TEST_REQUIRE(window != NULL);
    UMI_TEST_REQUIRE(find(GTK_WIDGET(window), "welcome") != NULL);
    /* The OS shell composes the same branded application catalogue. Merely
     * creating it must not start an application or fabricate process evidence. */
    GtkWidget *titlebar = gtk_window_get_titlebar(window);
    UMI_TEST_REQUIRE(GTK_IS_HEADER_BAR(titlebar));
    UMI_TEST_REQUIRE(g_object_get_data(G_OBJECT(titlebar),"umicom-automation-id") != NULL);
    UMI_TEST_REQUIRE(g_object_get_data(G_OBJECT(window),"umicom.os.application-titlebar") != NULL);

    GtkWidget *launcher = find(GTK_WIDGET(window), "umicom.workspace.launcher");
    UMI_TEST_REQUIRE(launcher != NULL && GTK_IS_BUTTON(launcher));
    UMI_TEST_REQUIRE(gtk_window_get_application(window) == NULL);
    /* Creating the shell must not open an extra workspace or system window. */
    UMI_TEST_REQUIRE(g_list_model_get_n_items(gtk_window_get_toplevels()) == 1U);
    /* The actual OS launcher exposes the shared restore workflow without
     * opening storage as a side effect of opening the window. */
    g_signal_emit_by_name(launcher, "clicked");
    GListModel *windows = gtk_window_get_toplevels();
    UMI_TEST_REQUIRE(g_list_model_get_n_items(windows) == 2U);
    for (guint index = 0U; index < g_list_model_get_n_items(windows); ++index) {
        GtkWindow *child = g_list_model_get_item(windows, index);
        if (child != window) {
            GtkWidget *preview = find(GTK_WIDGET(child), "umicom.workspace.preview");
            UMI_TEST_REQUIRE(preview && !gtk_widget_get_sensitive(preview));
            UMI_TEST_REQUIRE(find(GTK_WIDGET(child), "umicom.workspace.checkpoint") != NULL);
            GtkWidget *history = find(GTK_WIDGET(child), "umicom.workspace.history");
            GtkWidget *refresh = find(GTK_WIDGET(child), "umicom.workspace.history-refresh");
            UMI_TEST_REQUIRE(GTK_IS_DROP_DOWN(history) && !gtk_widget_get_sensitive(history));
            UMI_TEST_REQUIRE(GTK_IS_BUTTON(refresh) && !gtk_widget_get_sensitive(refresh));
            UMI_TEST_REQUIRE(g_list_model_get_n_items(gtk_drop_down_get_model(GTK_DROP_DOWN(history))) == 0U);
            gtk_window_destroy(child); g_object_unref(child); break;
        }
        g_object_unref(child);
    }
    gtk_window_destroy(window);
    return EXIT_SUCCESS;
}
