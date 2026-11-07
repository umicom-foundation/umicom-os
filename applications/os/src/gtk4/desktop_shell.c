/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/gtk4/desktop_shell.c
 *
 * PURPOSE:
 *   Expose recoverable desktop notes and read-only system tools through Framework.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
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

/* The original notes-first composition is retained for engineering review.
 * The desktop home below replaces its long introductory screen with explicit
 * application, workspace and system entry points. Framework still owns all
 * persistence, catalogue discovery and process execution. */
#if 0
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

#endif

/* Resolve the controller from the live owner window at activation time. The
 * connection follows the window lifetime and does not borrow a freed titlebar. */
static void open_applications(GtkButton *button, gpointer user_data)
{
    GtkWindow *window = GTK_WINDOW(user_data);
    UmiGtk4WorkstationWindowTitlebar *titlebar =
        g_object_get_data(G_OBJECT(window), "umicom.os.application-titlebar");
    GtkWidget *status = g_object_get_data(G_OBJECT(button), "desktop-status");
    UmiStatus result = umi_gtk4_ws_window_titlebar_open_applications(titlebar);
    gtk_label_set_text(GTK_LABEL(status), result == UMI_STATUS_OK
        ? "Choose applications in the catalogue, then choose Open selected."
        : "The application catalogue is not ready. Try the Applications control in the titlebar.");
}

/* FlowBox wraps cards when the window narrows. Card contents retain their
 * natural height so larger text never disappears behind a fixed-height row. */
static GtkWidget *desktop_card(GtkWidget *cards, const char *title, const char *description)
{
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    GtkWidget *heading = gtk_label_new(title);
    gtk_widget_set_margin_start(box, 16);
    gtk_widget_set_margin_end(box, 16);
    gtk_widget_set_margin_top(box, 16);
    gtk_widget_set_margin_bottom(box, 16);
    gtk_widget_set_size_request(box, 260, -1);
    gtk_widget_set_hexpand(box, TRUE);
    gtk_widget_set_valign(box, GTK_ALIGN_START);
    gtk_widget_add_css_class(heading, "title-3");
    gtk_label_set_xalign(GTK_LABEL(heading), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(heading), TRUE);
    gtk_box_append(GTK_BOX(box), heading);
    explanation(box, description);
    gtk_flow_box_insert(GTK_FLOW_BOX(cards), box, -1);
    return box;
}

GtkWindow *umi_os_desktop_shell_create(GtkApplication *application)
{
    GtkWindow *window = GTK_WINDOW(application != NULL
        ? gtk_application_window_new(application) : gtk_window_new());
    GtkWidget *page = gtk_scrolled_window_new();
    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    GtkWidget *title = gtk_label_new("Your Umicom desktop");
    GtkWidget *cards = gtk_flow_box_new();
    GtkWidget *help = gtk_expander_new("Workspace and recovery guide");
    GtkWidget *help_content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    UmiGtk4WorkstationWindowTitlebar *titlebar = NULL;
    UmiGtk4WorkstationShellHeaderConfig identity =
        umi_gtk4_ws_shell_header_config_default("org.umicom.os", "Umicom OS Desktop");
    gtk_widget_set_name(GTK_WIDGET(window), "umicom-os-main");
    gtk_widget_set_name(root, "desktop-root");
    gtk_widget_set_name(title, "welcome");
    gtk_widget_set_name(cards, "desktop-actions");
    gtk_widget_set_name(help, "desktop-recovery-guide");
    gtk_window_set_title(window, "Umicom OS Desktop");
    gtk_window_set_default_size(window, 1100, 720);
    if (umi_gtk4_ws_window_titlebar_create(window, &identity, NULL, &titlebar) == UMI_STATUS_OK)
        g_object_set_data_full(G_OBJECT(window), "umicom.os.application-titlebar",
            titlebar, release_application_titlebar);
    gtk_widget_set_margin_start(root, 24);
    gtk_widget_set_margin_end(root, 24);
    gtk_widget_set_margin_top(root, 24);
    gtk_widget_set_margin_bottom(root, 24);
    gtk_widget_add_css_class(title, "title-1");
    gtk_label_set_xalign(GTK_LABEL(title), 0.0F);
    gtk_label_set_wrap(GTK_LABEL(title), TRUE);
    gtk_box_append(GTK_BOX(root), title);
    explanation(root, "Open your applications, continue a workspace or inspect this computer.");
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(cards), GTK_SELECTION_NONE);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(cards), 1);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(cards), 3);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(cards), 12);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(cards), 12);
    gtk_box_append(GTK_BOX(root), cards);

    GtkWidget *apps = desktop_card(cards, "Applications",
        "Find Umicom applications and open the tools you need.");
    GtkWidget *open = gtk_button_new_with_label("Browse applications");
    GtkWidget *status = gtk_label_new("Opening the catalogue does not start an application.");
    gtk_widget_set_name(open, "desktop-open-applications");
    gtk_widget_set_name(status, "desktop-application-status");
    gtk_widget_add_css_class(open, "suggested-action");
    gtk_label_set_wrap(GTK_LABEL(status), TRUE);
    gtk_label_set_xalign(GTK_LABEL(status), 0.0F);
    gtk_widget_set_sensitive(open, titlebar != NULL);
    g_object_set_data_full(G_OBJECT(open), "desktop-status", g_object_ref_sink(status), g_object_unref);
    g_signal_connect_object(open, "clicked", G_CALLBACK(open_applications), window, 0);
    gtk_box_append(GTK_BOX(apps), open);
    gtk_box_append(GTK_BOX(apps), status);

    GtkWidget *workspace = desktop_card(cards, "Workspace",
        "Keep notes and review saved checkpoints in a separate window.");
    if (UmiDesktopWorkspaceGtkAttach(workspace) != UMI_STATUS_OK)
        explanation(workspace, "The desktop workspace could not be attached.");
    GtkWidget *system = desktop_card(cards, "System Centre",
        "Inspect memory, processes, storage and network observations.");
    if (UmiDesktopSystemGtk4Attach(system) != UMI_STATUS_OK)
        explanation(system, "System Centre could not be attached.");
    explanation(system, "Choose Refresh in System Centre to collect a read-only snapshot.");

    /* Recovery instructions remain available without occupying the home
     * screen. Expanding this guide has no storage or restore side effects. */
    explanation(help_content, "Choose Open persistent desktop workspace, then choose a workspace location. "
        "Storage opens only when you choose Open workspace in that window.");
    explanation(help_content, "To recover earlier work, save or discard your draft. Select a retained "
        "checkpoint and choose Preview checkpoint. Compare the complete notes and preferences "
        "before choosing Restore reviewed checkpoint.");
    explanation(help_content, "The checkpoint list shows retained notes and revision numbers, newest first. "
        "Refresh checkpoints updates the list without discarding an unsaved draft.");
    gtk_expander_set_child(GTK_EXPANDER(help), help_content);
    gtk_box_append(GTK_BOX(root), help);
    explanation(root, "This desktop runs on your current operating system. "
        "Installing or booting a complete Umicom OS image is a separate workflow.");
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(page), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(page), root);
    gtk_window_set_child(window, page);
    return window;
}
