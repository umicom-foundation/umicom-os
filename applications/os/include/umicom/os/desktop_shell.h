/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/desktop_shell.h
 * PURPOSE: Compose the user-space desktop from shared Framework workspace services.
 * AUTHOR AND ORGANISATION: Sammy Hegab, Umicom Foundation
 * LICENCE: MIT
 *---------------------------------------------------------------------------*/
#ifndef UMICOM_OS_DESKTOP_SHELL_H
#define UMICOM_OS_DESKTOP_SHELL_H
#include <gtk/gtk.h>
G_BEGIN_DECLS
/* Creates a window without opening storage or collecting system information.
 * The application owns window lifetime. NULL application is useful to embed
 * the shell in another native host. Workspace and system actions are explicit. */
GtkWindow *umi_os_desktop_shell_create(GtkApplication *application);
G_END_DECLS
#endif
