/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/desktop.h
 *
 * PURPOSE:
 *   Describe the Umicom OS desktop composition independently of GTK widget pointers.
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
#ifndef UMICOM_OS_DESKTOP_H
#define UMICOM_OS_DESKTOP_H
#include <stddef.h>
#define UMI_OS_DESKTOP_NAME_CAPACITY 96U
typedef struct UmiOsDesktop { char name[UMI_OS_DESKTOP_NAME_CAPACITY]; size_t launcher_count; size_t system_panel_count; } UmiOsDesktop;
void umi_os_desktop_init(UmiOsDesktop *desktop);
#endif
