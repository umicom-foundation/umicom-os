/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/gtk4/desktop_shell.c
 * PURPOSE: Expose recoverable desktop notes and read-only system tools through Framework.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#include "umicom/os/desktop_shell.h"
#include "umicom/desktop_workspace/gtk4.h"
#include "umicom/desktop_system/gtk4.h"
#include "umicom/ui/gtk4/workstation/shell_header.h"

/* Labels describe the user's choices; state ownership and persistence remain
 * in Framework. New desktop tools should attach their existing shared view
 * here rather than introduce another store or background service in the shell. */
static void explanation(GtkWidget *box, const char *text)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 85);
    gtk_box_append(GTK_BOX(box), label);
}
/* Store the titlebar with the window so its callbacks cannot outlive the
 * shell. Applications launched from its catalogue have independent lifetimes. */
static void release_application_titlebar(gpointer data)
{
    umi_gtk4_ws_window_titlebar_destroy(data);
}

GtkWindow *umi_os_desktop_shell_create(GtkApplication *application)
{
    GtkWindow *window = GTK_WINDOW(application != NULL
        ? gtk_application_window_new(application) : gtk_window_new());
    GtkWidget *page = gtk_scrolled_window_new();
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    gtk_widget_set_name(GTK_WIDGET(window), "umicom-os-main");
    gtk_widget_set_name(root, "desktop-root");
    gtk_window_set_title(window, "Umicom OS Desktop");
    /* The canonical Framework titlebar supplies Umicom branding, application
     * search, explicit multi-open and process receipts without OS-local launch
     * logic. Missing executables remain visible with installation guidance. */
    UmiGtk4WorkstationWindowTitlebar *titlebar = NULL;
    UmiGtk4WorkstationShellHeaderConfig identity =
        umi_gtk4_ws_shell_header_config_default("org.umicom.os", "Umicom OS Desktop");
    if (umi_gtk4_ws_window_titlebar_create(window,&identity,NULL,&titlebar) == UMI_STATUS_OK)
        g_object_set_data_full(G_OBJECT(window),"umicom.os.application-titlebar",
            titlebar,release_application_titlebar);

    gtk_window_set_default_size(window, 1100, 720);
    gtk_widget_set_margin_start(root, 24); gtk_widget_set_margin_end(root, 24);
    gtk_widget_set_margin_top(root, 24); gtk_widget_set_margin_bottom(root, 24);
    GtkWidget *title = gtk_label_new("Umicom OS desktop shell — Framework GTK4 components");
    gtk_widget_set_name(title, "welcome"); gtk_widget_add_css_class(title, "title-1");
    gtk_label_set_wrap(GTK_LABEL(title), TRUE); gtk_label_set_xalign(GTK_LABEL(title), 0.0F);
    gtk_box_append(GTK_BOX(root), title);
    explanation(root, "Keep notes, adjust workspace appearance and recover a saved checkpoint. "
        "The desktop workspace opens in its own window; storage opens only when you choose Open workspace.");
    /* The shell explains the user workflow; Framework owns copied review
     * evidence, persistence and the guarded restore transaction. */
    explanation(root, "To recover earlier work, save or discard your draft, enter a retained checkpoint number, "
        "and choose Preview checkpoint. Compare the complete notes and preferences before choosing Restore reviewed checkpoint.");
    /* The shared picker exposes retained revision identities. Selecting a
     * row only prepares the manual field; restore still requires full review. */
    explanation(root, "The checkpoint list shows retained notes and revision numbers, newest first. "
        "Select a checkpoint, then choose Preview checkpoint. Refresh checkpoints updates this list without discarding an unsaved draft.");
    if (UmiDesktopWorkspaceGtkAttach(root) != UMI_STATUS_OK)
        explanation(root, "The desktop workspace could not be attached.");
    gtk_box_append(GTK_BOX(root), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    explanation(root, "System Centre shows memory, processes, storage and network observations. "
        "Choose Refresh inside the centre to collect a new read-only snapshot.");
    if (UmiDesktopSystemGtk4Attach(root) != UMI_STATUS_OK)
        explanation(root, "System Centre could not be attached.");
    gtk_box_append(GTK_BOX(root), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    explanation(root, "This is the Umicom OS user-space desktop running on your current operating system. "
        "It does not install an operating system, change boot settings or replace your desktop session.");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(page), root);
    gtk_window_set_child(window, page);
    return window;
}
