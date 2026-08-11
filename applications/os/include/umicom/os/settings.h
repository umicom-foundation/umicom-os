/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/settings.h
 *
 * PURPOSE:
 *   Define user-facing OS settings while leaving persistence to Framework services.
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
#ifndef UMICOM_OS_SETTINGS_H
#define UMICOM_OS_SETTINGS_H
#include <stdbool.h>
typedef struct UmiOsSettings { bool dark_mode; bool animations; bool developer_mode; } UmiOsSettings;
UmiOsSettings umi_os_settings_default(void);
#endif
