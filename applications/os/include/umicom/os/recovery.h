/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/include/umicom/os/recovery.h
 *
 * PURPOSE:
 *   Make the Framework-independent recovery boundary explicit to user-space code.
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
#ifndef UMICOM_OS_RECOVERY_H
#define UMICOM_OS_RECOVERY_H
#include <stdbool.h>
typedef struct UmiOsRecoveryBoundary { bool framework_required; bool networking_optional; } UmiOsRecoveryBoundary;
UmiOsRecoveryBoundary umi_os_recovery_boundary(void);
#endif
