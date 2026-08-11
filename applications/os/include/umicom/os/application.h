/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/application.h
 *
 * PURPOSE:
 *   Define the user-space Umicom OS application lifecycle and product identity.
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
#ifndef UMICOM_OS_APPLICATION_H
#define UMICOM_OS_APPLICATION_H
#include "umicom/base/status.h"
typedef struct UmiOsApplication { int initialised; int running; } UmiOsApplication;
void umi_os_application_init(UmiOsApplication *app);
UmiStatus umi_os_application_start(UmiOsApplication *app);
UmiStatus umi_os_application_stop(UmiOsApplication *app);
#endif
