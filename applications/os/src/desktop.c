/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/desktop.c
 *
 * PURPOSE:
 *   Implement one product-level Umicom OS user-space service or model.
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
#include "umicom/os/desktop.h"

#include <string.h>

void umi_os_desktop_init(UmiOsDesktop *desktop)
{
    static const char NAME[] = "Umicom Desktop";
    if (desktop == NULL) {
        return;
    }
    (void)memset(desktop, 0, sizeof(*desktop));
    (void)memcpy(desktop->name, NAME, sizeof(NAME));
    desktop->launcher_count = 1U;
    desktop->system_panel_count = 4U;
}
