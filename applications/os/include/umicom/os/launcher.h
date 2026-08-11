/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/launcher.h
 *
 * PURPOSE:
 *   Store bounded launcher entries for independently runnable Umicom applications.
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
#ifndef UMICOM_OS_LAUNCHER_H
#define UMICOM_OS_LAUNCHER_H
#include <stddef.h>
#include "umicom/base/status.h"
#define UMI_OS_LAUNCHER_CAPACITY 64U
#define UMI_OS_LAUNCH_TEXT_CAPACITY 128U
typedef struct UmiOsLaunchEntry { char id[UMI_OS_LAUNCH_TEXT_CAPACITY]; char label[UMI_OS_LAUNCH_TEXT_CAPACITY]; char executable[UMI_OS_LAUNCH_TEXT_CAPACITY]; } UmiOsLaunchEntry;
typedef struct UmiOsLauncher { UmiOsLaunchEntry entries[UMI_OS_LAUNCHER_CAPACITY]; size_t count; } UmiOsLauncher;
void umi_os_launcher_init(UmiOsLauncher *launcher);
UmiStatus umi_os_launcher_add(UmiOsLauncher *launcher,const char *id,const char *label,const char *executable);
#endif
