/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: applications/os/src/recovery.c
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
#include "umicom/os/recovery.h"
UmiOsRecoveryBoundary umi_os_recovery_boundary(void){UmiOsRecoveryBoundary b={false,true};return b;}
